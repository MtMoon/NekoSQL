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
	tables.clear();
}

DataManager::~DataManager() {
	delete fm;
	delete bm;
}

//更上层的接口函数
void DataManager::setDatabase(string dirname) {
	if (currentBase != dirname) {
		currentBase = dirname;
		tables.clear();
	}
}

string DataManager::getCurrentDBName() {
	return currentBase;

}

// 文件操作函数
//注意！！调用文件操作函数时，必须保证currentBase所示目录已存在！
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
	filepath = "DataBase/" + currentBase + "/" + filepath;

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

	//获取槽数



	//修改剩余字节数

	return true;


}

//LP为插入的位置
//data的first为整条记录的Byte数组，second为该Byte数组的长度
bool DataManager::insertRecord(const char* tablename, Data data, LP pos) {
	return true;
}

bool DataManager::deleteRecord(const char* tablename, LP pos) {
	return true;
}

//更新记录
//需要修改null位图和列偏移数组
//直接获取内存页中须更新数据的指针
//若需更新数据全是定长，则直接修改缓存页
//若有变长，则先删除后插入
bool DataManager::updateRecord(const char* tablename, LP pos, DP data[], int size) {

	//获取记录
	Data d = getRecordByLP(tablename,pos);
	TableInfo tb = getTableInfo(tablename);

	int nvlen = 9; //非变长数据部分的字段
	int flen = 0;
	for (int i=0; i<tb.FN; i++) {
		flen += tb.Flen[i];
	}
	nvlen += flen;
	nvlen += ceil(double(tb.FN + tb.VN) / 8);
	nvlen += 2*tb.VN;
	int offpos = nvlen - 2*tb.VN; //列偏移数组起始位置

	int nstart = 7+flen; //null位图起始位置
	Byte* temp = NULL;

	//更新定长字段
	bool flag = false; //是否有变长列需要更新
	for (int i=0; i<size; i++) {
		if (RecordTool::isVCol(tb, data[i].first)) {
			flag = true;
			continue;
		}
		LP p = RecordTool::getSegOffset(tb, data[i].first);
		//修改定长数据
		temp = d.first + p.first;
		for (int j=0; j<data[i].second.second; j++) {
			*temp++ = data[i].second.first[j];
		}
		//更新NULL位图
		int nullbyte = nstart + p.second / 8;
		int whichbit = p.second % 8;
		*(d.first + nullbyte) |= ~(1<<whichbit);

	}

	bool ans = true;


	if (flag) { //如果有变长数据
		//更新变长字段

			Data vdata[tb.VN];
			int lastpos = nvlen;
			for (int i=0; i<tb.VN; i++) {
				int c = 0;
				temp = d.first+offpos+2*i;
				c = RecordTool::byte2Int(temp, 2);
				vdata[i].first = d.first + c;
				vdata[i].second = c - lastpos;
				lastpos = c;
			}

			int vlen = 0;
			for (int i=0; i<tb.VN; i++) {
				for (int j=0; j<size; j++) {
					if (tb.Vname[i] == data[j].first) {

						//修改NULL位图
						int nullbyte = nstart + (tb.FN+i) / 8;
						int whichbit = (tb.FN+i) % 8;
						*(d.first + nullbyte) |= ~(1<<whichbit);

						vdata[i] = data[i].second;
						break;
					}
				}
			}

			for (int i=0; i<tb.VN; i++) {
				vlen += vdata[i].second;
			}

			Byte* line = new Byte[vlen+nvlen];
			temp = d.first;
			for (int i=0; i<nvlen; i++) { //拷贝非变长数据部分
				line[i] = *temp++;
			}

			//拷贝变长数据和修改列偏移数组
			int vstart = nvlen;
			for (int i=0; i<tb.VN; i++) {
				for (int j=0; j<vdata[i].second; j++) {
					line[vstart++] = vdata[i].first[j];
				}
				//修改列偏移数组
				Byte* offstart = line + offpos + 2*i;
				int c = vstart;
				Byte* tempc = (Byte*)&c;
				*offstart++ = tempc[0];
				*offstart = tempc[1];
			}
			deleteRecord(tablename, pos);
			ans = insertRecord(tablename, Data(line, vlen+nvlen), pos);
	} else {
			ans = insertRecord(tablename, d, pos);
	}

	return ans;


}

//获取属性值满足特定条件的记录
vector<LP> DataManager::searchRecord(const char*tablename, DP condi) {
	int pageNum =  getPageNum(tablename);
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath;

	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);
	Byte* temp = NULL;

	//遍历除了第一页之外的各个页
	//获取有空余byte的页的index
	int pageindex = 0;
	vector<LP> vec;
	TableInfo tb = getTableInfo(tablename);
	for (int i=1; i<pageNum; i++) {
		bm->getPage(fileID, i, pageindex);
		Byte* page = (Byte*)bm->addr[pageindex];
		//取出这一页的槽数
		temp = page+2;
		int slotNum = 0;
		slotNum = RecordTool::byte2Int(temp, 2);
		for (int j=0; j<slotNum; j++) {
			if (hasSameSegVal(tb, tablename, LP(i,j), condi)) {
				vec.push_back(LP(i,j));
			}
		}
	}

	return vec;
}

