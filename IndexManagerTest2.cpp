/*
 * IndexManagerTest2.cpp
 *
 *  Created on: 2015-12-25
 *      Author: yxy
 */

#include <vector>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "indexmanager/IndexManager.h"
#include "datamanager/DataManager.h"

using namespace std;

//解析一条数据
void analyse(string sql, int& intvalue) {
	string str = sql.substr(2, sql.length()-3);
	if (str[0] <'0' || str[0]>'9') {
		intvalue = -1;
		return;
	}
	//cout << str << endl;
	int end = 0;
	for (int i=0; i<str.length(); i++) {
		if (str[i] == ',') {
			end = i;
			break;
		}
	}
	string sub = str.substr(0,end);
	intvalue = std::atoi(sub.c_str() );
}

int main() {
	DataManager* dm = new DataManager();
	IndexManager* im = new IndexManager(dm);
	dm->setDatabase("orderDB");
	im->setDataBase("orderDB");
	bool flag = false;
	IndexInfo indexinfo = im->getIndexInfo("publisher","id",flag);
	cout << "inde exist flag: " << flag << endl;
	cout << indexinfo.tableName << "#" << indexinfo.indexName << endl;
	im->setIndex(indexinfo.tableName, indexinfo.indexName);
	cout << "_____________" << endl;

	//int deb;
	//cin >> deb;

	int debugType = 1;

	if (debugType == 0) {

		indexinfo.fieldName = "id";
		indexinfo.tableName = "publisher";
		indexinfo.fieldType = 0;
		indexinfo.fieldLen = 4;
		indexinfo.ifFixed = 1;
		indexinfo.ifNull = 0;
		indexinfo.indexType = 1;
		indexinfo.legal = true;
		indexinfo.indexName = "pid";
		int cflag = im->createIndex(indexinfo);
		cout << "create index: " << cflag << endl;
	} else if (debugType == 1) {

		ifstream fin("sqlstatements/publisher.sql");
				if (!fin) {
					cout << "file not fount" << endl;
					exit(0);
				}
				string line = "";
				getline(fin,line);
				int id1 = 1;
				int id2 = 0;
				int count = 0;
				while (getline(fin,line)) {
					int value =  0;
					analyse(line, value);
					if (line == "" || line == " " || value==-1) {
						continue;
					}
					//cout << value << endl;


					ConDP key;
					key.isnull = false;
					key.name = "id";
					key.type = 0;
					key.value_str = "";
					key.value_int = value;
					vector<LP> indexAns = im->searchKey(key);
					cout << "**********" << "line:" << line << "#" << count << "#" << value << "#" << indexAns.size() << "************" << endl;
					assert(indexAns.size()==1);
					assert(indexAns[0].first == id1 && indexAns[0].second == id2);
					//cout << "index search ans: " << indexAns.size() << endl;
					/*for (int i=0; i<indexAns.size(); i++) {
						cout << indexAns[i].first << " " << indexAns[i].second << endl;
					}*/

					id2++;
					if (id2%100==0) {
						id2 = 0;
						id1++;
					}
					count++;
				}
				fin.clear();
				fin.close();

	} else if (debugType == 2) {
		ifstream fin("sqlstatements/publisher.sql");
		if (!fin) {
			cout << "file not fount" << endl;
			exit(0);
		}
		string line = "";
		getline(fin,line);
		int id1 = 1;
		int id2 = 0;
		int count = 0;
		while (getline(fin,line)) {
			int value =  0;
			analyse(line, value);
			if (line == "" || line == " " || value == -1) {
				continue;
			}
			//cout << value << endl;

			ConDP key;
			key.isnull = false;
			key.name = "id";
			key.type = 0;
			key.value_str = "";
			key.value_int = value;
			vector<LP> indexAns = im->searchKey(key);
			assert(indexAns.size() == 0);
			cout << count << " insert:" << value << " " << im->insertRecord(key, LP(id1,id2)) << endl;

			id2++;
			if (id2%100==0) {
				id2 = 0;
				id1++;
			}
			count++;
		}
		fin.clear();
		fin.close();
	}



	return 0;
}




