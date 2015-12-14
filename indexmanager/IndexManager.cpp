/*
 * IndexManager.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#include "IndexManager.h"

IndexManager::IndexManager(DataManager* datamanager) {

	ifm = new FileManager();
	ibm = new BufPageManager(ifm);
	dm = datamanager;

	currentIndexInfo.legal = false;

	currentDB = "";
	currentTable = "";
	currentIndex = "";
	currentFileID = -1;
	hot = 0;
	_order = 60;
	lower_bound = ceil(double(_order) / 2) -1;
	upper_bound = _order-1;
}

IndexManager::~IndexManager() {
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

	int metaLen = 77;
	Byte meta[metaLen];
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

	RecordTool::int2Byte(meta+off, 2, indexInfo.fieldLen);
	off += 2;


	RecordTool::copyByte(page, meta, metaLen);
	ibm->markDirty(index);
	ibm->writeBack(index);

	//写入根节点页

	off = 0;
	ibm->allocPage(fileID, 1, index, false);
	page = (Byte*)ibm->addr[index];
	RecordTool::int2Byte(page,2, PAGE_SIZE-96);
	RecordTool::int2Byte(page+2, 1, 1);
	RecordTool::int2Byte(page+4, 1, 0);
	RecordTool::int2Byte(page+5, 2, 0);
	RecordTool::int2Byte(page+7, 4, 0);
	ibm->markDirty(index);
	ibm->writeBack(index);

	ifm->closeFile(fileID);

	reBuildData(indexInfo);



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
//若已有数据文件，则根据已有数据文件生成当前索引
// return 0 但数据文件中尚无记录，无需建索引
// return 1 成功重建索引
// return -1 尚未选择database
// return -2 其他错误
int IndexManager::reBuildData(IndexInfo indexinfo) {
	if (indexinfo.indexType == 2) {
		return -2;
	}

	if (currentDB == "") {
		return -1;
	}

	string dfilepath = "DataBase/" + currentDB + "/" + indexinfo.tableName + ".data";
	int datapageNum = 0;
	datapageNum = getFilePageNum(dfilepath.c_str());
	if (datapageNum  <= 1) {
		return 0;
	}

	//获取数据文件的所有KP
	vector<KP> vec = dm->getAllKPInTable(indexinfo.tableName.c_str(), indexinfo.fieldName);
	openIndex(currentTable, currentIndex);
	int vecSize = vec.size();
	for (int i=0; i<vecSize; i++) {
		insert(vec[i].second, vec[i].first);
	}

	return 1;
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
	int next = 0, type = 0, pageoff = 0;
	while (v != -1) {
		next = nodeSearch(key, v, type, pageoff);
		if (type == 1) { //若是找到了叶级页，则之间返回
			return next;
		}
		hot = v;
		v = next;
	}

	return -1;
}

//插入，用于非簇集索引，用于插入到页级索引页
bool IndexManager::insert(ConDP key, LP pos) {
	int v = search(key);
	if (v == -1) { //当v为-1时，说明根节点为空
		fillRoot(key);
		v = search(key);
	}
	assert(v != -1);
	//查找页级索引页
	int type = 0, pageoff = 96;
	nodeSearch(key, v, type, pageoff);
	int pageindex = 0;
	ibm->getPage(currentFileID, v, pageindex);
	Byte* page = (Byte*)ibm->addr[pageindex];
	//构造叶节点索引行
	IndexInfo indexinfo = getCurrentIndexInfo();
	int lineLen = calcuIndexLineLen(indexinfo, key, 1);
	Byte index[lineLen];
	makeIndexLine(indexinfo, key, 1, 0, pos, lineLen, index);


	int spaceLeft = RecordTool::byte2Int(page,2);
	int lineNum = RecordTool::byte2Int(page+5,2);
	//移动记录行
	if (lineNum != 0) {
		int leftStart = PAGE_SIZE - spaceLeft;
		for (int i=pageoff; i<leftStart; i++) {
			page[i+lineLen] = page[i];
		}
	}

	RecordTool::copyByte(page+pageoff, index, lineLen);
	//修改剩余字节数和索引行数
	RecordTool::int2Byte(page,2, spaceLeft-lineLen);
	RecordTool::int2Byte(page+5, 2, lineNum+1);

	ibm->markDirty(pageindex);
	ibm->writeBack(pageindex);
	solveOverflow(v);
}

//处理上溢页分裂
void IndexManager::solveOverflow(int v) {
	openIndex(currentTable, currentIndex);
	IndexInfo indexinfo = getCurrentIndexInfo();
	int index1 = -1;
	Byte* page1 = (Byte*)(ibm->getPage(currentFileID, v, index1));
	int lineNum = RecordTool::byte2Int(page1+5, 2);
	if (lineNum <= upper_bound) {
		return;
	}
	int parent = RecordTool::byte2Int(page1+7, 4);
	int spaceLeft = RecordTool::byte2Int(page1, 2);
	int pageType = RecordTool::byte2Int(page1+4, 1);
	//如果待分裂的是根页
	if (parent == 0 && v == 1) {
		v  = newIndexPage(1, 1);
		//将根页的内容全部拷贝到v中
		int indexv = -1;
		Byte* pagev = (Byte*)(ibm->getPage(currentFileID, v, indexv));
		int len = PAGE_SIZE - spaceLeft;
		RecordTool::copyByte(pagev+96, page1+96, len-96);
		//修改新的叶节点的数量信息
		RecordTool::int2Byte(pagev, 2, spaceLeft);
		RecordTool::int2Byte(pagev+5, 2, lineNum);
		//修改根节点的信息
		//仅保留第一个指针, 指向pagev
		int len = RecordTool::byte2Int(page1+96+1, 2);
		RecordTool::int2Byte(page1+96+3, 2, v);
		RecordTool::int2Byte(page1, 2, PAGE_SIZE-96-len);
		RecordTool::int2Byte(page1+96+5, 2, 1);
		parent = 1;
		page1 = pagev;
	}

	//分裂
	int u = newIndexPage(pageType+1, parent);
	int index2 = -1;
	Byte* page2 = (Byte*)(ibm->getPage(currentFileID, u, index2));

	int r = _order / 2; //轴点
	int pageOff = 96;

	//查找轴点的起始页偏移
	int start = 96;
	Byte* line = NULL;
	line = page1 + start;
	for (int i=0; i<r; i++) {
		int lineLen = RecordTool::byte2Int(line+1, 2);
		start += lineLen;
	}

	//暂存轴点索引行
	int rLen = RecordTool::byte2Int(page1+start+1, 2);
	Byte rLine[rLen];
	RecordTool::copyByte(rLine, page1+start, rLen);

	//移动剩余节点行到新页
	int end = PAGE_SIZE - spaceLeft;
	int len = end-start;
	RecordTool::copyByte(page2+pageOff, page1+start, len);
	//修改新页剩余空间和索引行数量
	RecordTool::int2Byte(page2, 2, PAGE_SIZE-len);
	RecordTool::int2Byte(page2+5, 2, lineNum-r);
	//修改旧页剩余空间和索引行数量
	RecordTool::int2Byte(page1, 2, spaceLeft+len);
	RecordTool::int2Byte(page1+5, 2, r);

	//修改新页孩子的指针
	pageOff = 96;
	line = NULL;
	for (int i=0; i<lineNum-r; i++) {
		line = page2+pageOff;
		int lineLen = RecordTool::byte2Int(line+1, 2);

		int ptr = RecordTool::byte2Int(line+3, 4);
		int tempIndex = -1;
		Byte* temppage = (Byte*)(ibm->getPage(currentFileID, ptr, tempIndex));
		RecordTool::int2Byte(temppage+7, 4, u);
		ibm->markDirty(tempIndex);
		ibm->writeBack(tempIndex);
		pageOff += lineLen;
	}
	//修改轴点索引行的指针
	RecordTool::int2Byte(rLine+3, 4, u);
	LP temppos;
	ConDP rkey = getKeyByLine(rLine, indexinfo, temppos);
	//轴点关键码上升
	int roff = 96;
	int rtype = 0;
	nodeSearch(rkey, parent, rtype, roff);
	int parentindex = 0;
	Byte* parentpage = (Byte*)(ibm->getPage(currentFileID, parent, parentindex));
	int parentLeft = RecordTool::byte2Int(parentpage, 2);
	int parentLineNum = RecordTool::byte2Int(parentpage+5, 2);
	int rend = PAGE_SIZE - parentLeft;
	for (int i=rend-1; i>=roff; i--) {
		parentpage[i+rLen] = parentpage[i];
	}
	RecordTool::copyByte(parentpage+roff, rLine, rLen);
	//修改父节点的索引行数和剩余空间数
	RecordTool::int2Byte(parentpage, 2, parentLeft-rLen);
	RecordTool::int2Byte(parentpage+5, 2, parentLineNum+1);
	//写入被修改的页
	ibm->markDirty(index1);
	ibm->writeBack(index1);
	ibm->markDirty(index2);
	ibm->writeBack(index2);
	ibm->markDirty(parentindex);
	ibm->writeBack(parentindex);
	solveOverflow(parent);

}

//删除节点码值
void  IndexManager::removeLine(ConDP key, LP pos) {
	int v = search(key);
	if (v == -1) {
		return;
	}
	IndexInfo indexinfo = getCurrentIndexInfo();
	int pageOff = 0;
	int type = 0;
	nodeSearch(key, v, type, pageOff);

	int index = 0;
	Byte* page = (Byte*)(ibm->getPage(currentFileID, v, index)); //页级索引页
	int spaceLeft = RecordTool::byte2Int(page, 2);
	int lineNum = RecordTool::byte2Int(page+5, 2);
	int pageEnd = PAGE_SIZE - spaceLeft;

	int dstart = -1; //删除的起点
	int dend = -1; //删除的终止位置

	bool flag = false;

	while (pageOff < pageEnd) {
		int lineLen = RecordTool::byte2Int(page+pageOff+1, 2);
		Byte line[lineLen];
		RecordTool::copyByte(line, page+pageOff, lineLen);
		LP keypos;
		ConDP linekey = getKeyByLine(line, indexinfo, keypos);
		if (ConDPEqual(key, linekey) && pos.first == keypos.first && pos.second == keypos.second) {
			dstart = pageOff;
			dend = pageOff + lineLen;
			flag = true;
			break;
		}
		pageOff += lineLen;
	}

	if (!flag) {
		return;
	}

	for (int i=dstart; i<dend; i++) {
		page[i] = page[dend+i];
	}
	int len = dend-dstart;
	//修改剩余空间及索引行数量
	RecordTool::int2Byte(page, 2, spaceLeft-len);
	RecordTool::int2Byte(page+5, 2, lineNum-1);

	//写入被修改的页
	ibm->markDirty(index);
	ibm->writeBack(index);

}

//插入时根节点为空，填充根节点
void IndexManager::fillRoot(ConDP key) {

	int pid1 = -1;
	int pid2 = -1;
	int index = 0;

	//非簇集索引

	pid1 = newIndexPage(2, 1);
	pid2 = newIndexPage(2, 1);



	//构造两个索引行
	IndexInfo indexinfo = getCurrentIndexInfo();
	int lineLen = calcuIndexLineLen(indexinfo, key, 0);
	Byte index1[lineLen];
	makeIndexLine(indexinfo, key, 0, pid1, LP(-1,-1), lineLen, index1);

	Byte index2[lineLen];
	makeIndexLine(indexinfo, key, 0, pid2, LP(-1,-1), lineLen, index2);

	//将这两个记录行写入根页
	Byte* page = (Byte*)(ibm->getPage(currentFileID, 1, index));

	RecordTool::copyByte(page+96, index1, lineLen);
	RecordTool::copyByte(page+96+lineLen, index2, lineLen);
	int spaceLeft = RecordTool::byte2Int(page, 2);
	int lineNum = RecordTool::byte2Int(page+5, 2);

	RecordTool::int2Byte(page, 2, spaceLeft-2*lineLen);
	RecordTool::int2Byte(page+5, 2, lineNum+2);

	ibm->markDirty(index);
	ibm->writeBack(index);
}

//B+Tree相关的工具函数

//计算索引行的长度
int IndexManager::calcuIndexLineLen(IndexInfo& indexinfo, ConDP key, int type) {
	int len = 9;
	if (indexinfo.ifNull == 1) {
		len += 1;
	}
	if (indexinfo.ifFixed == 1) {
		len += indexinfo.fieldLen;
	} else if (indexinfo.ifFixed == 0) {
		len += 2;
		if (key.type == 0) {
			len += 4;
		} else if (key.type == 1) {
			len += key.value_str.length();
		}
	}
	if (type == 1) { //如果是非簇集索引的叶级索引行，则再加上4byte的指针
		len += 4;
	}
	return len;
}

//构造索引行，不包括簇集索引叶级页的索引行
//type 0 中间页索引行  1 叶级页索引行
// 当type 为0时， LP 为-1，-1即可
void IndexManager::makeIndexLine(IndexInfo& indexinfo, ConDP key, int type, int pid, LP pos, int lineLen, Byte* line) {
	Byte tag;
	tag &= 0x00;
	if (indexinfo.ifFixed == 1) {
		tag |= 1;
	}

	if (type == 1) {
		tag |= (1<<1);
	}
	int off = 1;
	RecordTool::int2Byte(line+off, 2, lineLen);
	off += 2;
	RecordTool::int2Byte(line+off, 4, pid);
	off += 4;
	if (indexinfo.ifNull == 1) {
		if (key.isnull) {
			RecordTool::int2Byte(line+off, 1, 1);
		} else {
			RecordTool::int2Byte(line+off, 1, 0);
		}
		off += 1;
	}

	if (indexinfo.ifFixed == 1) { //定长码值长度
		RecordTool::int2Byte(line+off, 2, indexinfo.fieldLen);
	} else {
		RecordTool::int2Byte(line+off, 2, 0);
	}

	off += 2;

	//码值
	if (indexinfo.ifFixed == 1) { //定长码值
		if (key.type == 0) {
			RecordTool::int2Byte(line+off, indexinfo.fieldLen, key.value_int);
		} else if (key.type == 1) {
			RecordTool::str2Byte(line+off, indexinfo.fieldLen, key.value_str.c_str());
		}
		off += indexinfo.fieldLen;

	} else { //变长码值
		RecordTool::int2Byte(line+off, 2, off+2+key.value_str.length()); //列偏移
		off += 2;
		RecordTool::str2Byte(line+off, key.value_str.length(), key.value_str.c_str());
		off += key.value_str.length();
	}

	//是页级索引
	if (type == 1) {
		RecordTool::int2Byte(line+off, 2, pos.first); //写入页号和槽号
		off += 2;
		RecordTool::int2Byte(line+off, 2, pos.second);
	}


}

//获取key值
//以及返回该条索引行里存储的数据记录指针(如果是非簇集的页级索引行的话)
ConDP  IndexManager::getKeyByLine(Byte* line, IndexInfo& indexinfo, LP& pos) {
	ConDP conkey;
	conkey.isnull = false;
	conkey.name = indexinfo.fieldName;
	conkey.type = indexinfo.fieldType;
	int lineLen = RecordTool::byte2Int(line+1, 2);

	//简化为建索引字段不能为NULL
	int tempoff = 7;
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
		pos.first = RecordTool::byte2Int(line+end, 2);
		pos.second = RecordTool::byte2Int(line+end+2, 2);
	}

	if (indexinfo.fieldType == 0) { //int
		int key = RecordTool::byte2Int(line+start, end-start);
		conkey.value_int = key;
	} else if (indexinfo.fieldType == 1) { //string
		char key[end-start];
		RecordTool::byte2Str(key, line+start, end-start);
		conkey.value_str = string(key);
	}
	return conkey;
}

//在一个索引节点内搜索码值
//查找大于e的最小索引码值
//return 返回下页页号
// type 下页类型，0 中间索引页，1 叶级页
// pageoff conkey值在节点中的偏移量
int  IndexManager::nodeSearch(ConDP conkey, int v, int& type, int& pageoff) {
	int pageindex = 0;
	ibm->getPage(currentFileID, v, pageindex);
	Byte* page = (Byte*)ibm->addr[pageindex];
	IndexInfo indexinfo = getCurrentIndexInfo();
	int off = 96;
	pageoff = off;
	//遍历索引页中的所有索引行
	int lineNum = 0;
	type = -1;
	lineNum = RecordTool::byte2Int(page+5, 2);
	if (lineNum == 0) {
		return -1;
	}

	//取出第一个指针
	int lastptr = -1;
	lastptr = RecordTool::byte2Int(page+off+3, 4);
	Byte tag = page[off];
	type = (tag >> 1) & 1;
	int lineLen = RecordTool::byte2Int(page+off+1, 2);
	int count = 1;
	off += lineLen;
	Byte* line = NULL;
	pageoff = off;
	while (count < lineNum) { //第一个指针已经单独处理
		line = page + off;
		lineLen = RecordTool::byte2Int(line+1, 2);
		//取出下页类型
		tag = line[0];
		type = (tag >> 1) & 1;

		//取出下页页号
		int next = RecordTool::byte2Int(line+3, 4);

		//简化为建索引字段不能为NULL
		int tempoff = 7;
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
		pageoff = off;
		if (indexinfo.fieldType == 0) { //int
			int key = RecordTool::byte2Int(line+start, end-start);
			if (conkey.value_int < key) {
				return lastptr;
			}
		} else if (indexinfo.fieldType == 1) { //string
			char key[end-start];
			RecordTool::byte2Str(key, line+start, end-start);
			if (conkey.value_str < string(key)) {
				return lastptr;
			}
		}

		lastptr = next;

		off += lineLen;
		pageoff = off;
		count++;
	}
	//至此，则为待检索码值大于等于该页中最大码值
	//返回又边界指针
	return lastptr;

}

//新开一个索引页，返回页号
int  IndexManager::newIndexPage(int type, int parent) {
	string ifilepath = "DataBase/" + currentDB + "/" + currentIndex+ "_" + currentTable + ".index";
	int pageNum = getFilePageNum(ifilepath.c_str());

	int pid = pageNum;
	int index = 0;
	openIndex(currentTable, currentIndex);
	ibm->allocPage(currentFileID, pid, index, false);
	Byte* page = (Byte*)(ibm->getPage(currentFileID, pid, index));

	//填充页头，0索引根页，1 索引中间页，2 索引叶级页(非簇集)
	//parent 父节点页号，如果本身为根页，则paren为0，因为根页id为1，第0页是meta页
	RecordTool::int2Byte(page,2, PAGE_SIZE-96);
	if (type == 0) {
		RecordTool::int2Byte(page+2, 1, 0);
	} else {
		RecordTool::int2Byte(page+2, 1, 1);
	}
	if (type == 2) {
		RecordTool::int2Byte(page+4, 1, 1);
	} else {
		RecordTool::int2Byte(page+4, 1, 0);
	}
	RecordTool::int2Byte(page+5, 2, 0);
	RecordTool::int2Byte(page+7, 4, parent);

	ibm->markDirty(index);
	ibm->writeBack(index);

	return pid;
}

/*-----------------------------------------------------------------------------------------------------*/
//工具函数

//判断两个key是否相等
bool IndexManager::ConDPEqual(ConDP key1, ConDP key2) {
	bool ans = false;
	if (key1.type != key2.type) {
		return ans;
	}

	if (key1.type == 0 && key1.value_int == key2.value_int) {
		ans = true;
	}

	if (key1.type == 1 && key1.value_str == key2.value_str) {
		ans = true;
	}

	return ans;
}

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
	off += 1;

	indexinfo.fieldLen = RecordTool::byte2Int(page+off, 2);

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
