/*
 * IndexManager.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#include "IndexManager.h"

IndexManager::IndexManager() {
	fm = new FileManager();
	bm = new BufPageManager(fm);
	currentDB = "";
	currentTable = "";
	currentIndex = "";
	currentFileID = -1;
	hot = 0;
}

IndexManager::~IndexManager() {
	delete fm;
	delete bm;
}

//供用户调用的定义函数

//创建索引
//返回1为创建成功，0为索引已存在，-1为尚未选中任何database， -2 database不存在
int IndexManager::createIndex( IndexInfo indexInfo) {
	if (currentDB == "") {
		return -1;
	}
	string dbPath = "DataBase/" + currentDB;
	if (is_dir_exist(dbPath.c_str()) == -1) {
		return -2;
	}

	string filePath = dbPath + "/" + indexInfo.indexName + "_" + indexInfo.tableName + ".index";
	if (is_file_exist(filePath.c_str()) == 0) {
		return 0;
	}
	bool success = fm->createFile(filePath.c_str());
	if (!success) {
		return -3;
	}
	int fileID;
	fm->openFile(filePath.c_str(), fileID);
	int index;
	bm->allocPage(fileID, 0, index, false);
	Byte* page = (Byte*)bm->addr[index];

	int metaLen = 51;
	Byte meta[51];
	int off  = 0;
	RecordTool::str2Byte(meta + off, 24, indexInfo.tableName.c_str());
	off += 24;
	RecordTool::str2Byte(meta+off, 24, indexInfo.fieldName.c_str());
	off += 24;
	RecordTool::int2Byte(meta+off, 1, indexInfo.fieldType);
	off += 1;
	Byte tag;
	tag &= 0x00;
	if (indexInfo.ifFixed == 1) {
		tag |= 1;
	}
	if (indexInfo.ifNull == 1) {
		tag |= (1<<1);
	}
	meta[off] = tag;
	off += 1;

	RecordTool::int2Byte(meta+off, 1, indexInfo.indexType);
	off += 1;
	RecordTool::copyByte(page, meta, 51);
	bm->markDirty(index);
	bm->writeBack(index);

	//写入根节点页
	off = 0;
	bm->allocPage(fileID, 1, index, false);
	page = (Byte*)bm->addr[index];
	RecordTool::int2Byte(page+off, 2,  PAGE_SIZE-96);
	off += 2;
	RecordTool::int2Byte(page+off, 1,  0); //level0即根页
	off += 2;
	RecordTool::int2Byte(page+off, 1,  0); //页类型，非页级索引页
	off += 1;
	RecordTool::int2Byte(page+off, 2,  0); //已有索引行数量为0
	bm->markDirty(index);
	bm->writeBack(index);

	fm->closeFile(fileID);



	return 1;
}

//删除索引
//返回1为删除成功，0为索引不存在，-1为尚未选中任何database， -2 database不存在
int IndexManager::dropIndex(string tableName, string indexName) {
	if (currentDB == "") {
		return -1;
	}
	string dbPath = "DataBase/" + currentDB;
	if (is_dir_exist(dbPath.c_str()) == -1) {
		return -2;
	}

	string filePath = dbPath + "/" + indexName + "_" + tableName + ".index";
	if (is_file_exist(filePath.c_str()) == -1) {
		return 0;
	}
	if (remove(filePath.c_str()) == 0) {
			return 1;
	} else {
			return -3;
	}
	return -3;
}

/*-----------------------------------------------------------------------------------------------------*/
//供查询解析模块调用的功能函数

void  IndexManager::setDataBase(string dbName) {
	currentDB = dbName;
}

int IndexManager::insertRecord(ConDP key, Data record) {

}

/*-----------------------------------------------------------------------------------------------------*/

