/*
 * IndexManager.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#include "IndexManager.h"

IndexManager::IndexManager() {
	dfm = new FileManager();
	dbm = new BufPageManager(dfm);

	ifm = new FileManager();
	ibm = new BufPageManager(ifm);

	currentIndexInfo.legal = false;

	currentDB = "";
	currentTable = "";
	currentIndex = "";
	currentFileID = -1;
	hot = 0;
	_order = 80;
	lower_bound = ceil(double(_order) / 2) -1;
	upper_bound = _order-1;
}

IndexManager::~IndexManager() {
	delete dfm;
	delete dbm;

	delete ifm;
	delete ibm;
}

//供用户调用的定义函数

//创建索引
//返回1为创建成功，0为索引已存在，-1为尚未选中任何database， -2 database不存在
int IndexManager::createIndex( IndexInfo indexInfo) {
	if (currentDB == "") {
		return -1;
	}
	string dbPath = "DataBase/" + currentDB;
	if (is_dir_exist(dbPath.c_str()) == -1) {
		return -2;
	}

	string filePath = dbPath + "/" + indexInfo.indexName + "_" + indexInfo.tableName + ".index";
	if (is_file_exist(filePath.c_str()) == 0) {
		return 0;
	}
	bool success =ifm->createFile(filePath.c_str());
	if (!success) {
		return -3;
	}
	int fileID;
	ifm->openFile(filePath.c_str(), fileID);
	int index;
	ibm->allocPage(fileID, 0, index, false);
	Byte* page = (Byte*)ibm->addr[index];

	int metaLen = 51;
	Byte meta[51];
	int off  = 0;
	RecordTool::str2Byte(meta + off, 24, indexInfo.tableName.c_str());
	off += 24;
	RecordTool::str2Byte(meta+off, 24, indexInfo.fieldName.c_str());
	off += 24;
	RecordTool::str2Byte(meta+off, 24, indexInfo.indexName.c_str());
	off += 24;
	RecordTool::int2Byte(meta+off, 1, indexInfo.fieldType);
	off += 1;
	Byte tag;
	tag &= 0x00;
	if (indexInfo.ifFixed == 1) {
		tag |= 1;
	}
	if (indexInfo.ifNull == 1) {
		tag |= (1<<1);
	}
	meta[off] = tag;
	off += 1;

	RecordTool::int2Byte(meta+off, 1, indexInfo.indexType);
	off += 1;
	RecordTool::copyByte(page, meta, 51);
	ibm->markDirty(index);
	ibm->writeBack(index);

	//写入根节点页
	off = 0;
	ibm->allocPage(fileID, 1, index, false);
	page = (Byte*)ibm->addr[index];
	RecordTool::int2Byte(page+off, 2,  PAGE_SIZE-96);
	off += 2;
	RecordTool::int2Byte(page+off, 1,  0); //level0即根页
	off += 2;
	RecordTool::int2Byte(page+off, 1,  0); //页类型，非页级索引页
	off += 1;
	RecordTool::int2Byte(page+off, 2,  0); //已有索引行数量为0
	ibm->markDirty(index);
	ibm->writeBack(index);

	ifm->closeFile(fileID);



	return 1;
}

//删除索引
//返回1为删除成功，0为索引不存在，-1为尚未选中任何database， -2 database不存在
int IndexManager::dropIndex(string tableName, string indexName) {
	if (currentDB == "") {
		return -1;
	}
	string dbPath = "DataBase/" + currentDB;
	if (is_dir_exist(dbPath.c_str()) == -1) {
		return -2;
	}

	string filePath = dbPath + "/" + indexName + "_" + tableName + ".index";
	if (is_file_exist(filePath.c_str()) == -1) {
		return 0;
	}
	if (remove(filePath.c_str()) == 0) {
			return 1;
	} else {
			return -3;
	}
	return -3;
}

/*-----------------------------------------------------------------------------------------------------*/
//供查询解析模块调用的功能函数

void  IndexManager::setDataBase(string dbName) {
	currentDB = dbName;
	currentIndexInfo.legal = false;
}

int IndexManager::insertRecord(ConDP key, Data record) {

}

/*-----------------------------------------------------------------------------------------------------*/

