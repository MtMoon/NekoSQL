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
	currentTable = "";
	currentFileID = -1;
	tables.clear();
}

DataManager::~DataManager() {
	closeTable(currentTable.c_str());
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
	bool success = fm->createFile(filename);
	int len = currentBase.length()+1;
	string newFilename(filename+len+9);
	newEmptySpecialPage(newFilename.c_str());
	return success;
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
bool DataManager::insertRecord(const char* tablename, DP data[], const int size, LP& rpos) {
	
	int len = 0; //数据记录长度
	Byte* record = NULL;
	TableInfo tb = getTableInfo(tablename);
	//生成数据记录
	record = RecordTool::makeRecord(tb, len, data, size);

	//cout << "insert Record, len: " << len << endl;
	//cout << "len from byte: " << RecordTool::byte2Int(record+1, 2)  << endl;

	if (len > PAGE_SIZE-6)
		return false;
	// 寻找合适的页插入
	int pageNum =  getPageNum(tablename);
	openTable(tablename);

	//遍历除了第一页之外的各个页
	//获取有空余byte的页的index
	int pageindex = 0, pageID = -1;
	for (int i=1; i<pageNum; i++) {
		bm->getPage(currentFileID, i, pageindex);
		if (getPageLeftSize(pageindex) >= len+2) {
			pageID = i;	
			break;
		}
	}

	if (pageID != -1)
	{
		rpos.first = pageID;
		Byte* buf = (Byte*)(bm->getPage(currentFileID, pageID, pageindex));
		int spaceLeft = RecordTool::byte2Int(buf, 2);
		int slotNum = RecordTool::byte2Int(buf+2, 2);
		int start = PAGE_SIZE-slotNum*2-spaceLeft;
		bool flag = false;
		for (int i = 0; i < slotNum; i++)
		{
			int slotOffset = PAGE_SIZE-(i+1)*2;
			int slotVal = RecordTool::byte2Int(buf+slotOffset, 2);
			if (slotVal == -1)
			{
				RecordTool::int2Byte(buf+slotOffset, 2, start);
				RecordTool::copyByte(buf+start, record, len);
				RecordTool::int2Byte(buf, 2, spaceLeft-len);
				rpos.second = i;
				flag = true;
				break;
			}
		}
		if (!flag)
		{
			RecordTool::int2Byte(buf+PAGE_SIZE-(slotNum+1)*2, 2, start);
			RecordTool::copyByte(buf+start, record, len);
			RecordTool::int2Byte(buf, 2 ,spaceLeft-len-2);
			RecordTool::int2Byte(buf+2, 2, slotNum+1);
		}
	}
	else
	{
		pageID = newNormalPage(tablename, currentFileID);
		rpos.first = pageID;
		Byte* buf = (Byte*)(bm->getPage(currentFileID, pageID, pageindex));
		int spaceLeft = RecordTool::byte2Int(buf, 2);
		RecordTool::int2Byte(buf+PAGE_SIZE-2, 2, 96); //changed by yxy
		RecordTool::copyByte(buf+96, record, len); //changed by yxy
		RecordTool::int2Byte(buf, 2, spaceLeft-len-2);
		RecordTool::int2Byte(buf+2, 2, 1);
		rpos.second = 0;
	}
	bm->markDirty(pageindex);
	bm->writeBack(pageindex);
	//fm->closeFile(fileID);
	return true;
}

//LP为插入的位置
//data的first为整条记录的Byte数组，second为该Byte数组的长度
bool DataManager::insertRecord(const char* tablename, Data data, LP pos) {
	int pageNum =  getPageNum(tablename);
	openTable(tablename);

	int pageID = pos.first, slotID = pos.second;
	if (pageID < 1 || pageID >= pageNum)
	{
		closeTable(tablename);
		return false;
	}
	int pageindex;
	Byte* buf = (Byte*)(bm->getPage(currentFileID, pageID, pageindex));
	int spaceLeft = RecordTool::byte2Int(buf, 2);
	int slotNum = RecordTool::byte2Int(buf+2, 2);
	if (data.second > spaceLeft)
	{
		closeTable(tablename);
		return false;
	}
	if (slotID < 0 || slotID >= slotNum)
	{
		closeTable(tablename);
		return false;
	}
	int slotOffset = PAGE_SIZE-(slotID+1)*2;
	int slotVal = RecordTool::byte2Int(buf+slotOffset, 2);
	if (slotVal != -1)
	{
		closeTable(tablename);
		return false;
	}

	int start = PAGE_SIZE-slotNum*2-spaceLeft;
	RecordTool::int2Byte(buf+slotOffset, 2, start);
	RecordTool::copyByte(buf+start, data.first, data.second);
	RecordTool::int2Byte(buf, 2, spaceLeft-data.second);
	
	bm->markDirty(pageindex);
	bm->writeBack(pageindex);
	//fm->closeFile(fileID);
	return true;
}

bool DataManager::deleteRecord(const char* tablename, LP pos) {
	int pageNum =  getPageNum(tablename);
	openTable(tablename);

	int pageID = pos.first, slotID = pos.second;
	if (pageID < 1 || pageID >= pageNum)
	{
		closeTable(tablename);
		return false;
	}
	int pageindex;
	Byte* buf = (Byte*)(bm->getPage(currentFileID, pageID, pageindex));
	int spaceLeft = RecordTool::byte2Int(buf, 2);
	int slotNum = RecordTool::byte2Int(buf+2, 2);
	if (slotID < 0 || slotID >= slotNum)
	{
		closeTable(tablename);
		return false;
	}
	int slotOffset = PAGE_SIZE-(slotID+1)*2;
	int slotVal = RecordTool::byte2Int(buf+slotOffset, 2);
	if (slotVal == -1)
	{
		closeTable(tablename);
		return false;
	}

	int newStart = slotVal;
	int len = RecordTool::getRecordLen(buf+newStart);
	int start = newStart+len;
	int maxOffset = 0;
	for (int i = 0; i < slotNum; i++)
	{
		int tempOffset = PAGE_SIZE-(i+1)*2;
		int tempVal = RecordTool::byte2Int(buf+tempOffset, 2);
		if (tempVal != -1 && tempVal > maxOffset)
			maxOffset = tempVal;
	}
	int end = maxOffset + RecordTool::getRecordLen(buf+maxOffset);
	RecordTool::copyByte(buf+newStart, buf+start, end-start);
	RecordTool::int2Byte(buf+slotOffset, 2, -1);
	for (int i = 0; i < slotNum; i++)
	{
		int tempOffset = PAGE_SIZE-(i+1)*2;
		int tempVal = RecordTool::byte2Int(buf+tempOffset, 2);
		if (tempVal != -1 && tempVal > newStart)
			RecordTool::int2Byte(buf+tempOffset, 2, tempVal-len);
	}
	RecordTool::int2Byte(buf, 2, spaceLeft+len);
	
	bm->markDirty(pageindex);
	bm->writeBack(pageindex);
	//fm->closeFile(fileID);
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
		if (tb.types[i] == 0) {
			flen += 4;
			continue;
		}
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

		//修改数据总长度
		int totalLen = vlen + nvlen;
		temp = (Byte*)&totalLen;
		line[1] = *temp++;
		line[2] = *temp;

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
		//cout << "in update vlen+nvlen: " << vlen + nvlen << endl;
		ans = insertRecord(tablename, Data(line, vlen+nvlen), pos);
	} else {
		ans = insertRecord(tablename, d, pos);
	}

	return ans;


}

