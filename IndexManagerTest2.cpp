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
string analyse(string sql) {

	if (sql.find("INSERT") != string::npos) {
		return "";
	}

	string str = sql;
	int start = 0;
	for (int i=0; i<str.length(); i++) {
		if (str[i] == '\'') {
			start = i;
			break;
		}
	}
	start++;
	int end = 0;

	for (int i=start; i<str.length(); i++) {
		if (str[i] == '\'') {
			end = i;
			break;
		}
	}
	cout << "start: " << start << " end: " << end << endl;
	string sub = str.substr(start,end-start);
	//intvalue = std::atoi(sub.c_str() );
	return sub;

}

int main() {
	DataManager* dm = new DataManager();
	IndexManager* im = new IndexManager(dm);
	dm->setDatabase("orderDB");
	im->setDataBase("orderDB");
	bool flag = false;
	IndexInfo indexinfo = im->getIndexInfo("publisher","name",flag);
	cout << "inde exist flag: " << flag << endl;
	cout << indexinfo.tableName << "#" << indexinfo.indexName << endl;
	im->setIndex(indexinfo.tableName, indexinfo.indexName);
	cout << "_____________" << endl;

	//int deb;
	//cin >> deb;

	int debugType = 1;

	if (debugType == 0) {

		indexinfo.fieldName = "name";
		indexinfo.tableName = "publisher";
		indexinfo.fieldType = 1;
		indexinfo.fieldLen = 0;
		indexinfo.ifFixed = 0;
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
					string value =  "";
					value = analyse(line);

					if (line == "" || line == " " || value== "") {
						continue;
					}


					ConDP key;
					key.isnull = false;
					key.name = "name";
					key.type = 1;
					key.value_str = value + "yxy";
					vector<LP> indexAns = im->searchKey(key);
					cout << "**********" << "line:" << line << "#" << count << "#" << value << "#" << indexAns.size() << "************" << endl;
					//assert(indexAns[0].first == id1+1 && indexAns[0].second == id2+1);
					//cout << "index search ans: " << indexAns.size() << endl;
					/*for (int i=0; i<indexAns.size(); i++) {
						cout << indexAns[i].first << " " << indexAns[i].second << endl;
					}*/
					assert(indexAns.size()>0);

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
			string value =  "";
			value = analyse(line);
			cout << "#" << value << "#" << endl;
			if (line == "" || line == " " || value == "") {
				continue;
			}
			ConDP key;
			key.isnull = false;
			key.name = "name";
			key.type = 1;
			key.value_str = value;
			//vector<LP> indexAns = im->searchKey(key);
			cout << "*******" << count << "  insert:" << "#" << value << "# " << endl;
			//assert(indexAns.size() == 0);
			int flag = 0;
			flag = im->insertRecord(key, LP(id1,id2));
			cout << "insert flag: " <<  flag << endl;
			assert(flag == 1);

			id2++;
			if (id2%100==0) {
				id2 = 0;
				id1++;
			}
			count++;
		}
		fin.clear();
		fin.close();
	} else if (debugType == 3) { //删除
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
					string value =  "";
					value = analyse(line);
					//cout << "#" << value << "#" << endl;
					if (line == "" || line == " " || value == "") {
						continue;
					}
					ConDP key;
					key.isnull = false;
					key.name = "name";
					key.type = 1;
					key.value_str = value;
					//vector<LP> indexAns = im->searchKey(key);
					cout << "*******" << count << "  delete:" << "#" << value << "# " << endl;
					//assert(indexAns.size() == 0);
					int flag = 0;
					flag = im->deleteRecord(key, LP(id1,id2));
					cout << "delete flag: " <<  flag << endl;
					assert(flag == 1);

					id2++;
					if (id2%100==0) {
						id2 = 0;
						id1++;
					}
					count++;
				}
				fin.clear();
				fin.close();
	} else if (debugType == 4) { //更新
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
			string value =  "";
			value = analyse(line);
			//cout << "#" << value << "#" << endl;
			if (line == "" || line == " " || value == "") {
				continue;
			}
			ConDP oldkey;
			oldkey.isnull = false;
			oldkey.name = "name";
			oldkey.type = 1;
			oldkey.value_str = value;


			ConDP newkey = oldkey;
			newkey.value_str += "yxy";
			//vector<LP> indexAns = im->searchKey(key);
			cout << "*******" << count << "  update:" << "#" << value << "# " << endl;
			//assert(indexAns.size() == 0);
			int flag = 0;
			flag =  im->upDateRecord(oldkey, newkey, LP(id1,id2), LP(id1+1,id2+1)) ;
			cout << "update flag: " << flag << endl;
			assert(flag == 1);

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




