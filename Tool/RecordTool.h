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
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <assert.h>
#include <cmath>
#include "../utils/pagedef.h"
using namespace std;

typedef unsigned char Byte;

typedef pair<Byte*, int> Data; //一个数据， first为Data指针，second为数据长度


typedef pair<string, Data> DP; //data pair, 用以指定一个数据对, first 为字段名 second为值,
typedef pair<int,int> LP; //location air 用以定位一条数据记录，first为页号 second为槽号

struct condidp { //专门用户search的结构体，用于值的比较
	int value_int; //int类型的字段值
	string value_str; //char或varchar类型的字段值
	string name; //字段名
	int type; //字段类型，0为int，1为char，-1为非法
	bool isnull; //是否为null
};

typedef struct condidp ConDP;

/**
 *定义一个表的固定基本信息
 *这部分信息存在表对应文件的第一页
 *注意，会变化的信息，如表中页数，不在此结构体中保存
 *Attention! 2015.11.13 增加Vlen,keys和type
 */
struct tableinfo {
	int FN; // fixed-length column num
	int VN; // variable-length column num
	string* Fname; // 定长列名数组
	string* Vname; //变长列名数组
	int*  Flen; //定长数据长度
	int* Vlen; //变长数据的最大长度
	int* keys; //每个字段的键类型 0为不是key，1为一般key，2为primary key 参照下面的fieldInfo
	int* types; //各个字段的类型  目前：0 int 1 char 2 varchar，其余非法
	Byte* nullMap;
};


typedef struct tableinfo TableInfo;

/**
 *定义一个字段的信息
 */
struct fieldinfo {
	string fieldName; //字段名
	int fieldType; //字段类型目前：0 int 1 char 2 varchar，其余非法
	int fieldSize; //字段byte数
	bool ifNull; //是否能够为null
	int key; //0为不是key，1为一般key，2为primary key
	//这里暂时不支持default

};

typedef struct fieldinfo FieldInfo;

class RecordTool {
public:
	RecordTool();
	~RecordTool();
	//生成一条数据记录
	static Byte* makeRecord(TableInfo tb, int& len, DP data[], int size);
	static bool isVCol(TableInfo& tb, string& colname); //判断一个列是否是变长列

	static Data int2Data(int c); //int转换为data
	static Data str2Data(char* str, int size); //字符数组转化为Data
	static int byte2Int(Byte* byte, int size); //将从byte首地址开始的sizg个byte转为int size不能大于4

	static int data2Int(Data d);
	static char* data2Str(Data d);

	//获取某个定长字段的位置 first为其在数据行中的起始位置，second表示其列序号
	static LP getSegOffset(TableInfo& tb, string& segname);
	static int getNVLen(TableInfo& tb); //获取某个表中数据行非变长数据部分的长度

	static bool hasSameSegVal(TableInfo& tb, Data record, ConDP condi, int cmpType); //判断某条数据的某个字段是否满足特定值
	static ConDP getFieldValueInRecord(TableInfo& tb, Data record, string fieldName); //获取某个记录行中某个字段的值

	static void copyByte(Byte* dst, const Byte* src, int len);
	static void int2Byte(Byte* byte, int size, int num);
	static int getRecordLen(Byte* byte);

	//打印用的工具函数

	//在命令行打印表信息
	static string printTableInfo(const TableInfo& tb);
};



#endif /* RECORDTOOL_H_ */
