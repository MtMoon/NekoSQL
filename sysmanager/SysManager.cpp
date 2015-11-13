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

	if ( rmdir(path.data()) == 0) {
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


