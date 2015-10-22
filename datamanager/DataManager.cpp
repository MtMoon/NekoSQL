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
}

DataManager::~DataManager() {
	delete fm;
	delete bm;
}

// 文件操作函数

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
	filepath = currentBase + "/" + filepath;

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
	int slotNum = 0;
	//获取槽数



	//修改剩余字节数



}

//更新记录
//需要修改null位图和列偏移数组
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
	nvlen += ceil((tb.FN + tb.VN) / 8);
	nvlen += 2*tb.VN;
	int offpos = nvlen - 2*tb.VN;

	int nstart = 7+flen;

	//更新定长字段
	bool flag = false;
	for (int i=0; i<size; i++) {
		if (RecordTool::isVCol(tb, data[i].first)) {
			flag = true;
			continue;
		}
		LP p = RecordTool::getSegOffset(tb, data[i].first);
		//修改定长数据
		Byte* temp = d.first + p.first;
		for (int j=0; j<data[i].second.second; j++) {
			*temp++ = data[i].second.first[j];
		}
		//更新NULL位图
		int nullbyte = nstart + p.second / 8;
		int whichbit = p.second % 8;
		*(d.first + nullbyte) |= (1<<whichbit);

	}




	if (flag) { //如果有变长数据
		//更新变长字段

			Data vdata[tb.VN];
			int lastpos = nvlen;
			for (int i=0; i<tb.VN; i++) {
				int c = 0;
				c = RecordTool::byte2Int(d.first+offpos+2*i, 2);
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
						*(d.first + nullbyte) |= (1<<whichbit);

						vdata[i] = data[i].second;
						break;
					}
				}
			}

			for (int i=0; i<tb.VN; i++) {
				vlen += vdata[i].second;
			}

			Byte* line = new Byte[vlen+nvlen];
			Byte* temp = d.first;
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
				Byte* offstart = line + offpos + tb.FN + i;
				int c = vstart;
				Byte* tempc = (Byte*)&c;
				*offstart++ = tempc[0];
				*offstart = tempc[1];
			}
			return insertRecord(tablename, Data(line, vlen+nvlen), pos);
	} else {
			return insertRecord(tablename, d, pos);
	}


}

//获取属性值满足特定条件的记录
vector<LP> DataManager::searchRecord(const char*tablename, DP condi) {
	int pageNum =  getPageNum(tablename);
	string filepath(tablename);
	filepath = currentBase + "/" + filepath;

	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);

	//遍历除了第一页之外的各个页
	//获取有空余byte的页的index
	int pageindex = 0;
	vector<LP> vec;
	TableInfo tb = getTableInfo(tablename);
	for (int i=1; i<pageNum; i++) {
		bm->getPage(fileID, i, pageindex);
		Byte* page = (Byte*)bm->addr[i];
		//取出这一页的槽数
		int slotNum = 0;
		slotNum = RecordTool::byte2Int(page+2, 2);
		for (int j=0; j<slotNum; j++) {
			if (hasSameSegVal(tb, tablename, LP(i,j), condi)) {
				vec.push_back(LP(i,j));
			}
		}
	}

	return vec;
}

bool DataManager::hasSameSegVal(TableInfo& tb, const char* tablename, LP pos, DP condi) {
	Byte* line = NULL;
	Data d;
	d = getRecordByLP(tablename, pos);
	line = d.first;
	bool ans = false;

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


	} else {
		LP off = RecordTool::getSegOffset(tb, condi.first);
		line += off.first;
		for (int i=0; i<tb.Flen[off.second]; i++) {
			if (condi.second[i] != *line++) {
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
	filepath = currentBase + "/" + filepath;
	int fileID = 0;
	fm->openFile(filepath.c_str(), fileID);
	int pageindex = 0;
	bm->getPage(fileID, pos.first, pageindex);

	Byte* page = (Byte*)bm->addr[pageindex];



	//获取记录起始位置
	int start = 0;
	start = RecordTool::byte2Int(page+PAGE_SIZE-2*(pos.second+1), 2);

	//获取记录长度
	int len = 0;
	len = RecordTool::byte2Int(page+start+1, 2);
	Byte* record = new Byte[len];
	Byte* temp = page+start;
	for (int i=0; i<len; i++) {
		record[i] = *temp++;
	}
	return Data(record,len);
}


int DataManager::getPageLeftSize(int pageindex) {
	BufType page = bm->addr[pageindex];
	//page头的前2个byte用来表示剩余字节数
	return RecordTool::byte2Int((const Byte*)page, 2);
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
	filepath = currentBase + "/" + filepath;

	struct stat buf;
	if(stat(filepath.c_str(), &buf)<0) {
		return 0;
	 }
	return (int)(buf.st_size / 8);
}

//加载表的信息，存于tables并返回
TableInfo DataManager::loadTableInfo(const char* tablename) {

}