//用于索引操作的内部函数
//重构数据文件
// return 0 非簇集索引，无需重构数据文件
// return 1 簇集索引，但数据文件中尚无记录，无需重构
// return 2 成功重构
// return -2 尚未选择database
int IndexManager::reBuildData(IndexInfo indexinfo) {
	if (indexinfo.indexType != 2) {
		return 0;
	}

	if (currentDB == "") {
		return -2;
	}

	string dfilepath = "DataBase/" + currentDB + "/" + indexinfo.tableName + ".data";
	int datapageNum = 0;
	datapageNum = getFilePageNum(dfilepath.c_str());
	if (datapageNum  <= 1) {
		return 1;
	}

	//重构所有数据记录
	string ifilepath = "DataBase/" + currentDB + "/" + indexinfo.indexName + "_" + indexinfo.tableName + ".index";

	//复制meta数据页
	Byte metapage[PAGE_SIZE];
	int fileid = 0;
	dfm->openFile(dfilepath.c_str(), fileid);
	int pageindex = 0;
	Byte* page = (Byte*)dbm->getPage(fileid, 0, pageindex);
	RecordTool::copyByte(metapage, page, PAGE_SIZE);
	dfm->closeFile(fileid);

	//加载table info
	TableInfo tb = loadTableInfo(metapage);

	//创建新数据文件
	string tdfilepath = "DataBase/" + currentDB + "/" + indexinfo.tableName + "_temp.data"; //临时数据文件
	dfm->openFile(tdfilepath.c_str(), fileid);
	dbm->allocPage(fileid, 0, pageindex, false);
	page = (Byte*)dbm->addr[pageindex];
	//写入元数据页
	RecordTool::copyByte(page, metapage, PAGE_SIZE);
	dbm->markDirty(pageindex);
	dbm->writeBack(pageindex);
	dfm->closeFile(fileid);

	//将原数据文件的数据行逐一读取，重新插入新的问文件中
	Byte onePage[PAGE_SIZE];
	for (int i=1; i<datapageNum; i++) {
		//以页为单位读取并插入,因为助教给的fm只能同时打开一个文件，而且页不想取该hash函数 ~~~~(>_<)~~~~
		dfm->openFile(dfilepath.c_str(), fileid);
		Byte* page = (Byte*)dbm->getPage(fileid, i, pageindex);
		RecordTool::copyByte(onePage, page, PAGE_SIZE);
		dfm->closeFile(fileid);

		openIndex(indexinfo.tableName, indexinfo.indexName);
		int slotNum = 0;
		slotNum = RecordTool::byte2Int(onePage+2, 2);
		//遍历槽取出所有记录行
		for (int j=0; j<slotNum; j++) {
			int start = 0;
			Byte* temp = onePage + PAGE_SIZE - 2*(j+1);
			start = RecordTool::byte2Int(temp, 2);
			if (start<=96) {
				continue;
			}
			int lineLen = 0;
			lineLen = RecordTool::byte2Int(onePage+start+1, 2);
			Byte line[lineLen];
			RecordTool::copyByte(line, onePage+start, lineLen);
			//插入新的记录文件
			//获取key
			ConDP key = RecordTool::getFieldValueInRecord(tb, Data(line, lineLen), indexinfo.fieldName);
			insertRecord(key, Data(line, lineLen));
		}
		closeIndex(indexinfo.tableName, indexinfo.indexName);

	}

	//删除数据原始文件，重命名临时数据文件
	if (remove(dfilepath.c_str()) != 0) {
		return -1;
	}

	if (rename(tdfilepath.c_str(), dfilepath.c_str()) != 0) {
		return -1;
	}


	return 2;
}

/**
 *  return 0: 该索引当前已打开 1:该表当前未打开，已重新打开索引
 */
int  IndexManager::openIndex(string tableName, string indexName) {

	if (tableName != currentTable || indexName != currentIndex) {
		if ("" != tableName && "" != indexName && currentFileID != -1) {
			ifm->closeFile(currentFileID);
		}
		string filepath = "";
		filepath = "DataBase/" + currentDB + "/" + indexName + "_" + tableName + ".index";
		ifm->openFile(filepath.c_str(), currentFileID);
		return 1;
	}
	assert(currentFileID >= 0);
	getCurrentIndexInfo();
	return 0;
}

/**
 *  * return 0 该表未打开过   1:该表当前打开,成功关闭
 */
