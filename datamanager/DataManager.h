/*
 * DataManager.h
 *
 *  Created on: 2015-10-19
 *      Author: yxy
 */
#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

#include "../bufmanager/BufPageManager.h"
#include "../Tool/RecordTool.h"
#include <string>
#include <vector>
#include <map>
#include <cstring>





/**
 * 记录管理模块, 实现接近底层的记录操作
 * 记录采用变长记录，以支持varchar类型
 */
class DataManager {

public:
	DataManager();
	~DataManager();

	// 文件操作函数

	//新建文件, 文件名应包含database dir name
	bool createFile(const char* filename);

	//删除文件, 文件名应包含database dir name
	bool deleteFile(const char* filename);

	//打开文件, 文件名应包含database dir name
	bool openFile(const char* filename, int& fileID);

	//关闭文件, 文件名应包含database dir name
	bool closeFile(const char* filename);

	//记录操作函数

	//插入一条记录
	bool insertRecord(const char* tablename, DP data[], const int size);
	//bool insertRecord(const char* tablename, const Byte* record, int len);

	//重载函数 直接以位置和Byte流插入
	bool insertRecord(const char* tablename, Data data, LP pos);

	//更新记录
	bool updateRecord(const char* tablename, LP pos, DP data[], int size);

	//删除记录
	bool deleteRecord(const char* tablename, LP pos);

	//获取属性值满足特定条件的记录
	 //对某一字段cmpType 0：值等于condi的记录，1：值大于condi的记录，2：值小于condi的记录，3：值为null的记录
	vector<LP> searchRecord(const char*tablename, ConDP condi, int cmpType);

	vector<LP> searchRecordInPage(const char* tablename, const int pageindex, ConDP condi, int cmpType); //在某个页内检索

	//工具函数
	TableInfo getTableInfo(const char* tablename);

	//写入表元信息，供SysManager在创建表时使用
	void writeTableInfo(string tableName, TableInfo tb);

	//更上层的接口函数
	void setDatabase(string dirname); 	//切换数据库目录
	string getCurrentDBName(); //获取当前使用的数据库名
	void invalidTbMap(string tbName); //无效化tables中存的表信息
	vector<LP> getAllLPInTable(const char* tablename); //获取一个表里所有有效数据的LP
	vector<KP> getAllKPInTable(const char* tablename, string fieldName); //获取一个表里的所有key pair，用于索引模块


private:
	BufPageManager* bm;
	FileManager* fm;
	string currentBase; //当前使用的数据库，即目录名
	string currentTable; //当前打开的表
	int currentFileID; //当前打开的表的id

	//储存已打开的表的信息
	//每次打开一个表，应先检查是否在map中 不在则需从文件读入
	//切换数据库时，map应当clear
	map<string, TableInfo> tables;

	//工具类函数
	TableInfo loadTableInfo(const char* tablename); //加载表信息

	int openTable(const char* tableName); //统一表(文件)的操作，避免多次打开关闭一个表
	int closeTable(const char* tableName);

	int getPageLeftSize(int pageindex); //获取某个page剩余byte数
	int getPageNum(const char* tablename); //加载表当前的页数
	Data getRecordByLP(const char* tablename, LP pos); //根据位置和表名获取一条数据


	bool newEmptySpecialPage(const char* tablename);	
	int newNormalPage(const char* tablename, int pageID);
	void pageInfo(const char* tablename, int pageID);
};



#endif /* DATAMANAGER_H_ */