//获取属性值满足特定条件的记录
vector<LP> DataManager::searchRecord(const char*tablename, ConDP condi,  int cmpType) {
	int pageNum =  getPageNum(tablename);

	//cout << "filepath: " << filepath << endl;
	openTable(tablename);
	//遍历除了第一页之外的各个页
	//获取有空余byte的页的index
	int pageindex = 0;
	vector<LP> vec;
	if (cmpType != 0 &&  cmpType != 1 && cmpType != 2 && cmpType != 3) {
		return vec;
	}
	for (int i=1; i<pageNum; i++) {
		vector<LP> tempVec = searchRecordInPage(tablename, i, condi,  cmpType);
		vec.insert(vec.begin(), tempVec.begin(), tempVec.end());
	}
	return vec;
}

vector<LP> DataManager::searchRecordInPage(const char* tablename, const int pageorder, ConDP condi,  int cmpType) {
	vector<LP> vec;
	if (cmpType != 0 &&  cmpType != 1 && cmpType != 2 && cmpType != 3) {
		return vec;
	}

	int pageindex = -1;
	openTable(tablename);
	bm->getPage(currentFileID, pageorder, pageindex);
	Byte* page = (Byte*)bm->addr[pageindex];
	TableInfo tb = getTableInfo(tablename);
	//取出这一页的槽数
	Byte* temp = NULL;
	temp = page+2;
	int slotNum = 0;
	slotNum = RecordTool::byte2Int(temp, 2);
	//cout << "slot Num: " << slotNum << endl;
	for (int i=0; i<slotNum; i++) {
		Data d = getRecordByLP(tablename,  LP(pageorder,i));
		if (d.first == NULL && d.second == 0) {
			continue;
		}
		if (RecordTool::hasSameSegVal(tb, d, condi, cmpType)) {
			vec.push_back(LP(pageorder,i));
		}
	}
	return vec;
}