int  IndexManager::closeIndex( string tableName, string indexName) {
	if (tableName != currentTable || indexName != currentIndex || currentFileID == -1) {
		return 0;
	}
	ifm->closeFile(currentFileID);
	currentFileID = -1;
	currentTable = "";
	currentIndex = "";
	return 1;
}

/*----------------------------------------------B+Tree part-----------------------------------*/
//找到包含key的叶节点页，返回页号
//失败时返回-1
int  IndexManager::search(ConDP key) {
	//从根节点出发
	int v = 1; //第0页为meta数据页
	hot = 0;
	int next = 0, type = 0;
	while (v != -1) {
		next = nodeSearch(key, v, type);
		if (type == 1) { //若是找到了叶级页，则之间返回
			return next;
		}
		hot = v;
		v = next;
	}

	return -1;
}

//插入，用于簇集索引，需操作数据页的插入和索引页的插入
bool IndexManager::insert(ConDP key, Data record) {
	int v = search(key);
	if (v == -1) { //当v为-1时，说明根节点为空

	}
}

//插入时根节点为空，填充根节点
void IndexManager::fillRoot(ConDP key) {
	//插入两个索引行，并申请两个新的数据页
	string dfilepath = "DataBase/" + currentDB + "/" + currentTable + ".data";
	int datapageNum = 0;
	datapageNum = getFilePageNum(dfilepath.c_str());
	int fileID;
	dfm->openFile(dfilepath.c_str(), fileID);
	int index;
	dbm->allocPage(fileID, datapageNum, index, false);
	dbm->markDirty(index);
	dbm->writeBack(index);
	datapageNum++;
	dbm->allocPage(fileID, datapageNum, index, false);
	dbm->markDirty(index);
	dbm->writeBack(index);
	dfm->closeFile(fileID);

	//构造两个索引行
	IndexInfo indexinfo = getCurrentIndexInfo();



}

//处理上溢页分裂
void IndexManager::solveOverflow(int v) {

}

//B+Tree相关的工具函数

//在一个索引节点内搜索码值
//查找大于e的最小索引码值
//return 返回下页页号
// type 下页类型，0 中间索引页，1 叶级页
//(具体是叶级索引页还是叶级数据页，通过indexinfo判断)
int  IndexManager::nodeSearch(ConDP conkey, int v, int& type) {
	int pageindex = 0;
	ibm->getPage(currentFileID, v, pageindex);
	Byte* page = (Byte*)ibm->addr[pageindex];
	IndexInfo indexinfo = getCurrentIndexInfo();
	//遍历索引页中的所有索引行
	int lineNum = 0;
	type = -1;
	lineNum = RecordTool::byte2Int(page+5, 2);
	if (lineNum == 0) {
		return -1;
	}
	int count = 0;
	int off = 96;
	Byte* line = NULL;
	while (count < lineNum-1) { //最后一个指针单独处理
		line = page + off;
		int lineLen = RecordTool::byte2Int(line+1, 2);
		//取出下页类型
		Byte tag = line[0];
		type = (tag >> 1) & 1;

		//取出下页页号
		int next = RecordTool::byte2Int(line+3, 4);

		//看key是否为null
		int tempoff = 7;
		if (indexinfo.ifNull == 1) {
			Byte nullByte = line[tempoff];
			int ifNull = nullByte & 1;
			if (ifNull == 1  && conkey.isnull) {
				return next;
			} else if ((ifNull == 0 && conkey.isnull) || (ifNull == 1 && !conkey.isnull)) {
				continue;
			}
			tempoff += 1;
		}
		//取出码值

		int start = 0;
		int end = 0;

		if (indexinfo.ifFixed == 1) { //定长码值
			int keylen = RecordTool::byte2Int(line+tempoff, 2); //定长码长度
			start = tempoff + 2;
			end = start + keylen;
		} else { //变长码值
			tempoff += 2;
			//取出变长列结束位置
			end = RecordTool::byte2Int(line+tempoff, 2);
			tempoff += 2;
			start = tempoff;
		}

		if (indexinfo.fieldType == 0) { //int
			int key = RecordTool::byte2Int(line+start, end-start);
			if (conkey.value_int < key) {
				return next;
			}
		} else if (indexinfo.fieldType == 1) { //string
			char key[end-start];
			RecordTool::byte2Str(key, line+start, end-start);
			if (conkey.value_str < string(key)) {
				return next;
			}
		}

		off += lineLen;
		count++;
	}
	//至此，则为待检索码值大于等于该页中最大码值
	//返回又边界指针
	int next = RecordTool::byte2Int(page+off+3, 4);
	return next;

}


