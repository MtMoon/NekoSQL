#include "QueryProcessor.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

using namespace std;

QueryProcessor::QueryProcessor(DataManager* dm, SysManager* sm, IndexManager* im, ErrorHandler* errh)
{
	assert(dm != NULL);
	assert(sm != NULL);
	assert(errh != NULL);
	dataManager = dm;
	sysManager = sm;
	indexManager = im;
	errHandler = errh;
}

QueryProcessor::~QueryProcessor()
{
}

bool QueryProcessor::IsInt(const string& value)
{
	int len = value.length();
	int index = 0;
	bool state = 0;
	while (index < len)
	{
		if (value[index] == ' ' || value[index] == '\t')
		{
			if (state == 0)
				state = 1;
			else if (state == 2)
				state = 3;
			index++;
		}
		else
		{
			if (state == 3)
				return false;
			else if (state <= 1)
			{
				if (value[index] != '-' && value[index] != '+' && (value[index] > '9' || value[index] < '0'))
					return false;
				state = 2;
			}
			else
			{
				if (value[index] > '9' && value[index] < '0')
					return false;
			}
			index++;
		}
	}
	int num = atoi(value.c_str());
	stringstream ss;
	string numStr;
	ss << num;
	ss >> numStr;
	if (numStr != value)
		return false;
	return true;
}


bool QueryProcessor::SelectRecord(const vector<Shadow>& cShadowList, const vector<TableAlias>& tableAliasList, const Relation& relation, ViewTable& viewTable)
{

	viewTable.first.clear();
	viewTable.second.clear();
	assert(tableAliasList.size() == relation.first.size());
	int tableCnt = relation.first.size();

	//check or generate shadowList
	vector<Shadow> shadowList;
	if (cShadowList.size() == 0)
	{
		for (int i = 0; i < tableCnt; i++)
		{
			int flag;
			vector<FieldInfo> fieldInfoList = sysManager->descTable(tableAliasList[i].first, flag);
			Shadow shadow;
			vector<string> fieldVec;
			for (int j = 0; j < fieldInfoList.size(); j++)
				fieldVec.push_back(fieldInfoList[j].fieldName);
			shadow.first = tableAliasList[i].first;
			shadow.second = fieldVec;
			shadowList.push_back(shadow);
		} 
	}
	else
	{
		for (int i = 0; i < cShadowList.size(); i++)
		{
			int flag;
			vector<FieldInfo> fieldInfoList = sysManager->descTable(cShadowList[i].first, flag);
			for (int j = 0; j < cShadowList[i].second.size(); j++)
			{
				bool found = false;
				for (int k = 0; k < fieldInfoList.size(); k++)
				{
					if (cShadowList[i].second[j] == fieldInfoList[k].fieldName)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					errHandler->ErrorHandle("SELECT", "sematic", "Field "+cShadowList[i].first+"."+cShadowList[i].second[j]+" does not exist.");
					return false;
				}
			}
		}
		shadowList = cShadowList;
	}

	//generate indexList
	int shadowCnt = shadowList.size();
	vector<TableInfo> tbList;
	vector<int> indexList;
	for (int i = 0; i < shadowCnt; i++)
		tbList.push_back(dataManager->getTableInfo((shadowList[i].first).c_str()));

	for (int i = 0; i < tableCnt; i++)
	{
		bool found = false;
		for (int j = 0; j < shadowCnt; j++)
		{
			if (relation.first[i] == shadowList[j].first)
			{
				indexList.push_back(j);
				found = true;
				break;
			}
		} 
		if (!found)
			indexList.push_back(-1);
	}

	//generate viewTitle
	ViewTitle viewTitle;
	if (shadowCnt == 1)
	{
		for (int i = 0; i < shadowList[0].second.size(); i++)
			viewTitle.push_back(shadowList[0].second[i]);
	}
	else
	{
		for (int i = 0; i < relation.first.size(); i++)
		{
			int index = indexList[i];
			if (index == -1)
				continue;
			for (int j = 0; j < shadowList[index].second.size(); j++)
				viewTitle.push_back(shadowList[index].first+"."+shadowList[index].second[j]);
		}
	}

	viewTable.first = viewTitle;
	for (int i = 0; i < relation.second.size(); i++)
	{
		ViewEntry viewEntry;
		for (int j = 0; j < relation.first.size(); j++)
		{
			LP pos = relation.second[i][j];
			int index = indexList[j];
			if (index == -1)
				continue;
			Data record = dataManager->getRecordByLP((relation.first[j]).c_str(), pos);
			for (int k = 0; k < shadowList[index].second.size(); k++)
			{
				ConDP condi = RecordTool::getFieldValueInRecord(tbList[index], record, shadowList[index].second[k]);
				string str="";
				if (!condi.isnull)
				{
					if (condi.type == 0)
					{
						char buffer[50];
						sprintf(buffer, "%d", condi.value_int);
						str = string(buffer);
					}
					else
						str = condi.value_str;
				}
				viewEntry.push_back(str);
			}
		}
		viewTable.second.push_back(viewEntry);
	}

	return true;
}

