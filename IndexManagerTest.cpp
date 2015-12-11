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

	/*vector<char> vec;
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
	indexinfo.tableName = "user";*/

	//im->createIndex("test1", indexinfo);

	BufPageManager* bm;
	FileManager* fm;
	fm = new FileManager();
	bm = new BufPageManager(fm);

	fm->createFile("fmtest.txt");

	int fileID;
	fm->openFile("fmtest.txt", fileID);
	int index;
	bm->allocPage(fileID, 2, index, false);
	bm->markDirty(index);
	bm->writeBack(index);

	bm->allocPage(fileID, 3, index, false);
	bm->markDirty(index);
	bm->writeBack(index);

	fm->closeFile(fileID);


	return 0;
}


