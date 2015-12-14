/*
 * SysManagerTest.cpp
 *
 *  Created on: 2015-11-14
 *      Author: yxy
 */

#include "bufmanager/BufPageManager.h"
#include "datamanager/DataManager.h"
#include "sysmanager/SysManager.h"
#include <iostream>
#include <iomanip>
#include <cstring>
using namespace std;

/*int main() {
	DataManager* dm = new DataManager();
	SysManager sm(dm);
	cout << "useDatabase: " << sm.useDatabase("test1") << endl;

	vector<FieldInfo> fieldvec;
	FieldInfo f1;
	f1.fieldName = "name2";
	f1.fieldType = 2;
	f1.fieldSize = 10;
	f1.ifNull = false;
	f1.key = 2;
	fieldvec.push_back(f1);

	FieldInfo f2;
	f2.fieldName = "age2";
	f2.fieldType = 0;
	f2.fieldSize = 2;
	f2.ifNull = false;
	f2.key = 0;
	fieldvec.push_back(f2);

	FieldInfo f3;
	f3.fieldName = "phone number2";
	f3.fieldType = 1;
	f3.fieldSize = 11;
	f3.ifNull = true;
	f3.key = 0;
	fieldvec.push_back(f3);

	cout << "create table: " << sm.createTable("user2", fieldvec) << endl;
	cout << "准备读取" << endl;
	//读取
	int flag = -1;
	vector<FieldInfo> vec = sm.descTable("user",flag);
	cout << "flag: " << flag << endl;
	for (int i=0; i<vec.size(); i++) {
		cout << vec[i].fieldName << "    " << vec[i].fieldType << "    " << vec[i].fieldSize << "    " << vec[i].ifNull << "    " << vec[i].key << endl;
	}

	return 0;
}*/


