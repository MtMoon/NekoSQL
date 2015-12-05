/*
 * DataManagerTest.cpp
 *
 *  Created on: 2015-11-26
 *      Author: yxy
 */

#include "bufmanager/BufPageManager.h"
#include "datamanager/DataManager.h"
#include "sysmanager/SysManager.h"
#include <iostream>
#include <iomanip>
#include <cstring>
using namespace std;

int main() {

	DataManager* dm = new DataManager();
	dm->setDatabase("test1");

	// test record1

	/*DP ageDp;
	ageDp.first = "age";
	Data agedata = RecordTool::int2Data(23);
	ageDp.second = agedata;

	DP phoneDp;
	phoneDp.first = "phone number";
	Byte* b = new Byte[11];
	for (int i=0; i<11; i++) {
		b[i] = 0;
	}
	phoneDp.second.first = b;
	phoneDp.second.second = 11;

	DP nameDp;
	nameDp.first = "name";
	Data namedata = RecordTool::str2Data("yxy",3);
	nameDp.second = namedata;

	DP array[3];
	array[2] = nameDp;
	array[0] = ageDp;
	array[1] = phoneDp;*/

	//dm->insertRecord("user", array, 3);
	//LP location;
	//location.first = 1;
	//location.second = 0;
	//dm->deleteRecord("user", location);
	/*vector<LP> ans = dm->searchRecord("user", nameDp);
	cout << "ans size: " << ans.size() << endl;
	LP a = ans[0];
	cout << a.first << " " << a.second << endl;
	Data d = dm->getRecordByLP("user", a);
	cout << "record size: " << d.second << endl;
	Data dname;
	Byte* bname = new Byte[3];

	for (int i=0; i<3; i++) {
		bname[i] = d.first[27+i];
	}
	dname.first = bname;
	dname.second = 3;
	cout << RecordTool::data2Str(dname) << endl;*/
	dm->deleteRecord("user", LP(1,0));
	DP updateDp[1];
	DP namedp;
	namedp.first = "name";
	Data newname = RecordTool::str2Data("magic", 5);
	namedp.second = newname;
	updateDp[0] = namedp;

	cout << "更新后的搜索结果" << endl;
	dm->updateRecord("user", LP(1,0), updateDp, 1);
	vector<LP> ans = dm->searchRecord("user", namedp);
	cout << ans.size() << endl;
	LP a = ans[0];
	cout << a.first << " " << a.second << endl;

	Data d = dm->getRecordByLP("user", a);
	cout << "record size: " << d.second << endl;
	Data dname;
	Byte* bname = new Byte[5];
	for (int i=0; i<5; i++) {
		bname[i] = d.first[27+i];
	}
	dname.first = bname;
	dname.second = 5;
	cout << RecordTool::data2Str(dname) << endl;



	return 0;
}