//用于索引操作的内部函数
//重构数据文件
// return 0 非簇集索引，无需重构数据文件
// return 1 簇集索引，但数据文件中尚无记录，无需重构
// return 2 成功重构
// return -2 尚未选择database
int IndexManager::reBuildData(IndexInfo indexinfo) {
	if (indexinfo.indexType != 2) {
		return 0;
	}

	if (currentDB == "") {
		return -2;
	}

	string dfilepath = "DataBase/" + currentDB + "/" + indexinfo.tableName + ".data";
	int datapageNum = 0;
	datapageNum = getFilePageNum(dfilepath.c_str());
	if (datapageNum  <= 1) {
		return 1;
	}

	//重构所有数据记录
	string ifilepath = "DataBase/" + currentDB + "/" + indexinfo.indexName + "_" + indexinfo.tableName + ".index";

	//复制meta数据页
	Byte metapage[PAGE_SIZE];
	int fileid = 0;
	fm->openFile(dfilepath.c_str(), fileid);
	int pageindex = 0;
	Byte* page = (Byte*)bm->getPage(fileid, 0, pageindex);
	RecordTool::copyByte(metapage, page, PAGE_SIZE);
	fm->closeFile(fileid);

	//加载table info
	TableInfo tb = loadTableInfo(metapage);

	//创建新数据文件
	string tdfilepath = "DataBase/" + currentDB + "/" + indexinfo.tableName + "_temp.data"; //临时数据文件
	fm->openFile(tdfilepath.c_str(), fileid);
	bm->allocPage(fileid, 0, pageindex, false);
	page = (Byte*)bm->addr[pageindex];
	//写入元数据页
	RecordTool::copyByte(page, metapage, PAGE_SIZE);
	bm->markDirty(pageindex);
	bm->writeBack(pageindex);
	fm->closeFile(fileid);

	//将原数据文件的数据行逐一读取，重新插入新的问文件中
	Byte onePage[PAGE_SIZE];
	for (int i=1; i<datapageNum; i++) {
		//以页为单位读取并插入,因为助教给的fm只能同时打开一个文件，而且页不想取该hash函数 ~~~~(>_<)~~~~
		fm->openFile(dfilepath.c_str(), fileid);
		Byte* page = (Byte*)bm->getPage(fileid, i, pageindex);
		RecordTool::copyByte(onePage, page, PAGE_SIZE);
		fm->closeFile(fileid);

		openIndex(indexinfo.tableName, indexinfo.indexName);
		int slotNum = 0;
		slotNum = RecordTool::byte2Int(onePage+2, 2);
		//遍历槽取出所有记录行
		for (int j=0; j<slotNum; j++) {
			int start = 0;
			Byte* temp = onePage + PAGE_SIZE - 2*(j+1);
			start = RecordTool::byte2Int(temp, 2);
			if (start<=96) {
				continue;
			}
			int lineLen = 0;
			lineLen = RecordTool::byte2Int(onePage+start+1, 2);
			Byte line[lineLen];
			RecordTool::copyByte(line, onePage+start, lineLen);
			//插入新的记录文件
			//获取key
			ConDP key = RecordTool::getFieldValueInRecord(tb, Data(line, lineLen), indexinfo.fieldName);
			insertRecord(key, Data(line, lineLen));
		}
		closeIndex(indexinfo.tableName, indexinfo.indexName);

	}

	//删除数据原始文件，重命名临时数据文件
	if (remove(dfilepath.c_str()) != 0) {
		return -1;
	}

	if (rename(tdfilepath.c_str(), dfilepath.c_str()) != 0) {
		return -1;
	}


	return 2;
}

/**
 *  return 0: 该索引当前已打开 1:该表当前未打开，已重新打开索引
 */
int  IndexManager::openIndex(string tableName, string indexName) {

	if (tableName != currentTable || indexName != currentIndex) {
		if ("" != tableName && "" != indexName && currentFileID != -1) {
			fm->closeFile(currentFileID);
		}
		string filepath = "";
		filepath = "DataBase/" + currentDB + "/" + indexName + "_" + tableName + ".index";
		fm->openFile(filepath.c_str(), currentFileID);
		return 1;
	}
	assert(currentFileID >= 0);
	return 0;
}

/**
 *  * return 0 该表未打开过   1:该表当前打开,成功关闭
 */
