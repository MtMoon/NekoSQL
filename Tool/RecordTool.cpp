/*
 * RecordTool.cpp
 *
 *  Created on: 2015-10-21
 *      Author: yxy
 */

#include "RecordTool.h"


//数据记录行采用SQL Server2000的数据行格式
//DP的顺序应与TableInfo中一致 先顺序FN个定长数据，再顺序VN个变长数据
//空串: Data.first = 全0bit(定长数据则定长的0比特流，变长数据则非NULL即可), Data.second = 0
//NULL: Data.first = NULL, Data.second = 0
Byte* RecordTool::makeRecord(TableInfo tb, int& len, DP data[], int size) {

	//计算记录行长度
	len = 9;
	int nullLen = ceil(double(tb.FN + tb.VN) / 8);
	len += nullLen;
	len += 2*tb.VN;
	for (int i=0; i<size; i++) {
		len += data[i].second.second;
	}
	Byte* line = new Byte[len];

	//make tag A 1Byte
	line[0] &= 0x00;
	line[0] |= (1<<4);

	if (tb.VN !=0) { //存在变长列
		line[0] |= (1<<5);
	}

	//去掉SQL Server 2000中的tagB，使用两个Byte存储该条数据的总长度
	Byte* temp = (Byte*)&len;
	line[1] &= *temp++;
	line[2] &= *temp;

	//定长部分长度
	int fsize = 0; //定长数据长度.
	for (int i=0; i<tb.FN; i++) {
		fsize += tb.Flen[i];
	}

	temp = (Byte*)&fsize;
	line[3] = *temp++;
	line[4] = *temp;

	//定长部分数据
	int count = 5;
	for (int i=0; i<tb.FN; i++) {
		if (data[i].second.second == 0) {
			for (int j=0; j<tb.Flen[i]; j++) {
						Byte tt;
						tt &= 0x00;
						line[count++] = tt;
			}
		} else {
			for (int j=0; j<data[i].second.second; j++) {
				line[count++] = data[i].second.first[j];
			}
		}
	}

	//列数
	int colNum = tb.FN + tb.VN;
	temp = (Byte*)&colNum;
	line[count++]  = *temp++;
	line[count++]  = *temp;

	//null位图

	for (int i=0; i<nullLen; i++) {

		Byte nmap;
		nmap &= 0x00;
		for (int j=0; j<8; j++) {
			if (data[j+i*8].second.first == NULL) {
				nmap |= (1<<j);
			}
		}
		line[count++] = nmap;;
	}

	//变列数目
	int vNum = tb.VN;
	temp = (Byte*)&vNum;
	line[count++]  = *temp++;
	line[count++]  = *temp;

	//列偏移数组
	int val = count + 2*tb.VN;
	for (int i=tb.FN; i<size; i++) {
			val = val + data[i].second.second;
			temp = (Byte*) &val;
			line[count++] = *temp++;
			line[count++] = *temp;
	}

	//变长列数据
	for (int i=tb.FN; i<size; i++) {
			for (int j=0; j<data[i].second.second; j++) {
				line[count++] = data[i].second.first[j];
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
int RecordTool::byte2Int(Byte* byte, int size) {
	assert(size>=1 && size <=4);
	int c = 0;
	Byte* temp = (Byte*)&c;
	for (int i=0; i<size; i++) {
		*temp++ = byte[i];
	}
	return c;
}

//获取某个定长字段的位置 first为其在数据行中的起始位置，，second表示其列序号
//注意，只限定长
LP RecordTool::getSegOffset(TableInfo& tb, string& segname) {
	int off = 0;
	int order = 0;
	for (int i=0; i<tb.FN; i++) {
		if (tb.Fname[i] == segname) {
			order = tb.Flen[i];
			break;
		}
		off += tb.Flen[i];
	}
	return LP(off+5, order);
}

