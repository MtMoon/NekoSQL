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

int main() {
/********************************************************************************************/
/***************************************User Interface***************************************/
/********************************************************************************************/	

	FileInterface* fi = new FileInterface();
	fi->setDB("orderDB");
	int type;
	cin >> type;
	if (type == 0 || type == 1 || type == 2)
	{
		fi->setType(type);
		string fname;
		cin >> fname;
		fi->processFile(fname);
	}
	getchar();

	ConsoleInterface* ci = new ConsoleInterface();
	ci->Start();
/********************************************************************************************/
/*************************************Common Operation***************************************/
/********************************************************************************************/	
/*
	DataManager* dm = new DataManager();
	SysManager* sm = new SysManager(dm);
	ErrorHandler* errh = new ErrorHandler();
	QueryProcessor* qp = new QueryProcessor(dm, sm, errh);
	SynAnalyze* sa = new SynAnalyze(sm, qp, errh);
	sm->useDatabase("data");
*/

/*	
	//create a new table
	vector<FieldInfo> fieldInfoList;
	FieldInfo* fieldInfo = new FieldInfo();
	fieldInfo->fieldName = "id";
	fieldInfo->fieldType = 0;
	fieldInfo->fieldSize = 4;
	fieldInfo->ifNull = false;
	fieldInfo->key = 2;
	fieldInfoList.push_back(*fieldInfo);
	fieldInfo = new FieldInfo();
	fieldInfo->fieldName = "name";
	fieldInfo->fieldType = 1;
	fieldInfo->fieldSize = 50;
	fieldInfo->ifNull = true;
	fieldInfo->key = 0;
	fieldInfoList.push_back(*fieldInfo);
	fieldInfo = new FieldInfo();
	fieldInfo->fieldName = "age";
	fieldInfo->fieldType = 0;
	fieldInfo->fieldSize = 4;
	fieldInfo->ifNull = true;
	fieldInfo->key = 0;
	fieldInfoList.push_back(*fieldInfo);
	sm->createTable("test2", fieldInfoList);	
*/

/*
	//insert a record
	vector<string> fieldNames;
	vector<string> values;
	vector<bool> isNulls;
	fieldNames.push_back("name");
	fieldNames.push_back("id");
	fieldNames.push_back("age");
	values.push_back("lemon");
	values.push_back("5");
	values.push_back("15");	
	isNulls.push_back(false);
	isNulls.push_back(false);
	isNulls.push_back(false);
	qp->InsertRecord("test2", fieldNames, values, isNulls);
*/	
	//print page info
	//dm->pageInfo("test", 1);
	//dm->pageInfo("test2", 1);


/********************************************************************************************/
/***********************************SynAnalyze Stage*****************************************/
/********************************************************************************************/
/*
	//OneTableAnalyze
	vector<TableAlias> taList;
	if (sa->OneTableAnalyze("", taList, "FROM"))
		cout << taList[0].first << " " << taList[0].second << endl;
*/
	
/*
	//TableFieldAnalyze
	vector<TableAlias> taList;
	vector<string> result;
	//taList.push_back(TableAlias("test", "test"));
	taList.push_back(TableAlias("test2", "apple"));
	string strList[9] = {"", "bitch", ".", "apple.hello", "apple.", "apple . hello", "test2.hello", "test.hello", ".apple.hello."};
	for (int i = 0; i < 9; i++)
	{
		result.clear();
		if (sa->TableFieldAnalyze(strList[i], taList, result, "FROM"))
			cout << result[0] << ":" << result[1] << endl;
	}
*/

	//FromAnalyze(not strictly tested)
	
/*
	//WhereLexAnalyze
	string str = "teacher.name = \'\\\\(\\\')\' AND(teacher.age < 20 OR teacher.age > 30)and teacher.id > 30";
	vector<string> result = sa->WhereLexAnalyze(str);
	cout << "Result Cnt:" << result.size() << endl;
	for (int i = 0; i < result.size(); i++)
		cout << result[i] << endl;
*/
	//CheckWhereSyntax
	//WhereSynAnalyze

/*	
	//WhereWordAnalyze
	vector<TableAlias> taList;
	taList.push_back(TableAlias("test", "test"));
	taList.push_back(TableAlias("test2", "apple"));
	string str[31] = {"", "apple.name", 
			"test.id is null", "test.id is not null", "test.id is is null", "test.id is null 12",
			"12 is null", "\'peach\' is not null",
			"test.id = apple.id", "test.id!= \'peach\'", "test.id = 12", "\'peach\'!=test.id", "12 = \'peach\'",
			"test.id > apple.id", "test.id > 12", "\'peach\' >test.id", "12 > \'peach\'",
			"test.id is = null", "test.id = apple.id!=apple.name", 
			"\'is null\' is not null", "\'!=\' > test.name", "\'\'\' > test.name", "\'\\\'\' > test.name",
			"is null", "apple.id=", "=12", ". = 12", "apple = 12", "12 = apple.", "12 = test2.id", "12 = apple.id.name"};
	vector<WordInfo> wordInfoList;
	for (int i = 0; i < 31; i++)
	{
		cout << str[i] << endl;
		if (sa->WhereWordAnalyze(str[i], taList, wordInfoList))
		{
			cout << "*************************" << endl;			
			for (int j = 0; j < wordInfoList.size(); j++)
			{
				cout << wordInfoList[j].table << "#" << wordInfoList[j].field << "#" 
					<< wordInfoList[j].constStr << "#" << wordInfoList[j].type << endl;
			}
			cout << "*************************" << endl;
		}		
	}
*/
/*
	//WhereAnalyze
	vector<TableAlias> taList;
	taList.push_back(TableAlias("test", "test"));
	//taList.push_back(TableAlias("test2", "apple"));
	Relation rel;
	string str[6] = {"",
			"test.id >= 7",
			"test.id >= 7 or 4 =test.id",
			"id >= 7 or 4=id",
			"test.name is null and test.desc is not null",
			"(test.name = 'apple' or test.name is null) and (test.id >4 and test.id < 6)"};
	string str[6] = {"",
			"test.id = 4",
			"test.id = apple.id",
			"test.name = apple.name",
			"test.id = apple.id and test.id = 4",
			"test.id = apple.id or apple.name is null"};
	for (int i = 0; i < 6; i++)
	{
		if (sa->WhereAnalyze(str[i], taList, rel))
		{
			cout << str[i] << endl;
			qp->PrintRelation(rel);
		}
	}
*/

/*	
	//ShadowAnalyze(no renaming)
	vector<Shadow> shadowList;
	vector<TableAlias> taList;
	taList.push_back(TableAlias("test", "test"));
	//taList.push_back(TableAlias("test2", "test2"));
	string str = "test.apple, peach ,test2.banana";
	if (sa->ShadowAnalyze(str, taList, shadowList))
		qp->PrintShadowList(shadowList);
*/
/*
	//SplitStrWithLimit
	vector<char> leftChar, rightChar;
	leftChar.push_back('(');
	rightChar.push_back(')');
	string str = "(a,'p\\'),',),()p,(l(,)e)";
	vector<string> vec = sa->SplitStrWithLimit(str, ',', leftChar, rightChar);
	cout << "Cnt:" << vec.size() << endl;
	for (int i = 0; i < vec.size(); i++)
		cout << vec[i] << endl;
*/
/*	
	//InsertAnalyze(Without real insertion)
	string str[10] = {"",
			"test (10, null, 'ten')",
			"values (10, null, 'ten')",
			"test5 values (10, null, 'ten')",			
			"test values ",
			"test values ,(10 ,null, 'ten'),",
			"test values ()",
			"test values a()a,",
			"test values (,(10, null, 'ten')",
			"test values (10, null, 'ten')",
			};
	string str2[5] = {
			"test values ()",
			"test values (,10)",
			"test values (apple,'apple')",
			"test values (NULL, 10, '10', 'null', '', ',', ')', '\\')')",
			"test values (10, null, 'ten'),(11, 'null', 'eleven)')"
			};
	for (int i = 0; i < 5; i++)
	{
		cout << i << ":" << str2[i] << endl;
		sa->InsertAnalyze(str2[i]);
	}
*/

/*
	//InsertAnalyze(real insertion)
	//dm->pageInfo("test", 1);
	//cout << "!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	string str = "test values (1,null,null),(2,'null','two'),(3,null,'three')";
	if (sa->InsertAnalyze(str))
		dm->pageInfo("test", 1);
*/

/*
	//DeleteAnalyze
	string  str[5] = {"",
			"test from test",
			"where id = 5",
			" from where id = 5",
			"from test5 where id = 5",
			};
	string str2[4] = {
			"from test",
			"from test where ",
			"from test where id = 5",
			"from test where id = 5 and test.name is null",
			};
	if (sa->DeleteAnalyze(str2[3]))
		dm->pageInfo("test", 1);
*/

/*
	//UpdateAnalyze
	string str[11] = {
			"",
			"test setname='change'",
			" set name = 'change'",
			"test5 set name='change'",
			"test set ",
			"test set name where id = 5",
			"test set name=='change'",
			"test set ='change' where id = 5",
			"test set name = null",
			"test set name = change where id = 5",
			"test set name = where id=5", 
			};
	string str2[5] = {
			"test set name = 'change'",
			"test set name = 'change' where",
			"test set name = 'null' where id = 5",
			"test set name = null where test.name is not null",
			"test set id = 100 where id = 5 and test.name is null"
			};
	if (sa->UpdateAnalyze(str2[4]))
		dm->pageInfo("test", 1);
*/

/*
	//SelectAnalyze(!!viewTable order does not strictly correspond to shadowList order!!)
	string str[5] = {
			"",
			"name fromtest where id = 5",
			" from test where id = 5",
			"id,name from where id = 5",
			"id,name from test2, test5 where test5.id = 5"
			};
	string str2[7] = {
			"id from test",
			"id,name from test where",
			"test.id,test.desc,test.name from test where id = 5",
			"* from test where id = 5 and test.name is not null",
			"* from test, test2 where test.id = test2.id",
			"test.id, test.name, test.desc, test2.age from test, test2 where test.id = test2.id and test.name = test2.name",
			"test.id, test.name, test2.name from test, test2 where test.id=test2.id and test.name is not null and test2.name is not null"
			};
	ViewTable viewTable;
	if (sa->SelectAnalyze(str2[6], viewTable))
		qp->PrintViewTable(viewTable);
*/

/*
	//CmdAnalyze
	bool hasView, hasStr;
	ViewTable viewTable;
	string sysStr;
	string str[10] = {
			";",
			"select * from test where id = 5 and test.name is not null",
			"create table test;",
			"haha select * from test where id = 5 and test.name is not null;",
			"update test set name = 'null' where id = 5;",
			"delete from test where id = 5;",
			"insert test values (1,null,null),(2,'null','two'),(3,null,'three');",
			"insert into test values (1,null,null),(2,'null','two'),(3,null,'three');",
			"select * from test where id = 5 and test.name is not null;",
			"show tables;"
			};
	if (sa->CmdAnalyze(str[8], viewTable, hasView, sysStr, hasStr))
	{
		if (hasView)
			qp->PrintViewTable(viewTable);
		if (hasStr)
			cout << sysStr << endl;
	}
	//dm->pageInfo("test", 1);
*/
/********************************************************************************************/
/*********************************QueryProcessor Stage***************************************/
/********************************************************************************************/
/*
	//SelectRecord
	vector<Shadow> shadowList;
	vector<TableAlias> tableAliasList;
	Relation rel;
	ViewTable viewTable;
	Shadow shadow;
	shadow.first = "test";
	shadow.second.clear();
	shadow.second.push_back("id");
	shadow.second.push_back("desc");
	shadow.second.push_back("name");
	shadowList.push_back(shadow);
	tableAliasList.push_back(TableAlias("test", "test"));
	tableAliasList.push_back(TableAlias("test2", "test2"));
	vector<string> attrInfo;
	attrInfo.push_back("test");
	attrInfo.push_back("test2");
	rel = GenRelation(attrInfo, "1 0 1 0	1 0 1 2		1 1 1 1		1 2 1 0		1 2 1 1		1 2 1 2");
	qp->SelectRecord(shadowList, tableAliasList, rel, viewTable);
	qp->PrintViewTable(viewTable);
*/

/*
	//UpdateRecord
	vector<LP> pos;
	pos.push_back(pair<int, int>(1, 1));
	pos.push_back(pair<int, int>(1, 3));
	qp->UpdateRecord("test", "name", "change", false, pos);
*/
/*
	//DeleteRecord
	//qp->DeleteAllRecord("test");
	//qp->DeleteAllRecord("test1");
	vector<string> attrInfo;
	attrInfo.push_back("test2");
	Relation rel = GenRelation(attrInfo, "1 1");
	qp->DeleteRecord("test2", rel);
*/
/*
	//basic search function test
	ConDP condi;
	condi.value_int = 12;
	condi.value_str = "apple";
	condi.name = "name";
	condi.type = 1;
	condi.isnull = true;
	vector<LP> loc = dm->searchRecord("test", condi, 3);
	cout << "Result Cnt:" << loc.size() << endl;
	for (int i = 0; i < loc.size(); i++)
		cout << loc[i].first << " " << loc[i].second << endl;
	vector<LP> all = dm->getAllLPInTable("test");
	cout << "All Record Cnt:" << all.size() << endl;
	for (int i = 0; i < all.size(); i++)
		cout << all[i].first << " " << all[i].second << endl;
	vector<LP> remain = qp->RemainLP(all, loc);
	cout << "Remain Record Cnt:" << remain.size() << endl;
	for (int i = 0; i < remain.size(); i++)
		cout << remain[i].first << " " << remain[i].second << endl;
*/

/*
//Condition Filter
	Relation relation;
	int flag;
	vector<FieldInfo> fieldInfoList = sm->descTable("test", flag);

	qp->ConditionFilter("test", fieldInfoList, "name", "az", "=", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", "!=", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", ">", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", ">=", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", "<", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", "<=", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", "Null", relation);
	qp->PrintRelation(relation);
	qp->ConditionFilter("test", fieldInfoList, "name", "az", "not Null", relation);
	qp->PrintRelation(relation);
*/


/*
//LinkFilter
	Relation relation;
	int flag;
	vector<FieldInfo> leftInfo = sm->descTable("test", flag); 	
	vector<FieldInfo> rightInfo = sm->descTable("test2", flag); 
	qp->LinkFilter("test", "test2", leftInfo, rightInfo, "id", "id", "<", relation);
	qp->PrintRelation(relation);
*/
	
/*
//AndOp
	Relation leftRel, rightRel;
	vector<string> attrInfo1;
	attrInfo1.push_back("test");
	attrInfo1.push_back("test1");
	attrInfo1.push_back("extra1");
	vector<string> attrInfo2;
	attrInfo2.push_back("test");
	leftRel = GenRelation(attrInfo1, "1 0 1 0 1 2	1 0 1 0 1 5	1 0 1 1 1 2	1 1 1 0 1 2	1 1 1 1 1 1");
	rightRel = GenRelation(attrInfo2, "1 0 1 2 1 4");
	Relation result = qp->AndOp(leftRel, rightRel);
	qp->PrintRelation(result);
*/

/*
//OrOp
	Relation leftRel, rightRel;
	vector<string> attrInfo1;
	attrInfo1.push_back("test");
	attrInfo1.push_back("base");
	vector<string> attrInfo2;
	attrInfo2.push_back("base");
	attrInfo2.push_back("test2");
	leftRel = GenRelation(attrInfo1, "1 8 1 0	1 5 1 1");
	rightRel = GenRelation(attrInfo2, "1 0 1 0	1 2 1 2");

	Relation result = qp->OrOp(leftRel, rightRel);
	qp->PrintRelation(result);
*/




/********************************************************************************************/
/************************************SystemMager Stage***************************************/
/********************************************************************************************/
	/*table info testing
	TableInfo tb = dm->getTableInfo("test");
	int len = 0;
	Data data(NULL, 0);

	DP* dataPairs = new DP[2];
	dataPairs[0].first = "id";
	dataPairs[1].first = "name";
	dataPairs[0].second = Data(NULL, 0);

	dataPairs[1].second = Data(NULL, 0);
	Byte* record = RecordTool::makeRecord(tb, len, dataPairs, 2);
	*/

	/*
	DataManager* dm = new DataManager();
	SysManager* sm = new SysManager(dm);
	sm->useDatabase("data");

	vector<FieldInfo> fieldInfoList;
	FieldInfo* fieldInfo = new FieldInfo();
	fieldInfo->fieldName = "id";
	fieldInfo->fieldType = 0;
	fieldInfo->fieldSize = 4;
	fieldInfo->ifNull = false;
	fieldInfo->key = 2;
	fieldInfoList.push_back(*fieldInfo);
	fieldInfo = new FieldInfo();
	fieldInfo->fieldName = "name";
	fieldInfo->fieldType = 1;
	fieldInfo->fieldSize = 50;
	fieldInfo->ifNull = true;
	fieldInfo->key = 0;
	fieldInfoList.push_back(*fieldInfo);

	sm->createTable("test", fieldInfoList);
	int flag;
	vector<string> tableNames = sm-> showTables(flag);
	for (int i = 0; i < tableNames.size(); i++)
		cout << tableNames[i] << endl;
	fieldInfoList = sm->descTable("test", flag);
	for (int i = 0; i < fieldInfoList.size(); i++)
		PrintFieldInfo(fieldInfoList[i]);
	
	sm->dropTable("test");
	tableNames = sm-> showTables(flag);
	for (int i = 0; i < tableNames.size(); i++)
		cout << tableNames[i] << endl;
	*/


/********************************************************************************************/
/************************************DataManger Stage****************************************/
/********************************************************************************************/
	/*
	DataManager* dm = new DataManager();
	dm->setDatabase("data");
	//dm->createFile("DataBase/data/1.txt");
	int fileID;
	
	//dm->openFile("data/1.txt", fileID);
	//cout << dm->getPageNum("1.txt") << endl;
	

	//dm->insertRecord("1.txt", makeR("apple", 5), 8);
	//dm->insertRecord("1.txt", makeR("banana", 6), 9);
	//dm->insertRecord("1.txt", makeR("apple", 5), 8);
	//dm->insertRecord("1.txt", makeR("banana", 6), 9);
	//dm->deleteRecord("1.txt", pair<int, int>(1, 0));
	//dm->insertRecord("1.txt", pair<Byte*, int>(makeR("apple", 5), 8), pair<int, int>(1,0));
	//cout << dm->getPageNum("1.txt") << endl;
	dm->pageInfo("1.txt", 1);
	//dm->closeFile("data/1.txt");
	*/


/********************************************************************************************/
/************************************FileManger&BufManager Stage*****************************/
/********************************************************************************************/
	/*
	FileManager* fm = new FileManager();
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
	}
	bpm->close();
	fm->closeFile(fileID);
	*/
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
	/*
	DataManager* dm = new DataManager();
	SysManager sm(dm);
	cout << sm.createDatabase("test1") << endl;
	cout << sm.createDatabase("test2") << endl;

	sm.useDatabase("test1");
	cout << dm->getCurrentDBName() << endl;

	sm.useDatabase("test2");
	cout << dm->getCurrentDBName() << endl;

	vector<string> dbs = sm.showDatabases();
	for (int i=0; i<dbs.size(); i++) {
		cout << dbs[i] << endl;
	}

	return 0;
	*/
}
