/*
 * RecordTool.cpp
 *
 *  Created on: 2015-10-21
 *      Author: yxy
 */

#include "RecordTool.h"
#include "assert.h"
#include <cmath>

//数据记录行采用SQL Server2000的数据行格式
Byte* RecordTool::makeRecord(TableInfo tb, int& len, DP data[], int size) {

	//计算记录行长度
	len = 8;
	int nullLen = ceil((tb.FN + tb.VN) / 8);
	len += nullLen;
	len += 2*tb.VN;
	for (int i=0; i<size; i++) {
		len += data[i].second.second;
	}
	Byte* line = new Byte[len];

	//make tag A 1Byte
	line[0] &= 0x00;
	line[0] |= (1<<4);

	if (tb.VN !=0) { //存在边长列
		line[0] |= (1<<5);
	}

	//make tag B
	line[1] &= 0x00;

	//定长部分长度
	int fsize = 0; //定长数据长度.
	for (int i=0; i<tb.FN; i++) {
		fsize += tb.Flen[i];
	}

	Byte* temp = (Byte*)&fsize;
	line[2] = *temp++;
	line[3] = *temp;

	//定长部分数据
	int count = 4;
	for (int i=0; i<size; i++) {
		if (isVCol(tb,data[i].first )) {
			continue;
		}
		for (int j=0; j<data[i].second.second; j++) {
			line[count++] = data[i].second.first[j];
		}
	}

	//列数
	int colNum = tb.FN + tb.VN;
	temp = (Byte*)&colNum;
	line[count++]  = *temp++;
	line[count++]  = *temp;

	//null位图
	for (int i=0; i<nullLen; i++) {
		line[count++] = tb.nullMap[i];
	}

	//变列数目
	int vNum = tb.VN;
	temp = (Byte*)&vNum;
	line[count++]  = *temp++;
	line[count++]  = *temp;

	//列偏移数组
	int val = count + 2*tb.VN;
	for (int i=0; i<size; i++) {
			if (isVCol(tb,data[i].first )) {
				val = val + data[i].second.second;
				temp = (Byte*) &val;
				line[count++] = *temp++;
				line[count++] = *temp;

			}
	}

	//变长列数据
	for (int i=0; i<size; i++) {
			if (isVCol(tb,data[i].first )) {
				for (int j=0; j<data[i].second.second; j++) {
					line[count++] = data[i].second.first[j];
				}

			}
	}

	assert(len == count);

	//清除data数组中数据
	for (int i=0; i<size; i++) {
		delete [] data[i].second.first;
	}

	return line;
}

bool RecordTool::isVCol(TableInfo& tb, string& colname) {//判断一个列是否是变长列
	if (tb.VN == 0) {
		return false;
	}

	bool ans = false;
	for (int i=0; i<tb.VN; i++) {
		if (tb.Vname[i] == colname) {
			ans = true;
			break;
		}
	}
	return ans;
}

//int转换为data
Data RecordTool::int2Data(int c) {
	Byte* byte = new Byte[4];
	Byte* temp = (Byte*)&c;
	for (int i=0; i<4; i++) {
		byte[i] = *temp++;
	}
	Data d;
	d.first = byte;
	d.second = 4;
	return d;
}

//字符数组转化为Data
Data RecordTool::str2Data(char* str, int size) {
	Data d;
	d.second = size;
	Byte* byte = new Byte[size];
	Byte* temp = (Byte*)str;
	for (int i=0; i<size; i++) {
		byte[i] = *temp++;
	}
	d.first = byte;
	return d;
}

int RecordTool::data2Int(Data d) {
	int c = 0;
	Byte* temp = (Byte*)&c;
	for (int i=0; i<d.second; i++) {
		*temp++ = d.first[i];
	}
	return c;
}

char* RecordTool::data2Str(Data d) {
	char* str = new char[d.second];
	char* temp = (char*)d.first;
	for (int i=0; i<d.second; i++) {
		str[i] = *temp++;
	}

	return str;
}

//将从byte首地址开始的sizg个byte转为int
// 1<= size <= 4
int RecordTool::byte2Int(const Byte* byte, int size) {
	assert(size>=1 && size <=4);
	int c = 0;
	Byte* temp = (Byte*)&c;
	for (int i=0; i<size; i++) {
		*temp++ = byte[i];
	}
	return c;
}


