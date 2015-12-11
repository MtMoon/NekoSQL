/*
 * IndexManager.h
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_

#include "../bufmanager/BufPageManager.h"
#include "../Tool/RecordTool.h"

struct indexinfo {
	string indexName;
	string tableName;
	string fieldName;
	int fieldType; //0 int 1 char
	int ifFixed; // 1 索引字段为定长，0 索引字段为变长
	int ifNull; // 0不允许字段为NULL，允许字段为NULL
	int indexType; //0 非簇集不唯一，1 非簇集唯一，2 簇集(默认唯一)
	int fieldLen; //若是定长，则存储定长码值长度
	bool legal; //是否合法

};

typedef struct indexinfo IndexInfo;

/**
 *	为了便于文件操作，将B+树直接写在该类中
 */
class IndexManager {
public:
	IndexManager();
	~IndexManager();

	//用户可以调用的索引定义操作函数

	//创建索引

	int createIndex(IndexInfo indexInfo);

	//删除索引
	int dropIndex(string tableName, string indexName);

	//查询解析模块调用的索引功能函数
	void setDataBase(string dbName);
	int insertRecord(ConDP key, Data record);

private:
	//用于数据文件的操作
	BufPageManager* dbm;
	FileManager* dfm;

	//用于索引文件的操作
	BufPageManager* ibm;
	FileManager* ifm;


	string currentDB;
	string currentTable;
	string currentIndex;
	IndexInfo currentIndexInfo;

	int currentFileID;

	//用于索引操作的函数
	//重构数据文件
	int reBuildData(IndexInfo indexinfo);

	//打开和关闭索引
	int openIndex(string tableName, string indexName);
	int closeIndex(string tableName, string indexName);

	//工具函数
	int is_dir_exist(const char* dirpath);
	int is_file_exist(const char* filepath);
	int getFilePageNum(const char* filepath); //获取文件页数
	TableInfo loadTableInfo(Byte* metaPage); //加载表信息
	IndexInfo getCurrentIndexInfo();

	/*----------------------------------------------B+Tree part-----------------------------------*/
	//B+Tree相关成员变量
	int hot;
	int _order; //B+树的阶数
	int lower_bound;
	int upper_bound;
	//B+Tree相关函数
	int search(ConDP key); //找到包含key的叶节点页，返回页号
	bool insert(ConDP key, Data record);
	bool insert(ConDP key, LP pos);
	void solveOverflow(int v); //处理上溢页分裂
	void fillRoot(ConDP key); //插入时根节点为空，填充根节点

	//B+Tree相关的工具函数
	int nodeSearch(ConDP key, int v, int& type);
	void makeIndexLine(IndexInfo& indexinfo, ConDP key, int type, int pid, LP pos, int lineLen, Byte* line);
	int calcuIndexLineLen(IndexInfo& indexinfo, ConDP key, int type); //计算索行的长度


};



#endif /* INDEXMANAGER_H_ */
