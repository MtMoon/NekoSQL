/*
 * RecordTool.cpp
 *
 *  Created on: 2015-10-21
 *      Author: yxy
 */

#include "RecordTool.h"

using namespace std;

//数据记录行采用SQL Server2000的数据行格式
//DP的顺序应与TableInfo中一致 先顺序FN个定长数据，再顺序VN个变长数据
//注意，传入的DP数组中的Data结构体中的Byte指针所指空间会在该函数中被释放
//返回的Byte*指向存有整条record的堆空间，请调用该函数者进行释放
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

	//make tag  1Byte
	line[0] &= 0x00;
	line[0] |= (1<<4);

	if (tb.VN !=0) { //存在变长列
		line[0] |= (1<<5);
	}

	//去掉SQL Server 2000中的tagB，使用两个Byte存储该条数据的总长度
	Byte* temp = (Byte*)&len;
	line[1] = *temp++;
	line[2] = *temp;

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
	//注意，data中的顺序需要按建表顺序先定长后变长
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

	//定长列数
	int colNum = tb.FN;
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
		line[count++] = nmap;
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
			int tv = val;
			temp = (Byte*) &tv;
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
	int slen = strlen(str);
	for (int i=0; i<slen; i++) {
		byte[i] = *temp++;
	}
	for (int i=slen; i<size; i++) {
		byte[i] = 0;
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

//将从byte首地址开始的size个byte转为int
// 1<= size <= 4
int RecordTool::byte2Int(Byte* byte, int size) {
	assert(size>=1 && size <=4);
	int c = 0;
	//c &= 0x00;
	Byte* temp = (Byte*)&c;
	for (int i=0; i<size; i++) {
		temp[i] = byte[i];
	}
	Byte token = '\x80';
	Byte ans = byte[size-1]&token;
	if (ans != '\0')
		for (int i = size; i < 4; i++)
			temp[i] = '\xff';
	return c;
}

//获取某个定长字段的位置 first为其在数据行中的起始位置，second表示其列序号
//注意，只限定长
LP RecordTool::getSegOffset(TableInfo& tb, string& segname) {
	int off = 0;
	int order = 0;
	for (int i=0; i<tb.FN; i++) {
		if (tb.Fname[i] == segname) {
			order = i;
			break;
		}
		off += tb.Flen[i];
	}
	return LP(off+5, order);
}


/******************************************************/

void RecordTool::copyByte(Byte* dst, const Byte* src, int len)
{
	for (int i = 0; i < len; i++)
		dst[i] = src[i];
}

void RecordTool::int2Byte(Byte* byte, int size, int num)
{
	if (size < 1 || size > 4)
		return;
	Byte* temp = (Byte*)&num;
	for (int i = 0; i < size; i++)
		byte[i] = temp[i];
}

int RecordTool::getRecordLen(Byte* byte)
{
	if (byte == NULL)
		return -1;
	return byte2Int(byte+1, 2);	
}

/*用于打印信息的工具函数*/
/*****************************************************************/
//在命令行打印表信息
string RecordTool::printTableInfo(const TableInfo& tb) {
	/*stringstream ss;
	ss.clear();
	cout << "定长列信息：" << endl;
	for (int i=0; i<tb.FN; i++) {
		int a = i/8;
		int b = i%8;
		std::cout << "field name: " << tb.Fname[i] << " field size: " << tb.Flen[i] << " field type: " << tb.types[i] << " field key: " << tb.keys[i] << " field null: " << (（tb.nullMap[a]>>b)  & 1)<< std::endl;
	}

	std::cout << "变长信息：" << std::endl;
	for (int i=0; i<tb.VN; i++) {
			int a = (tb.FN+i)/8;
			int b = (tb.FN+i)%8;
			std::cout << "field name: " << tb.Vname[i] << " field size: " << tb.Vlen[i] << " field type: " << tb.types[tb.FN+i] << " field key: " << tb.keys[tb.FN+i] << " field null: " << ((tb.nullMap[a]>>b)  & 1)<< std::endl;
	}*/
}
