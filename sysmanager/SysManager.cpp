/*
 * SysManager.cpp
 *
 *  Created on: 2015-11-12
 *      Author: yxy
 */

#include "SysManager.h"

SysManager::SysManager(DataManager* dm) {
	assert(dm != NULL);
	this->dataManager = dm;
	readDatabases();
}

SysManager::~SysManager() {

}

void SysManager::readDatabases() {
	dataBaseDic.clear();
	DIR *dir;
	struct dirent  *ptr;
	dir = opendir("DataBase");
	while((ptr = readdir(dir)) != NULL) {
		if (ptr->d_type == 4) {
			if (strcmp(ptr->d_name, ".") !=0 && strcmp(ptr->d_name, "..") !=0) {
				//printf("d_name: %s\n", ptr->d_name);
				++dataBaseDic[string(ptr->d_name)];
			}
		}
	}

	/*
	for (map<string,int>::iterator it = dataBaseDic.begin(); it != dataBaseDic.end(); ++it) {
		cout << it->first << endl;
	}*/


	closedir(dir);
}

int SysManager::is_dir_exist(const char* dirpath) {
	 if (dirpath == NULL) {
		 return -1;
	 }

	 if (opendir(dirpath) == NULL) {
		 return -1;
	 }

	 return 0;
}

int SysManager::is_file_exist(const char* filepath) {
   if (filepath == NULL) {
       return -1;
   }

   if (access(filepath, F_OK) == 0) {
       return 0;
   }

   return -1;
}

//数据库操作函数
/**
  * para: database name
  * return: 1 create successful; 0 database exists;-1 other error
*/
int SysManager::createDatabase(string dbName) {
	if (dataBaseDic.find(dbName) != dataBaseDic.end()) {
		return 0;
	}

	string path = "DataBase/" + dbName;
	if (is_dir_exist(path.data()) == 0) {
		return 0;
	}
	int ret = mkdir(path.data(), S_IRWXU|S_IRWXG);
	if (ret == 0) {
		readDatabases();
		return 1;
	}
	return -1;


}

/**
  * para: database name
  * return: 1 drop successful; 0 database not exists;-1 other error
*/
int SysManager::dropDatabase(string dbName) {
	string path = "DataBase/" + dbName;
	if (is_dir_exist(path.data()) != 0) {
			return 0;
	}
	DIR *dir;
	struct dirent  *ptr;
	dir = opendir(path.data());
	while((ptr = readdir(dir)) != NULL) {
		if (ptr->d_type == 8) {
			string filepath = path + "/" + string(ptr->d_name);
			bool ret = dataManager->deleteFile(filepath.data());
			if (!ret) {
				return -1;
			}
		}
	}
	closedir(dir);

	if ( rmdir(path.data()) == 0) {
		if (dbName == dataManager->getCurrentDBName()) {
			dataManager->setDatabase("");
		}

		readDatabases();
		return 1;
	}
	return -1;


}

/**
  * para: database name
  * return: 1 change successful; 0 database not exists;-1 other error
*/
int SysManager::useDatabase(string dbName) {
	if (dataBaseDic.find(dbName) == dataBaseDic.end()) {
		return 0;
	}
	dataManager->setDatabase(dbName);
	return 1;
}

/**
 * 列出所有Database的名字
 * return: 当无dataBase时，返回为vector 内数据为空
 */
vector<string> SysManager::showDatabases() {
	vector<string> ans;
	for (map<string,int>::iterator it = dataBaseDic.begin(); it != dataBaseDic.end(); ++it) {
		ans.push_back(it->first);
	}
	return ans;
}

//表操作

/**
 * 列出所有表的名字
 * return: 当无表时，返回vector 内数据为空
 * flag:1 成功 0 当前database中无表 -1 尚未选中任何database -2其他错误
*/
vector<string> SysManager::showTables(int& flag) {
	vector<string> ans;
	flag = 0;
	if (dataManager->getCurrentDBName() == "") {
		flag = -1;
		return ans;
	}

	string path = "DataBase/" + dataManager->getCurrentDBName();
	if (is_dir_exist(path.data()) != 0) {
		flag = -2;
		return ans;
	}

	DIR *dir;
	struct dirent  *ptr;
	dir = opendir(path.data());
	while((ptr = readdir(dir)) != NULL) {
		if (ptr->d_type == 8) {
			ans.push_back(string(ptr->d_name));
		}
	}
	closedir(dir);
	if (!ans.empty()) {
		flag = 0;
	}
	return ans;
}


/**
 * para: table name
 * return: 1 drop successful; 0 table not exists;-1 no selected database
 */