//根据位置和表名获取一条数据
//当该slot被删除，即start为-1时，返回Data(NULL,0);
Data DataManager::getRecordByLP(const char* tablename, LP pos) {
	openTable(tablename);
	int pageindex = 0;
	bm->getPage(currentFileID, pos.first, pageindex);
	//cout << "pageIndex: " << pageindex << endl;
	Byte* page = (Byte*)bm->addr[pageindex];



	//获取记录起始位置
	int start = 0;
	Byte* temp = page+PAGE_SIZE-2*(pos.second+1);
	start = RecordTool::byte2Int(temp, 2);
	//cout << "location: " << pos.first << " " << pos.second << endl;
	//cout << "start: " << start << endl;
	if (start == -1) {
		return Data(NULL,0);
	}
	//获取记录长度
	int len = 0;
	temp = page+start+1;
	len = RecordTool::byte2Int(temp, 2);
	//cout << "len: " << len << endl;
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
	if (tables.find(string(name)) != tables.end()) {
		return tables[string(name)];
	} else {
		//cout << "not fount in map" << endl;
		return loadTableInfo(tablename);
	}
}

//加载表当前的页数
int DataManager::getPageNum(const char* tablename) {
	//获取文件的大小再除以8k即可
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath + ".data";

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
	openTable(tablename);
	int pageindex = 0;

	Byte* page = (Byte*)bm->getPage(currentFileID, 0, pageindex);
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
	tables.insert(pair<string, TableInfo>(string(tablename), tb));
	return tb;
}

//写入表元信息，供SysManager在创建表时使用
void DataManager::writeTableInfo(string tableName, TableInfo tb) {
	string filepath;
	filepath = "DataBase/" + currentBase + "/" + tableName + ".data";
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
	fm->closeFile(fileID);
	bm->close();


}

//无效化tables中信息
void DataManager::invalidTbMap(string tbName) {
	map<string, TableInfo>::iterator it = tables.find(tbName);
	if (it != tables.end()) {
		tables.erase(it);
	}
}

//获取一个表里所有有效数据的LP
vector<LP> DataManager::getAllLPInTable(const char* tablename) {
	vector<LP> vec;
	openTable(tablename);
	int pageNum =  getPageNum(tablename);
	for (int i=0; i<pageNum; i++) {
		int pageindex = 0;
		bm->getPage(currentFileID, i, pageindex);
		Byte* page = (Byte*)bm->addr[pageindex];
		int slotNum = RecordTool::byte2Int(page+2, 2);
		//遍历槽
		for (int j=0; j<slotNum; j++) {
			int start = 0;
			Byte* temp = page + PAGE_SIZE - 2*(j+1);
			start = RecordTool::byte2Int(temp, 2);
			if (start>=96) {
				vec.push_back(LP(i,j));
			}
		}
	}


	return vec;
}

