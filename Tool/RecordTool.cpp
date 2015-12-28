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
	//printf("lalala2\n");
	//计算记录行长度
	len = 9;
	int nullLen = ceil(double(tb.FN + tb.VN) / 8);
	len += nullLen;
	len += 2*tb.VN;

	for (int i=0; i<tb.FN; i++) {
		if (tb.types[i] == 0) {
			len += 4;
			continue;
		}
		len += tb.Flen[i];
	}

	for (int i=tb.FN; i<size; i++) {
		len += data[i].second.second;
	}

	printf("len: %d \n", len);

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
		if (tb.types[i] == 0) {
			fsize += 4;
			continue;
		}
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
			int tempSize = tb.Flen[i];
			if (tb.types[i] == 0) {
				tempSize = 4;
			}
			for (int j=0; j<tempSize; j++) {
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
	//printf("lalala2\n");
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
			if (j+i*8>=size) {
				break;
			}
			if (data[j+i*8].second.first == NULL) {
				nmap |= (1<<j);
			}
		}
		line[count++] = nmap;
	}
	//printf("bull byte in make %x \n", line[count-1]);

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

	//assert(len == count);

	//清除data数组中数据
	/*for (int i=0; i<size; i++) {
		delete [] data[i].second.first;
	}*/

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

//判断某条数据的某个字段是否满足特定值
bool RecordTool::hasSameSegVal(TableInfo& tb, Data record, ConDP condi, int cmpType) {
	ConDP fieldValue = getFieldValueInRecord(tb, record, condi.name);
	bool ans = false;

	if ((cmpType != 3 && fieldValue.isnull) || (condi.type != fieldValue.type)) {
		//printf("condi.type %d,  fieldValue.type %d \ n", condi.type, fieldValue.type);
		//printf("is null %d \n", fieldValue.isnull);
		return ans;
	}
	//printf("lalal \n");
	if (cmpType == 3) {
		if (fieldValue.isnull) {
			return true;
		} else {
			return false;
		}
	}

	if (cmpType == 5) {
		if (!fieldValue.isnull) {
			return true;
		} else {
			return false;
		}
	}

	//整数
	if (condi.type == 0) {
		//printf("age %d \n", fieldValue.value_int);
		if (cmpType == 0 && condi.value_int == fieldValue.value_int) {
			ans = true;
		} else if (cmpType == 1 &&  fieldValue.value_int  > condi.value_int) {
			ans = true;
		} else if (cmpType == 2 &&  fieldValue.value_int < condi.value_int ) {
			ans = true;
		} else if (cmpType == 4 &&  fieldValue.value_int != condi.value_int )  {
			ans = true;
		}
	} else if (condi.type == 1) { //字符串
		if (cmpType == 0 && condi.value_str == fieldValue.value_str) {
			ans = true;
		} else if (cmpType == 1 && fieldValue.value_str > condi.value_str ) {
			ans = true;
		} else if (cmpType == 2 && fieldValue.value_str < condi.value_str ) {
			ans = true;
		} else if (cmpType == 4 && fieldValue.value_str != condi.value_str ) {
			ans = true;
		}
	}
	return ans;
}

//获取某个记录行中某个字段的值
ConDP RecordTool::getFieldValueInRecord(TableInfo& tb, Data record, string fieldName) {
	Byte* line = NULL;
	ConDP ans;
	ans.isnull = true;
	ans.name = "";
	ans.type = -1;
	ans.value_int = 0;
	ans.value_str = "";



	if (record.first == NULL && record.second == 0) {
		return ans;
	}
	line = record.first;

	//获取非变长数据部分长度
	int nvlen = getNVLen(tb);

	//获取null位图
	int nulllen = ceil(double(tb.FN+tb.VN)/8);
	Byte nullByte[nulllen];
	copyByte(nullByte, line+nvlen-2-2*tb.VN-nulllen, nulllen);

	line = record.first;
	Byte* temp = NULL;
	int fieldType = -1;
	int start = 0, end = 0;

	if (isVCol(tb, fieldName)) {
		//找到该字段是第几个变长列
		int col = 0;
		for (int i=0; i<tb.VN; i++) {
			if (tb.Vname[i] == fieldName) {
				col = i;
				fieldType = tb.types[tb.FN+i];
				break;
			}
		}
		if (fieldType == 0) {
			ans.type = fieldType;
		} else if (fieldType == 1 || fieldType == 2) {
			ans.type = 1;
		}
		//判断是否为null
		int whichByte = (tb.FN+col) / 8;
		int whichBit = (tb.FN+col) % 8;
		int ifnull = (nullByte[whichByte] >> whichBit) & 1;
		if (ifnull == 1) {
			return ans;
		}

		start = 0;
		if (col == 0) {
			start = nvlen;
		} else {
			int coff = nvlen - 2*tb.VN + 2*(col-1); //下一个变长列的开始位置是上一个变长列的结束未知
			start = byte2Int(line+coff,2);
		}
		temp = line+nvlen-2*tb.VN+2*col;
		end  = RecordTool::byte2Int(temp, 2);

	} else { //是定长数据
		LP off = getSegOffset(tb, fieldName);
		temp = line + off.first;
		start = off.first;
		fieldType = tb.types[off.second];
		if (fieldType == 0) {
			ans.type = fieldType;
		} else if (fieldType == 1 || fieldType == 2) {
			ans.type = 1;
		}

		int whichByte = (off.second) / 8;
		int whichBit = (off.second) % 8;
		int ifnull = (nullByte[whichByte] >> whichBit) & 1;
		if (ifnull == 1) {
				return ans;
		}

		if (fieldType == 0) {
			end = start+4;
		} else {
			end = start + tb.Flen[off.second];
		}

	}

	//获取数据
	line = record.first;
	//printRecord(tb, record);
	int len = end - start;
	//printf("start: %d, end: %d fieldType: %d \n", start, end, fieldType);
	Byte data[len];
	copyByte(data, line+start, len);
	//printf("byte %d \n", (int)data[0]);

	ans.isnull = false;
	ans.name = fieldName;
	if (fieldType == 0) {
		ans.value_int = byte2Int(data,4);
	} else if (fieldType != -1) {
		char str[len+1];
		byte2Str(str, data, len);
		str[len] = '\0';
		//printf("lalala %s , len is %d\n", str, len);
		ans.value_str = string(str);
	}
	return ans;
}

