/*
 * testfilesystem.cpp
 *
 *  Created on: 2015年10月6日
 *      Author: lql
 */
#include "bufmanager/BufPageManager.h"
#include "datamanager/DataManager.h"
#include "sysmanager/SysManager.h"
#include "ErrorHandler/ErrorHandler.h"
#include "QueryProcessor/QueryProcessor.h"
#include "SynAnalyze/SynAnalyze.h"
#include "UserInterface/UserInterface.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;

Byte* makeR(const char* str, int len)
{
	Byte* record = new Byte[len+3];
	record[0] = '\0';
	RecordTool::int2Byte(record+1, 2, len+3);
	for (int i = 0; i < len; i++)
		record[i+3] = (Byte)str[i];
	return record;
}


void PrintFieldInfo(const FieldInfo& fieldInfo)
{
	cout << fieldInfo.fieldName << ":" << fieldInfo.fieldType << " "
		<< fieldInfo.fieldSize << " " << fieldInfo.ifNull << " "
		<< fieldInfo.key << endl;
}

Relation GenRelation(const vector<string>& attrList, const string& str)
{
	Relation rel;
	for (int i = 0; i < attrList.size(); i++)
		rel.first.push_back(attrList[i]);
	
	stringstream ss(str);
	int num;
	int attrCnt = attrList.size();
	while (ss >> num)
	{
		Tuple tuple;
		int num1, num2;
		num1 = num;
		ss >> num2;
		tuple.push_back(LP(num1, num2));
		for (int i = 1; i < attrCnt; i++)
		{
			ss >> num1 >> num2;
			tuple.push_back(LP(num1, num2));
		}
		rel.second.push_back(tuple);
	}
	return rel;
}

/*int main() {

	FileInterface* fi = new FileInterface();
	fi->setDB("orderDB");
	int type;
	cin >> type;
	if (type == 0 || type == 1)
	{
		fi->setType(type);
		string fname;
		cin >> fname;
		cout << fname << endl;
		fi->processFile(fname);
	}
	getchar();
	ConsoleInterface* ci = new ConsoleInterface();
	ci->Start();

	return 0;
}*/