//获取一个表里的所有key pair，用于索引模块
vector<KP> DataManager::getAllKPInTable(const char* tablename, string fieldName) {
	vector<KP> ans;
	vector<LP> vec = getAllLPInTable(tablename);
	int vecSize = vec.size();
	TableInfo tb = getTableInfo(tablename);
	//cout << "tablename: " << tablename << endl;
	for (int i=0; i<vecSize; i++) {
		//cout << "getAllKPInTable, pos: " << vec[i].first << " " << vec[i].second << endl;
		Data d = getRecordByLP(tablename, vec[i]);
		ConDP key = RecordTool::getFieldValueInRecord(tb, d, fieldName);
		//cout << "getAllKPInTable: " <<  key.value_str << endl;
		ans.push_back(KP(vec[i], key));
	}

	return ans;
}

/**
 * return 0: 该表当前已打开 1:该表当前未打开，已重新打开表
 */
int DataManager::openTable(const char* tableName) {
	string tbStr = string(tableName);
	if (tbStr != currentTable) {
		if ("" != tbStr && currentFileID != -1 && currentTable != "") {
			//cout << "close table: " << currentTable << endl;
			fm->closeFile(currentFileID);
			bm->close();
		}
		string filepath = "DataBase/" + currentBase + "/" + tbStr + ".data";
		//cout << "filepath: " << filepath << endl;
		fm->openFile(filepath.c_str(), currentFileID);
		currentTable = tbStr;
		return 1;
	}
	assert(currentFileID >= 0);
	return 0;
}

/**
 * return 0 该表未打开过   1:该表当前打开,成功关闭
 */
int DataManager::closeTable(const char* tableName) {
	string tbStr = string(tableName);
	if (tbStr != currentTable || currentFileID == -1) {
		return 0;
	}

	fm->closeFile(currentFileID);
	bm->close();
	currentFileID = -1;
	currentTable = "";
	return 1;

}

/**********************************************************************/
bool DataManager::newEmptySpecialPage(const char* tablename)
{
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath + ".data";
	int fileID;
	fm->openFile(filepath.c_str(), fileID);
	int index;
	bm->allocPage(fileID, 0, index, false);
	bm->markDirty(index);
	bm->writeBack(index);
	fm->closeFile(fileID);
	bm->close();
	return true;
}


int DataManager::newNormalPage(const char* tablename, int fileID)
{
	int pageNum = getPageNum(tablename);
	int index;
	BufType b = bm->allocPage(fileID, pageNum, index, false);
	Byte* buf = (Byte*)b;
	RecordTool::int2Byte(buf, 2, PAGE_SIZE-96); //changed by yxy
	RecordTool::int2Byte(buf+2, 2, 0);
	bm->markDirty(index);
	bm->writeBack(index);
	return pageNum;
}


void DataManager::pageInfo(const char* tablename, int pageID)
{
	string filepath(tablename);
	filepath = "DataBase/" + currentBase + "/" + filepath;
	int fileID;
	fm->openFile(filepath.c_str(), fileID);
	if (pageID < 1 || pageID >= getPageNum(tablename))
		cout << "invalid pageID" << endl;
	else
	{
		TableInfo tableInfo = getTableInfo(tablename);
		int pageindex;
		Byte* buf = (Byte*)(bm->getPage(fileID, pageID, pageindex));
		int spaceLeft = RecordTool::byte2Int(buf, 2);
		int slotNum = RecordTool::byte2Int(buf+2, 2);
		cout << "Table " << string(tablename) << " Info:" << endl;
		cout << "space left:" << spaceLeft << endl;
		cout << "slot num:" << slotNum << endl;
		cout << "Following is data:" << endl;
		for (int i = 0; i < slotNum; i++)
		{
			int offset = PAGE_SIZE-(i+1)*2;
			int start = RecordTool::byte2Int(buf+offset, 2);
			if (start != -1)
			{
				Data data = getRecordByLP(tablename, LP(pageID, i));
				RecordTool::printRecord(tableInfo, data);
			}
			cout << endl;
		}
		
	}
	fm->closeFile(fileID);
	bm->close();
}