bool QueryProcessor::DeleteAllRecord(const string& tableName)
{

	vector<LP> allPos = dataManager->getAllLPInTable(tableName.c_str());

	int fieldFlag;
	vector<FieldInfo> fieldInfoList = sysManager->descTable(string(tableName), fieldFlag);
	TableInfo tb = dataManager->getTableInfo(tableName.c_str());
	for (int i = 0; i < fieldInfoList.size(); i++)
	{
		bool indexFlag;
		IndexInfo indexInfo = indexManager->getIndexInfo(tableName, fieldInfoList[i].fieldName, indexFlag);
		if (indexFlag)
		{
			indexManager->setIndex(tableName, indexInfo.indexName);
			for (int j = 0; j < allPos.size(); j++)
			{
				Data data = dataManager->getRecordByLP(tableName.c_str(), allPos[j]);
				ConDP condi = RecordTool::getFieldValueInRecord(tb, data, fieldInfoList[i].fieldName);
				if ((indexManager->deleteRecord(condi, allPos[j])) == 0)
				{
					errHandler->ErrorHandle("DELETE", "index", "index not exist.");
					return false;	
				}
			}
		}	
	}

	for (int i = 0; i < allPos.size(); i++)
	{
		if (!(dataManager->deleteRecord(tableName.c_str(), allPos[i])))
		{
			errHandler->ErrorHandle("DELETE", "operational", "Deletion failed.");
			//cout << tableName << ":<" << allPos[i].first << "," << allPos[i].second << ">" << endl;
			return false;
		}
	}

	return true;
}

bool QueryProcessor::DeleteRecord(const string& tableName, const Relation& relation)
{

	assert(relation.first.size() == 1);
	assert(relation.first[0] == tableName);
	int size = relation.second.size();

	int fieldFlag;
	vector<FieldInfo> fieldInfoList = sysManager->descTable(string(tableName), fieldFlag);
	TableInfo tb = dataManager->getTableInfo(tableName.c_str());
	for (int i = 0; i < fieldInfoList.size(); i++)
	{
		bool indexFlag;
		IndexInfo indexInfo = indexManager->getIndexInfo(tableName, fieldInfoList[i].fieldName, indexFlag);
		if (indexFlag)
		{
			indexManager->setIndex(tableName, indexInfo.indexName);
			for (int j = 0; j < size; j++)
			{
				Tuple tuple = relation.second[j];
				Data data = dataManager->getRecordByLP(tableName.c_str(), tuple[0]);
				ConDP condi = RecordTool::getFieldValueInRecord(tb, data, fieldInfoList[i].fieldName);
				if ((indexManager->deleteRecord(condi, tuple[0])) == 0)
				{
					errHandler->ErrorHandle("DELETE", "index", "index not exist.");
					return false;	
				}
			}
		}	
	}

	for (int i = 0; i < size; i++)
	{
		Tuple tuple = relation.second[i];	
		if (!(dataManager->deleteRecord(tableName.c_str(), tuple[0])))
		{
			errHandler->ErrorHandle("DELETE", "operational", "Deletion failed.");
			//cout << tableName << ":<" << tuple[0].first << "," << tuple[0].second << ">" << endl;
			return false;
		}
	}

	return true;
}

