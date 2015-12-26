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

	//sm.createDatabase("orderDB");
	//sm.dropDatabase("orderDB");

	cout << "useDatabase: " << sm.useDatabase("orderDB") << endl;

	vector<FieldInfo> fieldvec;
	FieldInfo f1;
	f1.fieldName = "id";
	f1.fieldType = 0;
	f1.fieldSize = 10;
	f1.ifNull = false;
	f1.key = 2;
	fieldvec.push_back(f1);

	FieldInfo f4;
	f4.fieldName = "publisher_id";
	f4.fieldType = 0;
	f4.fieldSize = 10;
	f4.ifNull = false;
	f4.key = 0;
	fieldvec.push_back(f4);

	FieldInfo f5;
	f5.fieldName = "copies";
	f5.fieldType = 0;
	f5.fieldSize = 10;
	f5.ifNull = true;
	f5.key = 0;
	fieldvec.push_back(f5);

	FieldInfo f6;
	f6.fieldName = "pages";
	f6.fieldType = 0;
	f6.fieldSize = 10;
	f6.ifNull = true;
	f6.key = 0;
	fieldvec.push_back(f6);

	FieldInfo f2;
	f2.fieldName = "title";
	f2.fieldType = 2;
	f2.fieldSize = 100;
	f2.ifNull = false;
	f2.key = 0;
	fieldvec.push_back(f2);

	FieldInfo f3;
	f3.fieldName = "authors";
	f3.fieldType = 2;
	f3.fieldSize = 200;
	f3.ifNull = true;
	f3.key = 0;
	fieldvec.push_back(f3);

	int flag = 0;
	//sm.dropTable("book");
	cout << "create table: " << sm.createTable("book", fieldvec) << endl;
	cout << "准备读取" << endl;
	//读取
	vector<FieldInfo> vec = sm.descTable("book",flag);
	cout << "flag: " << flag << endl;
	for (int i=0; i<vec.size(); i++) {
		cout << vec[i].fieldName << "    " << vec[i].fieldType << "    " << vec[i].fieldSize << "    " << vec[i].ifNull << "    " << vec[i].key << endl;
	}

	return 0;
}*/