/*-----------------------------------------------------------------------------------------------------*/
//工具函数

IndexInfo IndexManager::getCurrentIndexInfo() {

	//当前已加载且合法，则直接返回
	if (currentIndexInfo.legal == true && currentIndexInfo.tableName == currentTable
			&& currentIndexInfo.indexName == currentIndex) {
		return currentIndexInfo;
	}

	//当前存储的索引不合法则重新读s取
	int pageindex = 0;
	ibm->getPage(currentFileID, 0, pageindex);
	Byte* page = (Byte*)ibm->addr[pageindex];
	IndexInfo indexinfo;
	char tableName[24];
	int off = 0;
	RecordTool::byte2Str(tableName, page + off, 24);
	off += 24;
	char fieldName[24];
	RecordTool::byte2Str(fieldName, page + off, 24);
	off += 24;
	char indexName[24];
	RecordTool::byte2Str(indexName, page + off, 24);
	off += 24;

	indexinfo.tableName = string(tableName);
	indexinfo.fieldName = string(fieldName);
	indexinfo.indexName = string(indexName);

	assert(indexinfo.indexName == currentIndex && indexinfo.tableName == currentTable);

	int fieldType = 0;
	fieldType = RecordTool::byte2Int(page+off, 1);
	off += 1;

	indexinfo.fieldType = fieldType;

	Byte tag = page[off];
	off += 1;
	indexinfo.ifFixed = (tag & 1);
	indexinfo.ifNull = (tag >> 1) & 1;

	int indexType = 0;
	indexType = RecordTool::byte2Int(page+off, 1);
	indexinfo.indexType = indexType;

	return indexinfo;
}


int IndexManager::is_dir_exist(const char* dirpath) {
	 if (dirpath == NULL) {
		 return -1;
	 }

	 if (opendir(dirpath) == NULL) {
		 return -1;
	 }

	 return 0;
}

int IndexManager::is_file_exist(const char* filepath) {
   if (filepath == NULL) {
       return -1;
   }

   if (access(filepath, F_OK) == 0) {
       return 0;
   }

   return -1;
}

//获取文件页数
int  IndexManager::getFilePageNum(const char* filepath) {
	struct stat buf;
	if(stat(filepath, &buf)<0) {
		return 0;
	 }
	return ceil(double(buf.st_size) / PAGE_SIZE);
}

//加载表信息，仅在簇集索引重构数据文件时会用到
TableInfo IndexManager::loadTableInfo(Byte* metapage) {
	TableInfo tb;
	Byte* temp = metapage;
	//get FN
	int fn = 0;
	fn  =  RecordTool::byte2Int(temp,2);
	//cout << "fn: " << fn << endl;
	//get vn
	int vn = 0;
	temp = metapage;
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
		temp = metapage;
		name =  RecordTool::data2Str(Data(temp+off ,24));
		tb.Fname[i] = string(name);
		//cout << "fname: " << tb.Fname[i] << endl;
		off += 24;
	}

	//获取变长列名
	for (int i=0; i<vn; i++) {
			char* name = NULL;
			temp = metapage;
			name =  RecordTool::data2Str(Data(temp+off ,24));
			tb.Vname[i] = string(name);
			//cout << "vname: " << tb.Vname[i] << endl;
			off += 24;
	}

	//获取定长数据长度
	for (int i=0; i<fn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Flen[i] = t;
		//cout << "flen: " << tb.Flen[i] << endl;
		off += 4;
	}

	//获取变长数据长度
	for (int i=0; i<vn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,4);
		tb.Vlen[i] = t;
		//cout << "vlen: " << tb.Vlen[i] << endl;
		off += 4;
	}

	//获取key数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = metapage;
		t = RecordTool::byte2Int(temp+off,1);
		tb.keys[i] = t;
		//cout << "keys: " << tb.keys[i] << endl;
		off += 1;
	}

	//获取types数据
	for (int i=0; i<vn+fn; i++) {
		int t = 0;
		temp = metapage;
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
		tb.nullMap[i] = *(metapage+off);
		//cout << "nullmap: " << int(tb.nullMap[i]) << endl;
		off++;
	}
	return tb;
}


