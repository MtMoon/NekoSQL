/*
 * RecordTool.h
 *用于数据记录操作的工具类
 *以及相关常量，数据结构体的定义
 *  Created on: 2015-10-21
 *      Author: yxy
 */

#ifndef RECORDTOOL_H_
#define RECORDTOOL_H_

#include <string>
#include <cstdlib>
#include <utility>
#include "assert.h"
#include <cmath>
#include "../utils/pagedef.h"
using namespace std;

typedef unsigned char Byte;

typedef pair<Byte*, int> Data; //一个数据， first为Data指针，second为数据长度


typedef pair<string, Data> DP; //data pair, 用以指定一个数据对, first 为字段名 second为值,
typedef pair<int,int> LP; //location air 用以定位一条数据记录，first为页号 second为槽号

/**
 *定义一个表的固定基本信息
 *这部分信息存在表对应文件的第一页
 *注意，会变化的信息，如表中页数，不在此结构体中保存
 */
struct tableinfo {
	int FN; // fixed-length column num
	int VN; // variable-length column num
	string* Fname; // 定长列名数组
	string* Vname; //变长列名数组
	int*  Flen; //定长数据长度
	Byte* nullMap;
};

typedef struct tableinfo TableInfo;

class RecordTool {
public:
	RecordTool();
	~RecordTool();
	//生成一条数据记录
	static Byte* makeRecord(TableInfo tb, int& len, DP data[], int size);
	static bool isVCol(TableInfo& tb, string& colname); //判断一个列是否是变长列

	static Data int2Data(int c); //int转换为data
	static Data str2Data(char* str, int size); //字符数组转化为Data
	static int byte2Int(const Byte* byte, int size); //将从byte首地址开始的sizg个byte转为int size不能大于4

	static int data2Int(Data d);
	static char* data2Str(Data d);

	//获取某个定长字段的位置 first为其在数据行中的起始位置，second表示其列序号
	static LP getSegOffset(TableInfo& tb, string& segname);
};



#endif /* RECORDTOOL_H_ */
