/*
 * testfilesystem.cpp
 *
 *  Created on: 2015年10月6日
 *      Author: lql
 */
#include "bufmanager/BufPageManager.h"
#include "Tool/RecordTool.h"
#include <iostream>
#include <iomanip>
#include <cstring>
using namespace std;



int main() {
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

	int a = 135;
	Data d;
	char str[10] = "lalalala!";
	cout << strlen(str) << endl;
	/*d = RecordTool::int2Data(a);
	cout << d.second << endl;
	cout << RecordTool::data2Int(d) << endl;*/

	d = RecordTool::str2Data(str, strlen(str));
	cout << d.second << endl;
	cout << RecordTool::data2Str(d) << endl;

	return 0;
}