bool QueryProcessor::UpdateRecord(const string& tableName, const string& fieldName, const string& target, bool isNull, int type, const vector<LP>& pos)
{

	int flag;
	vector<FieldInfo> fieldInfoList = sysManager->descTable(string(tableName), flag);
	TableInfo tb = dataManager->getTableInfo(tableName.c_str());
	if (flag == -1)
	{
		errHandler->ErrorHandle("UPDATE", "database", "No Database selected.");
		return false;
	}
	else if (flag == 0)
	{
		errHandler->ErrorHandle("UPDATE", "database", "Table "+tableName+" does not exist.");
		return false;
	}
	assert(fieldInfoList.size() > 0);
	
	bool found = false;
	int fieldCnt = fieldInfoList.size(), index = -1;
	for (int i = 0; i < fieldCnt; i++)
		if (fieldName == fieldInfoList[i].fieldName)
		{
			found = true;
			index = i;
			break;
		}
	if (!found)
	{
		errHandler->ErrorHandle("UPDATE", "sematic", "Field "+fieldName+" does not exist.");
		return false;
	}
	

	vector<LP> updatePos;
	if (pos.size() == 0)
		updatePos = dataManager->getAllLPInTable(tableName.c_str());
	else
		updatePos = pos;

	//cout << "Index:" << index << endl;
	for (int i = 0; i < updatePos.size(); i++)
	{
		DP* dataPairs = new DP[fieldCnt];
		Data record = dataManager->getRecordByLP(tableName.c_str(), updatePos[i]);
		for (int j = 0; j < fieldCnt; j++)
		{
			ConDP condi;
			Data data;
			if (j != index)
			{
				condi = RecordTool::getFieldValueInRecord(tb, record, fieldInfoList[j].fieldName);
				if (condi.isnull)
					data = Data(NULL, 0);
				else
				{
					char buffer[50];
					sprintf(buffer, "%d", condi.value_int);
					string temp(buffer);
					ConDP tempCondi;
					if (!(MakeData(fieldInfoList[j], (condi.type == 0) ? temp : condi.value_str, "UPDATE", data, tempCondi)))
						return false;
				}			
			}
			else
			{
				if (isNull)
				{
					if (!(fieldInfoList[j].ifNull))
					{
						errHandler->ErrorHandle("UPDATE", "semantic", "field "+fieldInfoList[j].fieldName+" can not be null.");
						return false;
					}
					else
						data = Data(NULL, 0);
				}
				else
				{
					if (type == 0 && fieldInfoList[j].fieldType != 0 || type == 1 && fieldInfoList[j].fieldType == 0)
					{
						errHandler->ErrorHandle("UPDATE", "semantic", "field type mismatch:"+fieldInfoList[j].fieldName+":"+target);
						return false;				
					}
					ConDP tempCondi;
			 		if (!MakeData(fieldInfoList[j], target, "UPDATE", data, tempCondi))
						return false;
					/****************************primary key **********************
					if (fieldInfoList[j].key == 2)
					{
						vector<LP> posVec = dataManager->searchRecord(tableName.c_str(), tempCondi, 0);
						if (posVec.size() > 1 || posVec.size() == 1 && posVec[0] != updatePos[i])
						{
							string msg = "field value of "+fieldInfoList[i].fieldName+" already exists:";
							msg += (tempCondi.type == 0) ? target : tempCondi.value_str;
							errHandler->ErrorHandle("INSERT", "semantic", msg);
							return false;
						}
					}
					*/
				}
			}
			dataPairs[j].first = fieldInfoList[j].fieldName;
			dataPairs[j].second = data;
		}
		
/*
		cout << "<" << updatePos[i].first << "," << updatePos[i].second << ">" << endl;
		cout << "Cnt:" << fieldCnt << endl;	

		for (int j = 0; j < fieldCnt; j++)
		{
			cout << dataPairs[j].first << " " << dataPairs[j].second.second << " ";
			if (dataPairs[j].second.first == NULL)
				cout << "NULL";
			else
				cout << "NOT NULL";
			cout << ":";

			for (int k = 0; k < dataPairs[j].second.second; k++)
				cout << dataPairs[j].second.first[k];
			cout << endl;
		}
*/

		
		TableInfo tb = dataManager->getTableInfo(tableName.c_str());
		vector<ConDP> condiVec;
		for (int j = 0; j < fieldInfoList.size(); j++)
		{
			bool indexFlag;
			IndexInfo indexInfo = indexManager->getIndexInfo(tableName, fieldInfoList[j].fieldName, indexFlag);
			if (indexFlag)
			{
				Data data = dataManager->getRecordByLP(tableName.c_str(), updatePos[i]);
				ConDP condi = RecordTool::getFieldValueInRecord(tb, data, fieldInfoList[j].fieldName);
				condiVec.push_back(condi);
			}
		}

		dataManager->deleteRecord(tableName.c_str(), updatePos[i]);
		int len = 0;
		Byte* tempRecord = NULL;		
		tempRecord = RecordTool::makeRecord(tb, len, dataPairs, fieldCnt);
		Data tempData(tempRecord, len);
		dataManager->insertRecord(tableName.c_str(), tempData, updatePos[i]);

		for (int j = 0; j < condiVec.size(); j++)
		{
			bool indexFlag;
			IndexInfo indexInfo = indexManager->getIndexInfo(tableName, condiVec[j].name, indexFlag);
			assert(indexFlag);
			indexManager->setIndex(tableName, indexInfo.indexName);
			Data data = dataManager->getRecordByLP(tableName.c_str(), updatePos[i]);
			ConDP newCondi = RecordTool::getFieldValueInRecord(tb, data, condiVec[j].name);
			if (!(indexManager->upDateRecord(condiVec[j], newCondi, updatePos[i], updatePos[i])))
			{
				errHandler->ErrorHandle("UPDATE", "index", "index not exist.");
				return false;	
			}	
		}
		/*
		if (!(dataManager->updateRecord(tableName.c_str(), updatePos[i], dataPairs, fieldCnt)))
		{
			errHandler->ErrorHandle("UPDATE", "operational", "Update failed.");
			return false;
		}
		*/
		delete []dataPairs;
	}

	return true;
}

