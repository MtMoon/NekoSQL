/*
 * testfilesystem.cpp
 *
 *  Created on: 2015年10月6日
 *      Author: lql
 */
#include "bufmanager/BufPageManager.h"
#include "datamanager/DataManager.h"
#include "sysmanager/SysManager.h"
#include <iostream>
#include <iomanip>
#include <cstring>
using namespace std;



//int main() {
	/*FileManager* fm = new FileManager();
	BufPageManager* bpm = new BufPageManager(fm);
	fm->createFile("testfile.txt"); //新建文件
	int fileID;
	fm->openFile("testfile.txt", fileID); //打开文件，fileID是返回的文件id
	for (int pageID = 0; pageID < 1000; ++ pageID) {
		int index;
		//为pageID获取一个缓存页
		BufType b = bpm->allocPage(fileID, pageID, index, false);
		b[0] = pageID; //对缓存页进行写操作
		bpm->markDirty(index); //标记脏页
	}
	for (int pageID = 0; pageID < 1000; ++ pageID) {
		int index;
		//为pageID获取一个缓存页
		BufType b = bpm->getPage(fileID, pageID, index);
		cout << b[0] << endl; 		//读取缓存页中第一个整数
		bpm->access(index); //标记访问
	}*/

	//FileManager* fm = new FileManager();
	//FileManager* fm2 = new FileManager();
	//fm->createFile("testfile.txt"); //新建文件
	//int fileID1 = -5, fileID2 = -6;
	//cout << fm->openFile("testfile.txt", fileID1) << endl;
	//cout << fileID1 << endl;

	//cout << fm->closeFile(fileID1) << endl;
	//cout << fm->closeFile(fileID1) << endl;
	//cout << fm->openFile("testfile.txt", fileID2) << endl;
	//cout << fileID2 << endl;

	/*unsigned char A;
	A &= 0x00;
	A |= (1<<3);
	printf("%#x\n",A);  // 按16进制输出*/
	//cout << strlen(str) << endl;
	//d = RecordTool::str2Data(str, strlen(str));
	//cout << d.second << endl;
	//cout << RecordTool::data2Str(d) << endl;

	//DataManager dm;
	//dm.setDatabase("yxy");
	//cout << dm.getPageNum("lalala") << endl;
	//dm.createFile("yxy/text");
	//dm.deleteFile("yxy/text");

	//DataManager* dm = new DataManager();
	//SysManager sm(dm);
	//cout << sm.useDatabase("test1") << endl;
	//cout << dm->getCurrentDBName() << endl;
	/*cout << sm.createDatabase("test1") << endl;
	cout << sm.createDatabase("test2") << endl;

	sm.useDatabase("test1");
	cout << dm->getCurrentDBName() << endl;

	sm.useDatabase("test2");
	cout << dm->getCurrentDBName() << endl;

	vector<string> dbs = sm.showDatabases();
	for (int i=0; i<dbs.size(); i++) {
		cout << dbs[i] << endl;
	}*/

	/*int flag = -1;
	vector<string> tbs = sm.showTables(flag);
	cout << "flag: " << flag << endl;
	for (int i=0; i<tbs.size(); i++) {
		cout << tbs[i] << endl;
	}*/
	//cout << sm.dropTable("table2") << endl;

	//return 0;
//}
