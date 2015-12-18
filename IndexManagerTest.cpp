/*
 * IndexManagerTest.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yxy
 */

#include <vector>
#include <iostream>
#include <cstdlib>
#include "indexmanager/IndexManager.h"
#include "datamanager/DataManager.h"

using namespace std;

int main() {

	DataManager* dm = new DataManager();
	IndexManager* im = new IndexManager(dm);
	dm->setDatabase("test1");
	im->setDataBase("test1");
	bool flag = false;
	IndexInfo indexinfo = im->getIndexInfo("user","id",flag);
	cout << "inde exist flag: " << flag << endl;
	indexinfo.fieldName = "id";
	indexinfo.tableName = "user";
	indexinfo.fieldType = 1;
	indexinfo.fieldLen = 3;
	indexinfo.ifFixed = 1;
	indexinfo.ifNull = 0;
	indexinfo.indexType = 1;
	indexinfo.legal = true;
	indexinfo.indexName = "nind";

	im->setIndex(indexinfo.tableName, indexinfo.indexName);

	int debugtype = 2;

	ConDP key;
	key.isnull = false;
	key.name = "id";
	key.type = 1;
	key.value_str = "ccc";

	//im->deleteRecord(key, LP(1,7));

	if (debugtype == 1) {
		int cflag = im->createIndex(indexinfo);
		cout << "create index: " << cflag << endl;
	} else if (debugtype == 0){
		im->insertRecord(key, LP(1,5));
	} else if (debugtype == 2) {
		//cout << im->upDateRecord(key, key2, LP(1,3), LP(1,4)) << endl;;

		vector<LP> indexAns = im->searchKey(key);
		cout << "index search ans: " << indexAns.size() << endl;
		for (int i=0; i<indexAns.size(); i++) {
			cout << indexAns[i].first << " " << indexAns[i].second << endl;
		}
	}




	/*vector<KP> vec = dm->getAllKPInTable(indexinfo.tableName.c_str(), indexinfo.fieldName);
	for (int i=0; i<vec.size(); i++) {
		cout << "pos: " << vec[i].first.first << " " << vec[i].first.second << " ";
		if (vec[i].second.type == 0) {
			cout << vec[i].second.value_int;
		} else {
			//cout << "lalala" << endl;
			cout << vec[i].second.value_str;
		}
		cout << endl;
	}*/

	//插入数据
	/*DP ageDp;
	ageDp.first = "age";
	Data agedata = RecordTool::int2Data(25);
	ageDp.second = agedata;

	DP phoneDp;
	phoneDp.first = "id";
	Data phonedata = RecordTool::str2Data("ccc",3);
	phoneDp.second = phonedata;

	DP nameDp;
	nameDp.first = "name";
	Data namedata = RecordTool::str2Data("yxy",3);
	nameDp.second = namedata;

	DP array[3];
	array[2] = nameDp;
	array[0] = ageDp;
	array[1] = phoneDp;

	TableInfo tb = dm->getTableInfo("user");
	int len = 0;
	//Byte* d = RecordTool::makeRecord(tb, len, array, 3);
	//printf("line null byte is: %x", d[12]);
	//cout << "len: " << len << endl;
	//RecordTool::printRecord(tb, Data(d,len));

	//dm->deleteRecord("user", LP(1,0));

	LP rpos;
	dm->insertRecord("user", array, 3, rpos);
	cout << "rpos: " << rpos.first << " " << rpos.second << endl;


	vector<LP> location = dm->getAllLPInTable("user");

	//printf("null map: %x \n", tb.nullMap[0]);
	for (int i=0; i<location.size(); i++) {
		cout << location[i].first << " " << location[i].second << endl;
		Data d = dm->getRecordByLP("user", location[i]);
		//ConDP key = RecordTool::getFieldValueInRecord(tb, d, "id");
		//cout << "key: " << key.value_str << endl;
		RecordTool::printRecord(tb, d);
	}*/



	return 0;
}