bool QueryProcessor::InsertRecord(const string& tableName, const vector<string>& fieldNames, const vector<string>& values, const vector<bool>& isNulls, const vector<int>& types)
{
/*
	int flag;
	vector<FieldInfo> fieldInfoList = sysManager->descTable(string(tableName), flag);
*/

	int flag;
	vector<FieldInfo> realField = sysManager->descTable(string(tableName), flag);
	vector<int> matchInfo = sysManager->getPosInfo(string(tableName));	
	vector<FieldInfo> fieldInfoList;
	assert(matchInfo.size() == realField.size());
	for (int i = 0; i < matchInfo.size(); i++)
		fieldInfoList.push_back(realField[matchInfo[i]]);

/*
	cout << "RealField:" << endl;
	for (int i = 0; i < realField.size(); i++)
		cout << realField[i].fieldName << "#";
	cout << endl << "MatchInfo:" << endl;
	for (int i = 0; i < matchInfo.size(); i++)
		cout << matchInfo[i] << "#";
	cout << endl << "FieldInfoList:" << endl;
	for (int i = 0; i < fieldInfoList.size(); i++)
		cout << fieldInfoList[i].fieldName << "#";
	cout << endl;
*/
	if (flag == -1)
	{
		errHandler->ErrorHandle("INSERT", "database", "No Database selected.");
		return false;
	}
	else if (flag == 0)
	{
		errHandler->ErrorHandle("INSERT", "database", "Table "+tableName+" does not exist.");
		return false;
	}
	assert(fieldInfoList.size() > 0);
	bool allField;
	if (fieldNames.size() == 0)
		allField = true;
	else
		allField = false;
	
	int fieldCnt = fieldInfoList.size();
	if (allField && values.size() != fieldCnt || !allField && values.size() != fieldNames.size())
	{
		errHandler->ErrorHandle("INSERT", "semantic", "number of values is not equal to number of fields.");
		return false;
	}
	if (!allField)
	{
		for (int i = 0; i < fieldNames.size(); i++)
		{
			bool found = false;
			for (int j = 0; j < fieldInfoList.size(); j++)
			{
				if (fieldInfoList[j].fieldName == fieldNames[i])
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				errHandler->ErrorHandle("INSERT", "semantic", "field "+fieldNames[i]+" does not exist.");
				return false;
			}
		}
	}
	DP* dataPairs = new DP[fieldCnt];
	DP* newDataPairs = new DP[fieldCnt];
	for (int i = 0; i < fieldCnt; i++)
	{
		int index = -1;
		if (allField)
			index = i;
		else
		{
			for (int j = 0; j < fieldNames.size(); j++)
				if (fieldNames[j] == fieldInfoList[i].fieldName)
				{
					index = j;
					break;
				}
		}
		
		//cout << i << ":" << fieldInfoList[i].fieldName << " " << index << endl;
		Data data;
		if (index == -1 || isNulls[index])
		{
			if (!(fieldInfoList[i].ifNull))
			{
				errHandler->ErrorHandle("INSERT", "semantic", "field "+fieldInfoList[i].fieldName+" can not be null.");
				return false;
			}
			else
				data = Data(NULL, 0);
		}
		else
		{
			if (types[index] == 0 && fieldInfoList[i].fieldType != 0 || types[index] == 1 && fieldInfoList[i].fieldType == 0)
			{
				errHandler->ErrorHandle("INSERT", "semantic", "field type mismatch:"+fieldInfoList[i].fieldName+":"+values[index]);
				return false;	
			}
		
			ConDP condi;
			if (!(MakeData(fieldInfoList[i], values[index], "INSERT", data, condi)))
				return false;
			/****************************primary key **********************
			if (fieldInfoList[i].key == 2)
			{
				vector<LP> posVec = dataManager->searchRecord(tableName.c_str(), condi, 0);
				if (posVec.size() != 0)
				{
					string msg = "field value of "+fieldInfoList[i].fieldName+" already exists:";
					msg += (condi.type == 0) ? values[index] : condi.value_str;
					errHandler->ErrorHandle("INSERT", "semantic", msg);
					return false;
				}
			}
			*/
		}
		dataPairs[i].first = fieldInfoList[i].fieldName;
		//cout << i << ":" << dataPairs[i].first << "#" << fieldInfoList[i].fieldName << endl;
		dataPairs[i].second = data;
	}
	
	for (int i = 0; i < realField.size(); i++)
	{
		for (int j = 0; j < fieldCnt; j++)
			if (dataPairs[j].first == realField[i].fieldName)
				newDataPairs[i] = dataPairs[j];
	}
/*
	cout << "Cnt:" << fieldCnt << endl;	
	for (int i = 0; i < fieldCnt; i++)
	{
		cout << newDataPairs[i].first << " " << newDataPairs[i].second.second << " ";
		if (newDataPairs[i].second.first == NULL)
			cout << "NULL";
		else
			cout << "NOT NULL";
		cout << ":";
		for (int j = 0; j < newDataPairs[i].second.second; j++)
			cout << newDataPairs[i].second.first[j];
		cout << endl;
	}
*/

	LP tempLP;
	if (!(dataManager->insertRecord(tableName.c_str(), newDataPairs, fieldCnt, tempLP)))
	{
		errHandler->ErrorHandle("INSERT", "operational", "Insertion failed.");
		return false;
	}

	TableInfo tb = dataManager->getTableInfo(tableName.c_str());
	for (int i = 0; i < fieldInfoList.size(); i++)
	{
		bool indexFlag;
		IndexInfo indexInfo = indexManager->getIndexInfo(tableName, fieldInfoList[i].fieldName, indexFlag);
		if (indexFlag)
		{
			indexManager->setIndex(tableName, indexInfo.indexName);
			Data data = dataManager->getRecordByLP(tableName.c_str(), tempLP);
			ConDP condi = RecordTool::getFieldValueInRecord(tb, data, fieldInfoList[i].fieldName);
			indexManager->insertRecord(condi, tempLP);
		}	
	}
	return true;
}

bool QueryProcessor::MakeData(const FieldInfo& fieldInfo, const string& str, const string& opType, Data& data, ConDP& condi)
{
	condi.value_int = 0;
	condi.value_str = "";
	condi.name = fieldInfo.fieldName;
	condi.type = (fieldInfo.fieldType == 0) ? 0 : 1;
	condi.isnull = false;
	if (fieldInfo.fieldType == 0)
	{
		if (!IsInt(str))
		{
			errHandler->ErrorHandle(opType, "semantic", "field "+fieldInfo.fieldName+":invalid int foramt.");
			return false;
		}
		int temp = atoi(str.c_str());
		condi.value_int = temp;
		data = RecordTool::int2Data(temp);
	}
	else
	{
		if (str.length() > fieldInfo.fieldSize)
			errHandler->WarningHandle(opType, "semantic", "field "+fieldInfo.fieldName+": string's too long.");
		int size;
		if (fieldInfo.fieldType == 1)
			size = fieldInfo.fieldSize;
		else
			size = (str.length() < fieldInfo.fieldSize) ? str.length() : fieldInfo.fieldSize;
		int subSize = (str.length() > size) ? size : str.length();
		condi.value_str = str.substr(0, subSize);
		data = RecordTool::str2Data(str.c_str(), size);
	}
	return true;
}

