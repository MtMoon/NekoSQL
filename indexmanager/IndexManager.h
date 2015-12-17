/*
 * IndexManager.h
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_

#include "../bufmanager/BufPageManager.h"
#include "../datamanager/DataManager.h"
#include "../Tool/RecordTool.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>

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
	IndexManager(DataManager* datamanager);
	~IndexManager();

	//用户可以调用的索引定义操作函数
	//创建索引
	int createIndex(IndexInfo indexInfo);

	//删除索引
	int dropIndex(string tableName, string indexName);

	//查询解析模块调用的索引功能函数
	void setDataBase(string dbName);
	void setIndex(string tableName, string indexName);

	vector<LP> searchKey(ConDP key); //查找key值满足特定条件的记录的位置
	int insertRecord(ConDP key, LP pos);
	int deleteRecord(ConDP key, LP pos);
	bool upDateRecord(ConDP oldKey, ConDP newKey, LP oldPos, LP newPos);


	 //获取某个表某个字段的indexinfo, flag为false表示索引不存在
	IndexInfo getIndexInfo(string tableName, string fieldName, bool& flag);

private:

	//用于索引文件的操作
	BufPageManager* ibm;
	FileManager* ifm;

	//用于对接记录管理模块
	DataManager* dm;

	string currentDB;
	string currentTable;
	string currentIndex;
	IndexInfo currentIndexInfo;

	map<string, IndexInfo> indexMap; //索引已有的索引信息

	int currentFileID;

	//用于索引操作的函数
	//为以有的数据文件建立索引
	int reBuildData(IndexInfo indexinfo);

	//工具函数
	int is_dir_exist(const char* dirpath);
	int is_file_exist(const char* filepath);
	int getFilePageNum(const char* filepath); //获取文件页数
	IndexInfo getCurrentIndexInfo();
	bool ConDPEqual(ConDP key1, ConDP key2); //判断两个key是否相等
	int loadIndexMap(); //加载已有索引信息
	int addIndexInfo(IndexInfo indexinfo); //向索引文件表中加入追加写入索引信息
	int reWriteIndexSys(); //把当前加载的indexmap重新写入文件
	int openIndex(string tableName, string indexName);
	int closeIndex(string tableName, string indexName);

	/*----------------------------------------------B+Tree part-----------------------------------*/
	//B+Tree相关成员变量
	int hot;
	int _order; //B+树的阶数
	int lower_bound;
	int upper_bound;
	//B+Tree相关函数
	int search(ConDP key); //找到包含key的叶节点页，返回页号
	bool insert(ConDP key, LP pos);
	void solveOverflow(int v); //处理上溢页分裂
	bool removeLine(ConDP key, LP pos); //删除节点码值
	void fillRoot(ConDP key); //插入时根节点为空，填充根节点

	//B+Tree相关的工具函数
	int nodeSearch(ConDP key, int v, int& type);
	int leafNodeSearch(ConDP key, int v, int searchtype, int nodetype); //定位叶节点中满足key的某条索引行的页偏移
	void makeIndexLine(IndexInfo& indexinfo, ConDP key, int type, int nexttype, int pid, LP pos, int lineLen, Byte* line);
	int calcuIndexLineLen(IndexInfo& indexinfo, ConDP key, int type); //计算索行的长度
	int newIndexPage(int type, int parent); //新开一个索引页，返回页号
	ConDP getKeyByLine(Byte* line, IndexInfo& indexinfo, LP& pos); //获取key值



};



#endif /* INDEXMANAGER_H_ */