int  IndexManager::closeIndex( string tableName, string indexName) {
	if (tableName != currentTable || indexName != currentIndex || currentFileID == -1) {
		return 0;
	}
	fm->closeFile(currentFileID);
	currentFileID = -1;
	currentTable = "";
	currentIndex = "";
	return 1;
}

/*----------------------------------------------B+Tree part-----------------------------------*/
//找到包含key的叶节点页，返回页号
int  IndexManager::search(ConDP key) {
	//从根节点出发
	int pageindex = 0;
	int v = 0;
	bm->getPage(currentFileID, v, pageindex);
	Byte* page = (Byte*)bm->addr[pageindex];
	hot = 0;

	int r = 0;
	while (v != -1) {

	}



}

//B+Tree相关的工具函数

//在一个索引节点内搜索码值
int  IndexManager::nodeSearch(ConDP key, int & child, int& type, const Byte* page) {

}


/*-----------------------------------------------------------------------------------------------------*/
//工具函数
int IndexManager::is_dir_exist(const char* dirpath) {
	 if (dirpath == NULL) {
		 return -1;
	 }

	 if (opendir(dirpath) == NULL) {
		 return -1;
	 }

	 return 0;
}

int IndexManager::is_file_exist(const char* filepath) {
   if (filepath == NULL) {
       return -1;
   }

   if (access(filepath, F_OK) == 0) {
       return 0;
   }

   return -1;
}

//获取文件页数
int  IndexManager::getFilePageNum(const char* filepath) {
	struct stat buf;
	if(stat(filepath, &buf)<0) {
		return 0;
	 }
	return ceil(double(buf.st_size) / PAGE_SIZE);
}

//加载表信息，仅在簇集索引重构数据文件时会用到
TableInfo IndexManager::loadTableInfo(Byte* metapage) {
	TableInfo tb;
	Byte* temp = metapage;
	//get FN
	int fn = 0;
	fn  =  RecordTool::byte2Int(temp,2);
	//cout << "fn: " << fn << endl;
	//get vn
	int vn = 0;
	temp = metapage;
	vn =  RecordTool::byte2Int(temp+2,2);

	//cout << "vn: " << vn << endl;


	tb.Fname = new string[fn];
	tb.Vname = new string[vn];
	tb.Flen = new int[fn];
	tb.Vlen = new int[vn];
	tb.keys = new int[vn+fn];
	tb.types = new int[vn+fn];


	//获取定长列名
	int off = 4;
	for (int i=0; i<fn; i++) {
		char* name = NULL;
		temp = metapage;
		name =  RecordTool::data2Str(Data(temp+off ,24));
		tb.Fname[i] = string(name);
		//cout << "fname: " << tb.Fname[i] << endl;
		off += 24;
	}

	//获取变长列名
	for (int i=0; i<vn; i++) {
			char* name = NULL;
			temp = metapage;
			name =  RecordTool::data2Str(Data(temp+off ,24));
			tb.Vname[i] = string(name);
			//cout << "vname: " << tb.Vname[i] << endl;
			off += 24;
	}

	//获取定长数据长度
	for (int i=0; i<fn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Flen[i] = t;
		//cout << "flen: " << tb.Flen[i] << endl;
		off += 4;
	}

	//获取变长数据长度
	for (int i=0; i<vn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Vlen[i] = t;
		//cout << "vlen: " << tb.Vlen[i] << endl;
		off += 4;
	}

	//获取key数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,1);
		tb.keys[i] = t;
		//cout << "keys: " << tb.keys[i] << endl;
		off += 1;
	}

	//获取types数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,1);
		tb.types[i] = t;
		//cout << "types: " << tb.types[i] << endl;
		off += 1;
	}

	tb.FN = fn;
	tb.VN = vn;

	//获取null位图
	int nlen = ceil(double(tb.FN + tb.VN) / 8);
	//cout << "nlen:" << nlen << endl;
	tb.nullMap = new Byte[nlen];
	for (int i=0; i<nlen; i++) {
		tb.nullMap[i] = *(metapage+off);
		//cout << "nullmap: " << int(tb.nullMap[i]) << endl;
		off++;
	}
	return tb;
}


