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

/*int main() {

	//age phonenumber name
	//cout << "lalala" << endl;
	DataManager* dm = new DataManager();
	dm->setDatabase("test1");
	TableInfo tb = dm->getTableInfo("user");
	cout << tb.FN << endl;
	DP data[3];

	Byte ageByte[2];
	RecordTool::int2Byte(ageByte, 2, 23);
	Data ageData;
	ageData.first = ageByte;
	ageData.second = 2;

	Data phoneData;
	phoneData.first = NULL;
	phoneData.second = 0;

	Byte nameByte[3];
	RecordTool::str2Byte(nameByte,3, "yxy");
	Data nameData;
	nameData.first = nameByte;
	nameData.second = 3;

	data[0].first = "age";
	data[0].second = ageData;

	data[1].first = "phone number";
	data[1].second = phoneData;

	data[2].first = "name";
	data[2].second = nameData;
	int len = 0;
	cout << "lalala" << endl;
	Byte* record = RecordTool::makeRecord(tb,len, data, 3 );
	cout << "len: " << len << endl;




	return 0;
}*/