int RecordTool::getNVLen(TableInfo& tb) {
	int nvlen = 9;
	nvlen += 2*tb.VN;
	nvlen += ceil(double(tb.FN+tb.VN)/8);
	for (int i=0; i<tb.FN; i++) {
		if (tb.types[i] == 0) {
			nvlen += 4;
			continue;
		}
		nvlen += tb.Flen[i];
	}
	return nvlen;
}

//字符数组转化为Data
Data RecordTool::str2Data(const char* str, int size) {
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

//把size个byte转为str, 要求str指向的数组大小不小于size
void RecordTool::byte2Str(char* str, Byte* byte, int size) {
	//printf("RecordTool::byte2Strn size: %d \n", size);
	char* temp = (char*)byte;
	for (int i=0; i<size; i++) {
		str[i] = *temp++;
	}
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
		if (tb.types[i] == 0) {
			off += 4;
			continue;
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

void RecordTool::str2Byte(Byte* byte, int size, const char* str) {
	int slen = strlen(str);
	Byte* temp = (Byte*) str;
	for (int i=0; i<slen; i++) {
			byte[i] = temp[i];
	}
	for (int i=slen; i<size; i++) {
		byte[i] = 0;
	}
}

int RecordTool::getRecordLen(Byte* byte)
{
	if (byte == NULL)
		return -1;
	return byte2Int(byte+1, 2);	
}

/*用于打印信息的工具函数*/
/*****************************************************************/

//在命令行打印一条记录，Data record 的first为记录行的Byte数组指针， second为该记录行的总长度
void RecordTool::printRecord(TableInfo& tb, Data record) {
	Byte* line = record.first;
	if (line == NULL || record.second <= 0) {
		printf("parameter error! \n");
		return;
	}
	Byte tagA = line[0];
	int len = byte2Int(line+1, 2);
	if (len != record.second) {
		printf("record length error! \n");
		return;
	}
	//打印record信息
	printf("--------------------Record Info----------------------\n");
	//tag A
	printf("Tag: ");
	for (int i=0; i<8; i++) {
		printf("%d ", (tagA>>i) & 1);
	}
	printf("\n");

	//数据总长度
	printf("record line total line length: %d\n", len);

	//行定长数据总长度
	int flen = byte2Int(line+3,2);
	printf("fixed data length: %d\n", flen);

	//null 位图
	printf("null bit map: ");
	int nullLen = ceil(double(tb.FN+tb.VN)/8);
	Byte nullmap[nullLen];
	int off = 7+flen;
	copyByte(nullmap, line+off, nullLen);
	//printf("num byte %x \n", nullmap[0]);
	for (int i=0; i<nullLen; i++) {
		for (int j=0; j<8; j++) {
			printf("%d ", (nullmap[i]>>j) & 1);
		}
	}
	printf("\n");

	//定长数据
	printf("fixed data: \n");
	off = 5;
	for (int i=0; i<tb.FN; i++) {
		printf("%s   ", tb.Fname[i].c_str());
		if (tb.types[i] == 0) {
			printf("%d \n", byte2Int(line+off,4));
			off += 4;
		} else {
			printf("%s \n", data2Str(Data(line+off,tb.Flen[i])));
			off += tb.Flen[i];
		}
	}
	//变长数据
	printf("varied data: \n");
	int vdataoff = getNVLen(tb); //变长数据起始
	off = vdataoff-2*tb.VN; //列偏移数组起始

	for (int i=0; i<tb.VN; i++) {
		printf("%s :  ", tb.Vname[i].c_str());
		int vlen = byte2Int(line+off, 2);
		//printf("vlen:%d \n", vlen);
		//printf("vdataoff:%d \n", vdataoff);
		if (vlen-vdataoff<=0) {
			off += 2;
			continue;
		}
		printf("%s \n", data2Str(Data(line+vdataoff, vlen-vdataoff)));
		off += 2;
		vdataoff = vlen;
	}
	printf("-----------------------------------------------------------------------\n");

}


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