vector<LP> QueryProcessor::RemainLP(const vector<LP>& father, const vector<LP>& son)
{
	vector<LP> remain;
	for (int i = 0; i < father.size(); i++)
	{
		bool found = false;
		for (int j = 0; j < son.size(); j++)
		{
			if (father[i] == son[j])
			{
				found = true;
				break;
			}
		}
		if (!found)
			remain.push_back(father[i]);
	}
	return remain;
}

bool QueryProcessor::ConditionFilter(const string& tableName, const vector<FieldInfo>& fieldInfoList, const string& fieldName, const string& target, int type, const string& op, Relation& relation)
{
	relation.first.clear();
	relation.second.clear();
	
	int fieldCnt = fieldInfoList.size();
	int index = -1;
	for (int i = 0; i < fieldCnt; i++)
		if (fieldName == fieldInfoList[i].fieldName)
		{
			index = i;
			break;
		}
	if (index == -1)
	{
		errHandler->ErrorHandle("WHERE", "sematic", tableName+"."+fieldName+" does not exist.");
		return false;
	}

	Data data;
	ConDP tempCondi;
	if (op == "Null" || op == "not Null")
		data = Data(NULL, 0);
	else
	{
		if (type == 0 && fieldInfoList[index].fieldType != 0 || type == 1 && fieldInfoList[index].fieldType == 0)
		{
			errHandler->ErrorHandle("WHERE", "sematic", "field type mismatch:"+fieldInfoList[index].fieldName+":"+target);
			return false;			
		}
		if (!MakeData(fieldInfoList[index], target, "WHERE", data, tempCondi))
			return false;
	}

	ConDP condi;
	condi.value_int = 0;
	condi.value_str = "";
	condi.name = fieldName;
	condi.type = (fieldInfoList[index].fieldType == 0) ? 0 : 1;
	condi.isnull =	(op == "Null" || op == "not Null") ? true : false;

	ConDP nullCondi = condi;
	nullCondi.isnull = true;

	if (!(op == "Null" || op == "not Null"))
	{
		if (fieldInfoList[index].fieldType == 0)
			condi.value_int = RecordTool::data2Int(data);
		else
			condi.value_str = tempCondi.value_str;	
	}

	vector<LP> records;
	if (op == "Null")
	{
		records = dataManager->searchRecord(tableName.c_str(), condi, 3);
	}
	else if (op == "not Null")
	{
		/*
		vector<LP> allRecords = dataManager->getAllLPInTable(tableName.c_str());
		vector<LP> tempRecords = dataManager->searchRecord(tableName.c_str(), condi, 3);
		records = RemainLP(allRecords, tempRecords);
		*/
		records = dataManager->searchRecord(tableName.c_str(), condi, 5);
	}
	else if (op == "=")
	{
		bool indexFlag;
		IndexInfo indexInfo = indexManager->getIndexInfo(tableName, fieldName, indexFlag);
		if (indexFlag)
		{
			indexManager->setIndex(tableName, indexInfo.indexName);
			records = indexManager->searchKey(condi);
		}
		else
		{
			records = dataManager->searchRecord(tableName.c_str(), condi, 0);
		}
	}
	else if (op == "!=")
	{
		/*
		vector<LP> allRecords = dataManager->getAllLPInTable(tableName.c_str());
		vector<LP> tempRecords = dataManager->searchRecord(tableName.c_str(), condi, 0);
		vector<LP> nullRecords = dataManager->searchRecord(tableName.c_str(), nullCondi, 3);
		records = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		*/
		records = dataManager->searchRecord(tableName.c_str(), condi, 4);
	}
	else if (op == ">")
	{
		records = dataManager->searchRecord(tableName.c_str(), condi, 1);
	}
	else if (op == "<")
	{
		records = dataManager->searchRecord(tableName.c_str(), condi, 2);
	}
	else if (op == ">=")
	{
		/*
		vector<LP> allRecords = dataManager->getAllLPInTable(tableName.c_str());
		vector<LP> tempRecords = dataManager->searchRecord(tableName.c_str(), condi, 2);
		vector<LP> nullRecords = dataManager->searchRecord(tableName.c_str(), nullCondi, 3);
		records = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		*/
		vector<LP> largerRecords = dataManager->searchRecord(tableName.c_str(), condi, 1);
		vector<LP> equalRecords = dataManager->searchRecord(tableName.c_str(), condi, 0);
		for (int i = 0; i < largerRecords.size(); i++)
			records.push_back(largerRecords[i]);
		for (int i = 0; i < equalRecords.size(); i++)
			records.push_back(equalRecords[i]);
	}
	else if (op == "<=")
	{
		/*
		vector<LP> allRecords = dataManager->getAllLPInTable(tableName.c_str());
		vector<LP> tempRecords = dataManager->searchRecord(tableName.c_str(), condi, 1);
		vector<LP> nullRecords = dataManager->searchRecord(tableName.c_str(), nullCondi, 3);
		records = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		*/
		vector<LP> smallerRecords = dataManager->searchRecord(tableName.c_str(), condi, 2);
		vector<LP> equalRecords = dataManager->searchRecord(tableName.c_str(), condi, 0);
		for (int i = 0; i < smallerRecords.size(); i++)
			records.push_back(smallerRecords[i]);
		for (int i = 0; i < equalRecords.size(); i++)
			records.push_back(equalRecords[i]);
	}	
	else
	{
		errHandler->ErrorHandle("SYSTEM", "system", "ConditionFilter: invalid operator:"+op);
		return false;
	}

	relation.first.push_back(tableName);
	int size = records.size();
	for (int i = 0; i < size; i++)
	{
		Tuple tuple;
		tuple.push_back(records[i]);
		relation.second.push_back(tuple);
	}
	return true;
}

