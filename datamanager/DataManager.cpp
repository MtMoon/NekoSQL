/*
 * DataManager.cpp
 *
 *  Created on: 2015-10-19
 *      Author: yxy
 */

#include "DataManager.h"


DataManager::DataManager() {
	fm = new FileManager();
	bm = new BufPageManager(fm);
	currentBase = "";
}

DataManager::~DataManager() {
	delete fm;
	delete bm;
}

// 文件操作函数

//新建文件, 文件名应包含database dir name
//新建成功返回true 否则返回false
bool DataManager::createFile(const char* filename) {
	return fm->createFile(filename);
}

//删除文件, 文件名应包含database dir name
// 删除成功返回true 否则返回false
bool DataManager::deleteFile(const char* filename) {
	this->closeFile(filename); //先关闭再删除
	if (remove(filename) == 0) {
		return true;
	} else {
		return false;
	}
}

//打开文件, 文件名应包含database dir name
//打开成功返回true 否则返回false
// fileID存储FileManager的文件句柄
bool DataManager::openFile(const char* filename, int& fileID) {
	return fm->openFile(filename, fileID);
}

//关闭文件, 文件名应包含database dir name
bool DataManager::closeFile(const char* filename) {
	int fileID = -1;
	fm->openFile(filename, fileID);
	fm->closeFile(fileID);
	return true;
}

//记录操作函数

//插入一条记录
//DP数组中，必须包含定义时的所有字段, 为NULL的字段，对应的数据string以空串表示
//NULL位图中会区别空串和NULL
//数据类型的对应,是否可以为NULL需在上层检查
//若要支持NULL值字段不在sql语句中显示写出，需要上层转换
bool DataManager::insertRecord(const char* tablename, DP data[], const int size) {
	int len = 0; //数据记录长度
	Byte* record = NULL;
	TableInfo tb = getTableInfo(tablename);
	//生成数据记录
	record = RecordTool::makeRecord(tb, len, data, size);

	// 寻找合适的页插入
	int pageNum =  getPageNum(tablename);
	string filepath(tablename);
	filepath = currentBase + "/" + filepath;

	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);

	//遍历除了第一页之外的各个页
	//获取有空余byte的页的index
	BufType page = new unsigned int[2048];
	int pageindex = 0;
	for (int i=1; i<pageNum; i++) {
		bm->getPage(fileID, i, pageindex);
		if (getPageLeftSize(pageindex) >=  len) {
			break;
		}
	}

	//写入数据
	int slotNum = 0;
	//获取槽数



	//修改剩余字节数



}

int DataManager::getPageLeftSize(int pageindex) {
	BufType page = bm->addr[pageindex];
	//page头的前2个byte用来表示剩余字节数
	return RecordTool::byte2Int((const Byte*)page, 2);
}

//工具函数
//获取对应表的信息
TableInfo DataManager::getTableInfo(const char* tablename) {
	string name(tablename);
	if (tables.find(name) != tables.end()) {
		return tables[name];
	} else {
		return loadTableInfo(tablename);
	}
}

//加载表当前的页数
int DataManager::getPageNum(const char* tablename) {
	//获取文件的大小再除以8k即可
	string filepath(tablename);
	filepath = currentBase + "/" + filepath;

	struct stat buf;
	if(stat(filepath.c_str(), &buf)<0) {
		return 0;
	 }
	return (int)(buf.st_size / 8);
}

//加载表的信息，存于tables并返回
TableInfo DataManager::loadTableInfo(const char* tablename) {

}