//判断某条数据的某个字段是否满足特定值
bool DataManager::hasSameSegVal(TableInfo& tb, const char* tablename, LP pos, DP condi) {
	Byte* line = NULL;
	Data d;
	d = getRecordByLP(tablename, pos);
	line = d.first;
	bool ans = false;
	Byte* temp = NULL;

	if (RecordTool::isVCol(tb, condi.first)) {
		//找到该字段是第几个变长列
		int col = 0;
		for (int i=0; i<tb.VN; i++) {
			if (tb.Vname[i] == condi.first) {
				col = i;
				break;
			}
		}

		//获取非变长数据部分长度
		int nvlen = 9;
		nvlen += 2*tb.VN;
		nvlen += ceil(double(tb.FN+tb.VN)/8);
		for (int i=0; i<tb.FN; i++) {
			nvlen += tb.Flen[i];
		}

		int start = 0;
		if (col == 0) {
			start = nvlen;
		} else {
			int coff = nvlen - 2*tb.VN + 2*(col-1);
			temp = line+coff;
			start = RecordTool::byte2Int(temp,2);
		}

		temp = line+nvlen-2*tb.VN+2*col;
		int end  = RecordTool::byte2Int(temp, 2);

		int vlen = end-start;

		if (vlen != condi.second.second) {
			ans = false;
			return ans;
		}

		temp = line + start;
		for (int i=0; i<vlen; i++) {
			if (condi.second.first[i] != *temp++) {
				ans = false;
				break;
			}
		}

	} else { //是定长数据
		LP off = RecordTool::getSegOffset(tb, condi.first);
		temp = line + off.first;
		if (condi.second.second != tb.Flen[off.second]) {
			ans = false;
			return ans;
		}
		for (int i=0; i<condi.second.second; i++) {
			if (condi.second.first[i] != *temp++) {
				ans = false;
				break;
			}
		}
	}

	delete[] line;
	return ans;
}

//根据位置和表名获取一条数据
Data DataManager::getRecordByLP(const char* tablename, LP pos) {
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath;
	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);
	int pageindex = 0;
	bm->getPage(fileID, pos.first, pageindex);

	Byte* page = (Byte*)bm->addr[pageindex];



	//获取记录起始位置
	int start = 0;
	Byte* temp = page+PAGE_SIZE-2*(pos.second+1);
	start = RecordTool::byte2Int(temp, 2);

	//获取记录长度
	int len = 0;
	temp = page+start+1;
	len = RecordTool::byte2Int(temp, 2);
	Byte* record = new Byte[len];
	temp = page+start;
	for (int i=0; i<len; i++) {
		record[i] = *temp++;
	}
	return Data(record,len);
}


int DataManager::getPageLeftSize(int pageindex) {
	BufType page = bm->addr[pageindex];
	//page头的前2个byte用来表示剩余字节数
	return RecordTool::byte2Int((Byte*)page, 2);
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
	filepath = "DataBase/" + currentBase + "/" + filepath;

	struct stat buf;
	if(stat(filepath.c_str(), &buf)<0) {
		return 0;
	 }
	//cout << buf.st_size << endl;
	return ceil(double(buf.st_size) / PAGE_SIZE);
}