bool QueryProcessor::LinkFilter(const string& leftTable, const string& rightTable, const vector<FieldInfo>& leftInfo, const vector<FieldInfo>& rightInfo, const string& leftField, const string& rightField, const string& op, Relation& relation)
{

	relation.first.clear();
	relation.second.clear();

	int leftIndex = -1, rightIndex = -1;
	for (int i = 0; i < leftInfo.size(); i++)
		if (leftInfo[i].fieldName == leftField)
		{
			leftIndex = i;
			break;
		}
	if (leftIndex == -1)
	{
		errHandler->ErrorHandle("WHERE", "sematic", leftTable+"."+leftField+" does not exist.");
		return false;
	}
	for (int i = 0; i < rightInfo.size(); i++)
		if (rightInfo[i].fieldName == rightField)
		{
			rightIndex = i;
			break;
		}
	if (rightIndex == -1)
	{
		errHandler->ErrorHandle("WHERE", "sematic", rightTable+"."+rightField+" does not exist.");
		return false;
	} 
	if (leftInfo[leftIndex].fieldType == 0 && rightInfo[rightIndex].fieldType != 0
			|| leftInfo[leftIndex].fieldType != 0 && rightInfo[rightIndex].fieldType == 0)
	{
		errHandler->ErrorHandle("WHERE", "sematic", leftTable+"."+leftField+" and "+rightTable+"."+rightField+" are not comparable");
		return false;
	}

	vector<LP> leftRecords = dataManager->getAllLPInTable(leftTable.c_str());
	int leftSize = leftRecords.size();
	TableInfo leftTb = dataManager->getTableInfo(leftTable.c_str());
	for (int i = 0; i < leftSize; i++)
	{
		Data leftData = dataManager->getRecordByLP(leftTable.c_str(), leftRecords[i]);		
		ConDP condi = RecordTool::getFieldValueInRecord(leftTb, leftData, leftField);
		condi.name = rightField;
		if (condi.isnull)
			continue;
		ConDP nullCondi = condi;
		nullCondi.isnull = true;
		vector<LP> rightRecords;
		if (op == "=")
		{
			rightRecords = dataManager->searchRecord(rightTable.c_str(), condi, 0);
		}
		else if (op == "!=")
		{
			vector<LP> allRecords = dataManager->getAllLPInTable(rightTable.c_str());
			vector<LP> nullRecords = dataManager->searchRecord(rightTable.c_str(), nullCondi, 3);
			vector<LP> tempRecords = dataManager->searchRecord(rightTable.c_str(), condi, 0);
			rightRecords = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		}
		else if (op == ">")
		{

			rightRecords = dataManager->searchRecord(rightTable.c_str(), condi, 2);
		}
		else if (op == ">=")
		{
			vector<LP> allRecords = dataManager->getAllLPInTable(rightTable.c_str());
			vector<LP> nullRecords = dataManager->searchRecord(rightTable.c_str(), nullCondi, 3);
			vector<LP> tempRecords = dataManager->searchRecord(rightTable.c_str(), condi, 1);
			rightRecords = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		}
		else if (op == "<")
		{

			rightRecords = dataManager->searchRecord(rightTable.c_str(), condi, 1);
		}
		else if (op == "<=")
		{
			vector<LP> allRecords = dataManager->getAllLPInTable(rightTable.c_str());
			vector<LP> nullRecords = dataManager->searchRecord(rightTable.c_str(), nullCondi, 3);
			vector<LP> tempRecords = dataManager->searchRecord(rightTable.c_str(), condi, 2);
			rightRecords = RemainLP(RemainLP(allRecords, nullRecords), tempRecords);
		}
		else
		{
			errHandler->ErrorHandle("SYSTEM", "system", "LinkFilter: invalid operator:"+op);
			return false;
		}

		Tuple tuple;
		for (int j = 0; j < rightRecords.size(); j++)
		{
			tuple.clear();
			tuple.push_back(leftRecords[i]);
			tuple.push_back(rightRecords[j]);
			relation.second.push_back(tuple);
		}
	}
	relation.first.push_back(leftTable);
	relation.first.push_back(rightTable);

	return true;
}

bool QueryProcessor::IncCounter(vector<int>& counter, const vector<int>& limit)
{

	int size = counter.size();
	int index = size-1;
	while (index >= 0)
	{
		if (counter[index] < limit[index]-1)
		{
			counter[index]++;
			break;
		}
		else
		{
			counter[index] = 0;
			if (index == 0)
				return false;
			index--;
		}
	}
	return true;
}

