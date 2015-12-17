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
	IndexInfo indexinfo = im->getIndexInfo("user2","age2",flag);
	cout << "inde exist flag: " << flag << endl;
	indexinfo.fieldName = "age2";
	indexinfo.tableName = "user2";
	indexinfo.fieldType = 0;
	indexinfo.fieldLen = 4;
	indexinfo.ifFixed = 1;
	indexinfo.ifNull = 0;
	indexinfo.indexType = 1;
	indexinfo.legal = true;
	indexinfo.indexName = "nind";

	im->setIndex(indexinfo.tableName, indexinfo.indexName);

	int debugtype = 2;

	ConDP key;
	key.isnull = false;
	key.name = "age2";
	key.type = 0;
	key.value_int = 25;

	//im->deleteRecord(key, LP(1,4));

	if (debugtype == 1) {
		int cflag = im->createIndex(indexinfo);
		cout << "create index: " << cflag << endl;
	} else if (debugtype == 0){
		im->insertRecord(key, LP(1,7));
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
			cout << vec[i].second.value_str;
		}
		cout << endl;
	}*/

	//插入数据
	/*DP ageDp;
	ageDp.first = "age2";
	Data agedata = RecordTool::int2Data(25);
	ageDp.second = agedata;

	DP phoneDp;
	phoneDp.first = "phone number2";
	phoneDp.second.first = NULL;
	phoneDp.second.second = 0;

	DP nameDp;
	nameDp.first = "name2";
	Data namedata = RecordTool::str2Data("yxy",3);
	nameDp.second = namedata;

	DP array[3];
	array[2] = nameDp;
	array[0] = ageDp;
	array[1] = phoneDp;*/

	//dm->deleteRecord("user2", LP(1,1));

	/*LP rpos;
	dm->insertRecord("user2", array, 3, rpos);
	cout << "rpos: " << rpos.first << " " << rpos.second << endl;*/


	/*vector<LP> location = dm->getAllLPInTable("user2");
	TableInfo tb = dm->getTableInfo("user2");
	for (int i=0; i<location.size(); i++) {
		cout << location[i].first << " " << location[i].second << endl;
		Data d = dm->getRecordByLP("user2", location[i]);
		RecordTool::printRecord(tb, d);
	}*/



	return 0;
}


