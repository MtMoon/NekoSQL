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

using namespace std;

int main() {

	vector<char> vec;
	vec.push_back('a');
	vec.push_back('b');
	vec.push_back('d');

	vector<char>::iterator it = vec.begin();
	int r = 0;
	vec.erase(it+r);
	for (int i=0; i<vec.size(); i++) {
		cout << vec[i] << endl;
	}

	IndexManager* im = new IndexManager();
	IndexInfo indexinfo;
	indexinfo.fieldName = "name";
	indexinfo.fieldType = 1;
	indexinfo.ifFixed = 0;
	indexinfo.ifNull = 0;
	indexinfo.indexName = "index_name";
	indexinfo.indexType = 2;
	indexinfo.tableName = "user";

	//im->createIndex("test1", indexinfo);


	return 0;
}