Relation QueryProcessor::Linkage(const Relation& leftRel, const Relation& rightRel)
{
	Relation relation;

	for (int i = 0; i < leftRel.first.size(); i++)
		relation.first.push_back(leftRel.first[i]);
	for (int i = 0; i < rightRel.first.size(); i++)
		relation.first.push_back(rightRel.first[i]);

	int leftSize = leftRel.second.size();
	int rightSize = rightRel.second.size();
	for (int i = 0; i < leftSize; i++)
	{
		for (int j = 0; j < rightSize; j++)
		{
			Tuple tuple = leftRel.second[i];
			for (int k = 0; k < rightRel.second[j].size(); k++)
				tuple.push_back(rightRel.second[j][k]);
			relation.second.push_back(tuple);
		}
	}

	return relation;
}

Relation QueryProcessor::MultiLink(vector<Relation*> relations)
{
	Relation result;

	int relCnt = relations.size();
	for (int i = 0; i < relCnt; i++)
	{
		assert((relations[i]->first).size() > 0);
		int attrCnt = (relations[i]->first).size();
		for (int j = 0; j < attrCnt; j++)
			result.first.push_back(relations[i]->first[j]);
	}

	for (int i = 0; i < relCnt; i++)
	{
		if ((relations[i]->second).size() == 0)
			return result;
	}
	/*
	cout << "Result attr:" << endl;
	for (int i = 0; i < result.first.size(); i++)
		cout << result.first[i] << endl;
	*/
	vector<int> limit, counter;
	for (int i = 0; i < relCnt; i++)
	{
		limit.push_back((relations[i]->second).size());
		counter.push_back(0);
	}
	/*
	cout << "Limit:" << endl;
	for (int i = 0; i < limit.size(); i++)
		cout << limit[i] << endl;
	*/
	
	while (true)
	{
		/*
		cout << "Counter:";
		for (int i = 0; i < counter.size(); i++)
			cout << counter[i] << " ";
		cout << endl;
		*/
		Tuple tuple;
		for (int i = 0; i < relCnt; i++)
		{
			Tuple record = (relations[i]->second)[counter[i]];
			for (int j = 0; j < record.size(); j++)
				tuple.push_back(record[j]);
		}
		result.second.push_back(tuple);
		if (!IncCounter(counter, limit))
			break;
	}
	//PrintRelation(result);
	return result;
}

Relation QueryProcessor::ExtendRelation(const Relation& relation, const vector<string>& lackAttr)
{
	Relation newRel;

	if (lackAttr.size() == 0)
		newRel = relation;
	else
	{
		vector<Relation*> tempRelVec;
		Relation* tempRel;
		if (relation.first.size() != 0)
		{
			tempRel = new Relation();
			*tempRel = relation;
			tempRelVec.push_back(tempRel);
		}
		for (int i = 0; i < lackAttr.size(); i++)
		{
			vector<LP> allLP = dataManager->getAllLPInTable(lackAttr[i].c_str()); 
			tempRel = new Relation();
			(tempRel->first).push_back(lackAttr[i]);
			for (int j = 0; j < allLP.size(); j++)
			{
				Tuple tuple;
				tuple.push_back(allLP[j]);
				(tempRel->second).push_back(tuple);
			}
			tempRelVec.push_back(tempRel);
		}
	/*	
		cout << "tempRelVec:" << endl;
		for (int i = 0; i < tempRelVec.size(); i++)
			PrintRelation(*(tempRelVec[i]));
	*/	
		newRel = MultiLink(tempRelVec);
		for (int i = 1; i < tempRelVec.size(); i++)
			delete tempRelVec[i];
	
	}
	//PrintRelation(newRel);
	return newRel;
}

Relation QueryProcessor::AndOp(const Relation& leftRel, const Relation& rightRel)
{
	Relation relation;

	AttrInfo leftAttr = leftRel.first, rightAttr = rightRel.first;
	vector<int> leftSameIndex, rightSameIndex;
	vector<int> leftDifIndex, rightDifIndex;
	int sameCnt = 0;
	int leftSize = leftRel.second.size(), rightSize = rightRel.second.size();
	
	for (int i = 0; i < leftAttr.size(); i++)
		for (int j = 0; j < rightAttr.size(); j++)
			if (leftAttr[i] == rightAttr[j])
			{
				leftSameIndex.push_back(i);
				rightSameIndex.push_back(j);
				sameCnt++;
			}
	bool found;
	for (int i = 0; i < leftAttr.size(); i++)
	{
		found = false;
		for (int j = 0; j < leftSameIndex.size(); j++)
		{
			if (leftSameIndex[j] == i)
			{
				found = true;
				break;
			}
		}
		if (!found)
			leftDifIndex.push_back(i);
	}
	for (int i = 0; i < rightAttr.size(); i++)
	{
		found = false;
		for (int j = 0; j < rightSameIndex.size(); j++)
		{
			if (rightSameIndex[j] == i)
			{
				found = true;
				break;
			}
		}
		if (!found)
			rightDifIndex.push_back(i);
	}

	if (sameCnt == 0)
		return Linkage(leftRel, rightRel);
	
	relation.first = leftAttr;
	for (int i = 0; i < rightDifIndex.size(); i++)
		relation.first.push_back(rightAttr[rightDifIndex[i]]);

	bool match;
	for (int i = 0; i < leftSize; i++)
	{
		Tuple leftTuple = leftRel.second[i];
		for (int j = 0; j < rightSize; j++)
		{
			Tuple rightTuple = rightRel.second[j];
			match = true;
			for (int k = 0; k < sameCnt; k++)
			{
				if (leftTuple[leftSameIndex[k]] != rightTuple[rightSameIndex[k]])
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				Tuple tuple = leftTuple;
				for (int k = 0; k < rightDifIndex.size(); k++)
					tuple.push_back(rightTuple[rightDifIndex[k]]);
				relation.second.push_back(tuple);
			}
		}
	}

	return relation;
}