//加载表的信息，存于tables map并返回
TableInfo DataManager::loadTableInfo(const char* tablename) {
	//cout << "enter loadTableInfo" << endl;
	TableInfo tb;
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath;
	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);
	int pageindex = 0;

	Byte* page = (Byte*)bm->getPage(fileID, 0, pageindex);
	Byte* temp = page;
	//cout << "lalala1" << endl;
	//get FN
	int fn = 0;
	fn  =  RecordTool::byte2Int(temp,2);
	//cout << "fn: " << fn << endl;


	//get vn
	int vn = 0;
	temp = page;
	vn =  RecordTool::byte2Int(temp+2,2);

	//cout << "vn: " << vn << endl;


	tb.Fname = new string[fn];
	tb.Vname = new string[vn];
	tb.Flen = new int[fn];
	tb.Vlen = new int[vn];
	tb.keys = new int[vn+fn];
	tb.types = new int[vn+fn];


	//获取定长列名
	int off = 4;
	for (int i=0; i<fn; i++) {
		char* name = NULL;
		temp = page;
		name =  RecordTool::data2Str(Data(temp+off ,24));
		tb.Fname[i] = string(name);
		//cout << "fname: " << tb.Fname[i] << endl;
		off += 24;
	}

	//获取变长列名
	for (int i=0; i<vn; i++) {
			char* name = NULL;
			temp = page;
			name =  RecordTool::data2Str(Data(temp+off ,24));
			tb.Vname[i] = string(name);
			//cout << "vname: " << tb.Vname[i] << endl;
			off += 24;
	}

	//获取定长数据长度
	for (int i=0; i<fn; i++) {
		int t = 0;
		temp = page;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Flen[i] = t;
		//cout << "flen: " << tb.Flen[i] << endl;
		off += 4;
	}

	//获取变长数据长度
	for (int i=0; i<vn; i++) {
		int t = 0;
		temp = page;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Vlen[i] = t;
		//cout << "vlen: " << tb.Vlen[i] << endl;
		off += 4;
	}

	//获取key数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = page;
		t = RecordTool::byte2Int(temp+off,1);
		tb.keys[i] = t;
		//cout << "keys: " << tb.keys[i] << endl;
		off += 1;
	}

	//获取types数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = page;
		t = RecordTool::byte2Int(temp+off,1);
		tb.types[i] = t;
		//cout << "types: " << tb.types[i] << endl;
		off += 1;
	}

	tb.FN = fn;
	tb.VN = vn;

	//获取null位图
	int nlen = ceil(double(tb.FN + tb.VN) / 8);
	//cout << "nlen:" << nlen << endl;
	tb.nullMap = new Byte[nlen];
	for (int i=0; i<nlen; i++) {
		tb.nullMap[i] = *(page+off);
		//cout << "nullmap: " << int(tb.nullMap[i]) << endl;
		off++;
	}



	return tb;
}

//写入表元信息，供SysManager在创建表时使用
void DataManager::writeTableInfo(string tableName, TableInfo tb) {
	string filepath;
	filepath = "DataBase/" + currentBase + "/" + tableName;
	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);
	int pageindex = 0;
	bm->getPage(fileID, 0, pageindex);
	Byte* page = (Byte*)bm->addr[pageindex];

	//计算表元信息大小
	int metaSize = 0;
	metaSize = 4 + 24*tb.FN + 24*tb.VN + 4*tb.FN + 4*tb.VN + 2*(tb.FN+tb.VN) + ceil(double(tb.FN+tb.VN)/8);

	Byte metaData[metaSize];
	for (int i=0; i<metaSize; i++) {
		metaData[i] &= 0x00;
	}
	Byte* temp = NULL;

	//set FN
	temp = (Byte*)&tb.FN;
	metaData[0] = *temp++;
	metaData[1] = *temp;


	//set vn
	temp = (Byte*)&tb.VN;
	metaData[2] = *temp++;
	metaData[3] = *temp;

	int off = 4;
	//设置定长列名
	for (int i=0; i<tb.FN; i++) {
		temp = (Byte*)tb.Fname[i].data();
		for (int j=0; j<tb.Fname[i].length(); j++) {
			metaData[off+j] = *temp++;
		}
		off += 24;
	}

	//设置变长列名
	for (int i=0; i<tb.VN; i++) {
		temp = (Byte*)tb.Vname[i].data();
		for (int j=0; j<tb.Vname[i].length(); j++) {
			metaData[off+j] = *temp++;
		}
		off += 24;
	}

	//设置定长数据长度
	for (int i=0; i<tb.FN; i++) {
		temp = (Byte*)&tb.Flen[i];
		for (int j=0; j<4; j++) {
			metaData[off+j] = *temp++;
		}
		off += 4;
	}

	//设置变长数据长度
	for (int i=0; i<tb.VN; i++) {
		temp = (Byte*)&tb.Vlen[i];
		for (int j=0; j<4; j++) {
			metaData[off+j] = *temp++;
		}
		off += 4;
	}

	//设置key数据
	for (int i=0; i<tb.VN+tb.FN; i++) {
		temp = (Byte*)&tb.keys[i];
		metaData[off] = *temp;
		off += 1;
	}

	//设置types数据
	for (int i=0; i<tb.VN+tb.FN; i++) {
		temp = (Byte*)&tb.types[i];
		metaData[off] = *temp;
		off += 1;
	}

	//设置null位图
	int nlen = ceil(double(tb.FN + tb.VN) / 8);
	for (int i=0; i<nlen; i++) {
		metaData[off] = tb.nullMap[i];

		off++;
	}
	//cout << "metaSize: " << metaSize << endl;
	//cout << "off: " << off << endl;
	assert(off == metaSize);
	//写入数据
	for (int i=0; i<metaSize; i++) {
		page[i] = metaData[i];
	}
	bm->markDirty(pageindex);
	bm->writeBack(pageindex);


}

//无效化tables中信息
void DataManager::invalidTbMap(string tbName) {
	map<string, TableInfo>::iterator it = tables.find(tbName);
	if (it != tables.end()) {
		tables.erase(it);
	}
}