int SysManager::dropTable(string tbName) {
	if (dataManager->getCurrentDBName() == "") {
		return -1;
	}

	string filepath = "DataBase/" + dataManager->getCurrentDBName() + "/" + tbName;

	if (is_file_exist(filepath.data()) == -1) {
		return 0;
	}

	if (!dataManager->deleteFile(filepath.data())) {
		return -2;
	}

	//无效化dataManager中缓存的表信息
	dataManager->invalidTbMap(tbName);

	return 1;


}

/**
 * 显示一个表的字段信息
 * 当表不存在时，返回的vector 内数据为空
 * flag:1 成功 0 表不存在 -1 尚未选中任何database
 */
vector<FieldInfo> SysManager::descTable(string tableName, int& flag) {

	vector<FieldInfo> ans;
	flag = 1;
	if (dataManager->getCurrentDBName() == "") {
		flag = -1;
		return ans;
	}


	string filepath = "DataBase/" + dataManager->getCurrentDBName() + "/" + tableName;

	if (is_file_exist(filepath.data()) == -1) {
		flag = 0;
		return ans;
	}


	//获取表的信息
	TableInfo tb = dataManager->getTableInfo(tableName.data());

	//cout << "lalala" << endl;

	//获取定长字段信息
	for (int i=0; i<tb.FN; i++) {
		FieldInfo fi;
		fi.fieldName = tb.Fname[i];
		fi.fieldType = tb.types[i];
		fi.fieldSize = tb.Flen[i];
		fi.ifNull = (tb.nullMap[i/8] >> (i%8))&1;
		//cout << "null f: " << (int)((tb.nullMap[i/8] >> (i%8))&1);
		fi.key = tb.keys[i];
		ans.push_back(fi);
	}

	//获取变长字段信息
	for (int i=0; i<tb.VN; i++) {
			FieldInfo fi;
			fi.fieldName = tb.Vname[i];
			fi.fieldType = tb.types[tb.FN+i];
			fi.fieldSize = tb.Vlen[i];
			int which = tb.FN + i;
			fi.ifNull = (tb.nullMap[which/8] >> (which%8))&1;
			//cout << "null v: " << (int)((tb.nullMap[which/8] >> (which%8))&1);
			fi.key = tb.keys[tb.FN+i];
			ans.push_back(fi);
	}

	return ans;

}

/*
 * 创建表
 * 返回1为创建成功，0为表已存在，-1为尚未选中任何database
 */
int SysManager::createTable(string tableName, vector<FieldInfo> tbvec) {
	if (dataManager->getCurrentDBName() == "") {
		return -1;
	}

	string filepath = "DataBase/" + dataManager->getCurrentDBName() + "/" + tableName;

	if (is_file_exist(filepath.data()) == 0) {
		return 0;
	}

	//创建文件
	if (!dataManager->createFile(filepath.data())) {
		return -2;
	}

	//将表信息格式化为TableInfo
	TableInfo tb;

	tb.FN = 0;
	tb.VN = 0;
	for (int i=0; i<tbvec.size(); i++) {
		if (tbvec[i].fieldType == 2) {
			tb.VN++;
		} else {
			tb.FN++;
		}
	}

	tb.Flen = new int[tb.FN];
	tb.Fname = new string[tb.FN];

	tb.Vlen = new int[tb.VN];
	tb.Vname = new string[tb.VN];

	tb.keys = new int[tb.VN+tb.FN];
	tb.types = new int[tb.VN+tb.FN];

	int fc = 0;
	int vc = 0;

	int nlen = ceil(double(tb.FN+tb.VN)/8);
	tb.nullMap = new Byte[nlen];

	for (int i=0; i<tbvec.size(); i++) {
			if (tbvec[i].fieldType == 2) { //变长varchar
				tb.Vname[vc] = tbvec[i].fieldName;
				tb.Vlen[vc] = tbvec[i].fieldSize;
				tb.types[tb.FN+vc] = tbvec[i].fieldType;
				tb.keys[tb.FN+vc] = tbvec[i].key;

				int which = tb.FN + vc;
				int ifnull = 0;
				if (tbvec[i].ifNull) {
					ifnull = 1;
				}
				tb.nullMap[which/8] |= (ifnull<<(which%8));
				vc++;

			} else { //定长字段
				tb.Fname[fc] = tbvec[i].fieldName;
				tb.Flen[fc] = tbvec[i].fieldSize;
				tb.types[fc] = tbvec[i].fieldType;
				tb.keys[fc] = tbvec[i].key;

				int which = fc;
				int ifnull = 0;
				if (tbvec[i].ifNull) {
					ifnull = 1;
				}
				tb.nullMap[which/8] |= (ifnull<<(which%8));
				fc++;
			}
	}
	//cout << "nullmap: " << int(tb.nullMap[0]) << endl;

	dataManager->writeTableInfo(tableName, tb);
	return 1;


}