Relation QueryProcessor::OrOp(const Relation& leftRel, const Relation& rightRel)
{
	Relation relation;

	AttrInfo leftAttr = leftRel.first, rightAttr = rightRel.first;
	vector<int> rightIndex;
	vector<string> leftLackAttr, rightLackAttr;

	bool found, match;
	int loc = rightAttr.size();
	for (int i = 0; i < leftAttr.size(); i++)
	{
		found = false;
		for (int j = 0; j < rightAttr.size(); j++)
		{
			if (leftAttr[i] == rightAttr[j])
			{
				rightIndex.push_back(j);
				found = true;
				break;
			}
		}
		if (!found)
		{
			rightIndex.push_back(loc);
			loc++;
			rightLackAttr.push_back(leftAttr[i]);
		}
	}

	for (int i = 0; i < rightAttr.size(); i++)
	{
		found = false;
		for (int j = 0; j < leftAttr.size(); j++)
		{
			if (rightAttr[i] == leftAttr[j])
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			leftLackAttr.push_back(rightAttr[i]);
			rightIndex.push_back(i);
		}
	}

	/*
	cout << "LeftLackAttr:" << endl;
	for (int i = 0; i < leftLackAttr.size(); i++)
		cout << leftLackAttr[i] << endl;
	cout << "RightLackAttr:" << endl;
	for (int i = 0; i < rightLackAttr.size(); i++)
		cout << rightLackAttr[i] << endl;
	cout << "RightIndex:" << endl;
	for (int i = 0; i < rightIndex.size(); i++)
		cout << rightIndex[i] << endl;
	*/
	Relation newLeftRel, newRightRel;
	newLeftRel = ExtendRelation(leftRel, leftLackAttr);
	//PrintRelation(newLeftRel);
	newRightRel = ExtendRelation(rightRel, rightLackAttr);
	//PrintRelation(newRightRel);

	relation = newLeftRel;
	int leftSize = newLeftRel.second.size(), rightSize = newRightRel.second.size();
	for (int i = 0; i < rightSize; i++)
	{
		Tuple rightTuple = newRightRel.second[i];
		found = false;
		for (int j = 0; j < leftSize; j++)
		{
			Tuple leftTuple = newLeftRel.second[j];
			match = true;
			for (int k = 0; k < rightIndex.size(); k++)
			{
				if (leftTuple[k] != rightTuple[rightIndex[k]])
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			Tuple tuple;
			for (int k = 0; k < rightIndex.size(); k++)
				tuple.push_back(rightTuple[rightIndex[k]]);
			relation.second.push_back(tuple);
		}
	}

	return relation;
}


/***************************************************/
string QueryProcessor::TransferCharacter(const string& str)
{
	int len = str.length();
	int index = 0, newIndex = 0;
	bool inTransfer = false;
	char* newStr = new char[len+1];
	while (index < len)
	{
		if (inTransfer)
		{
			inTransfer = false;
			if (str[index] == '\\' || str[index] == '\'' || str[index] == '"')
				newStr[newIndex++] = str[index];
			else if (str[index] == 't')
				newStr[newIndex++] = '\t';
			else
				newStr[newIndex++] = str[index];
		}
		else
		{
			if (str[index] == '\\')
				inTransfer = true;
			else
				newStr[newIndex++] = str[index];
		}
		index++;
	}
	newStr[newIndex] = '\0';
	string result(newStr);
	return result;
}

void QueryProcessor::PrintRelation(const Relation& relation)
{
	cout << "Relation Info:" << endl;
	cout << "No.\t";
	for (int i = 0; i < relation.first.size(); i++)
		cout << relation.first[i] << "\t";
	cout << endl;
	
	int size = relation.second.size();
	for (int i = 0; i < size; i++)
	{
		cout << i << "\t";
		Tuple tuple = relation.second[i];
		for (int j = 0; j < tuple.size(); j++)
			cout << "<" << tuple[j].first << "," << tuple[j].second << ">\t";
		cout << endl; 
	}
	cout << endl;
}

void QueryProcessor::PrintViewTable(const ViewTable& viewTable)
{
	cout << "ViewTable:" << endl;
	int fieldCnt = viewTable.first.size();
	cout << "No." << "\t";
	for (int i = 0; i < fieldCnt; i++)
		cout << TransferCharacter(viewTable.first[i]) << "\t";
	cout << endl;
	for (int i = 0; i < viewTable.second.size(); i++)
	{
		cout << i+1 << "\t";
		for (int j = 0; j < fieldCnt; j++)
			cout << TransferCharacter(viewTable.second[i][j]) << "\t";
		cout << endl;
	}
	cout << endl;
}

void QueryProcessor::PrintShadowList(const vector<Shadow>& shadowList)
{
	cout << "ShadowList:" << endl;
	for (int i = 0; i < shadowList.size(); i++)
	{
		cout << shadowList[i].first << ":";
		for (int j = 0; j < shadowList[i].second.size(); j++)
			cout << shadowList[i].second[j] << " ";
		cout << endl;
	}
}
