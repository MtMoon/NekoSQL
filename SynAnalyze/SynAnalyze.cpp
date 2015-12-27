#include "SynAnalyze.h"
#include <string>
#include <vector>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include <utility>

using namespace std;

SynAnalyze::SynAnalyze(SysManager* smm, QueryProcessor* qpp, ErrorHandler* errorh, IndexManager* imm)
{
	assert(smm != NULL);
	assert(qpp != NULL);
	assert(errorh != NULL);
	sm = smm;
	im = imm;
	qp = qpp;
	errh = errorh;
}

SynAnalyze::~SynAnalyze() {}


char* strlwr(char *str)
{
	if (str == NULL)
		return NULL;
	
	char *p = str;
	while (*p != '\0')
	{
		if (*p >= 'A' && *p <= 'Z')
			*p = (*p) + 0x20;
		p++;
	}
	return str;
}

char* strupr(char *str)
{
	if (str == NULL)
		return NULL;
	
	char *p = str;
	while (*p != '\0')
	{
		if (*p >= 'a' && *p <= 'z')
			*p = (*p) - 0x20;
		p++;
	}
	return str;
}

string ToLowerCase(const string& str)
{
	const char* cstr = str.c_str();
	char* tstr = new char[str.length()+1];
	strcpy(tstr, cstr);
	strlwr(tstr);
	string newStr(tstr);
	delete[] tstr;
	return newStr;
}

string ToUpperCase(const string& str)
{
	const char* cstr = str.c_str();
	char* tstr = new char[str.length()+1];
	strcpy(tstr, cstr);
	strupr(tstr);
	string newStr(tstr);
	delete[] tstr;
	return newStr;
}

bool IsBlank(char c)
{
	return c == ' ' || c == '\a' || c == '\b' || c == '\f' || c == '\n'
		|| c == '\r' || c == '\t' || c == '\v' || c == '\0';
}

bool IsOperator(const string& str)
{
	return str == "and" || str == "or";
}

bool IsParenthesis(const string& str)
{
	return str == "(" || str == ")";
}

string CatStr(const vector<string>& strBuf)
{
	string str;
	if (strBuf.size() == 0)
		return str;
	for (int i = 0; i < strBuf.size()-1; i++)
		str += strBuf[i]+" ";
	str += strBuf[strBuf.size()-1];
	return str;
}

string StripStr(const string& str)
{
	int len = str.length();
	if (len == 0)
		return string("");

	int left = len, right = len;
	for (int i = 0; i < len; i++)
	{
		if (!IsBlank(str[i]))
		{
			left = i;
			break;
		}
	}
	if (left == len)
		return string("");

	for (int i = len-1; i >= left; i--)
	{
		if (!IsBlank(str[i]))
		{
			right = i;
			break;
		}
	}
	string newStr(str.substr(left, right-left+1));
	return newStr;
}

vector<string> SplitStr(const string& str, char c)
{
	vector<string> result;
	int start = 0;
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == c)
		{
			string temp(str.substr(start, i-start));
			result.push_back(temp);
			start = i+1;
		}
	}
	string temp(str.substr(start, str.length()-start));
	result.push_back(temp);
	return result;
}

vector<string> SplitStrWithLimit(const string& str, char c, const vector<char>& leftChar, const vector<char>& rightChar)
{
	assert(leftChar.size() == rightChar.size());
	int charCnt = leftChar.size();
	vector<string> strVec;
	int start = 0;
	int stage = -1; //priority stage
	bool inChange = false, inLock = false;
	vector<int> stageVec;

	for (int i = 0; i < str.length(); i++)
	{
		if (inLock)
		{
			if (inChange)
				inChange = false;
			else
			{
				if (str[i] == '\\')
					inChange = true;
				else if (str[i] == '\'')
					inLock = false;
			}
		}
		else
		{
			if (str[i] == '\'')
			{
				inLock = true;
				continue;
			}
			bool found = false;
			for (int j = stage+1; j < charCnt; j++)
				if (str[i] == leftChar[j])
				{
					stageVec.push_back(stage);
					stage = j;
					found = true;
				}
			if (found)
				continue;
			if (stage != -1 && str[i] == leftChar[stage] && str[i] != rightChar[stage])
			{
				stageVec.push_back(stage);
				continue;
			}
			if (stage != -1 && str[i] == rightChar[stage])
			{
				stage = stageVec.back();
				stageVec.pop_back();
				continue;
			}
			if (str[i] == c && stage == -1)
			{
				string temp(str.substr(start, i-start));
				strVec.push_back(temp);
				start = i+1;
			}
		}
	}
	string temp(str.substr(start, str.length()-start));
	strVec.push_back(temp);
	return strVec;
}

vector<string> SplitStrNonEmpty(const string& str, char c)
{
	vector<string> strVec = SplitStr(str, c);
	for (int i = 0; i < strVec.size(); i++)
		strVec[i] = StripStr(strVec[i]);
	vector<string> result;
	for (int i = 0; i < strVec.size(); i++)
		if (strVec[i].length() != 0)
			result.push_back(strVec[i]);
	return result;
}

vector<string> RemoveEmptyVector(const vector<string>& strVec)
{
	vector<string> result;
	for (int i = 0; i < strVec.size(); i++)
		if (StripStr(strVec[i]).length() != 0)
			result.push_back(strVec[i]);
	return result;
}

vector<string> StripVector(const vector<string>& strVec)
{
	vector<string> result;
	for (int i = 0; i < strVec.size(); i++)
		result.push_back(StripStr(strVec[i]));
	return result;
}

string GetWord(string& cstr)
{
	string str(cstr+" ");
	int start = 0;
	bool inWord = false;
	for (int i = 0; i < str.length(); i++)
	{
		if (IsBlank(str[i]))
		{
			if (inWord)
			{
				inWord = false;
				string tempStr = str.substr(start, i-start);
				cstr = cstr.substr(i, cstr.length()-i);
				return tempStr;
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}
		}
	}
	cstr = "";
	return "";
}

bool FindWordAndSplit(const string& cstr, const string& target, vector<string>& strVec)
{
	strVec.clear();
	string str(cstr+" ");
	int start = 0;
	bool inWord = false;
	int leftEnd = -1, rightStart = -1;
	for (int i = 0; i < str.length(); i++)
	{
		if (IsBlank(str[i]))
		{
			if (inWord)
			{
				inWord = false;
				string tempStr = ToLowerCase(str.substr(start, i-start));
				if (tempStr == target)
				{
					leftEnd = start;
					rightStart = i;
					break;
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}
		}
	}
	if (leftEnd == -1 && rightStart == -1)
		return false;
	strVec.push_back(cstr.substr(0, leftEnd));
	strVec.push_back(cstr.substr(rightStart, cstr.length()-rightStart));
	return true;
}

bool FindWordAndSplitWithLimit(const string& cstr, const string& target, vector<string>& strVec)
{
	strVec.clear();
	string str(cstr+" ");
	int start = 0;
	bool inWord = false, inLock = false, inChange = false;
	int leftEnd = -1, rightStart = -1;
	for (int i = 0; i < str.length(); i++)
	{
		if (IsBlank(str[i]) && !inLock)
		{
			if (inWord)
			{
				inWord = false;
				string tempStr = ToLowerCase(str.substr(start, i-start));
				if (tempStr == target)
				{
					leftEnd = start;
					rightStart = i;
					break;
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}
			if (!inLock)
			{
				if (str[i] == '\'')
					inLock = true;
			}
			else
			{
				if (inChange)
					inChange = false;
				else
				{
					if (str[i] == '\\')
						inChange = true;
					else if (str[i] == '\'')
						inLock = false;
				}
			}
		}
	}
	if (leftEnd == -1 && rightStart == -1)
		return false;
	strVec.push_back(cstr.substr(0, leftEnd));
	strVec.push_back(cstr.substr(rightStart, cstr.length()-rightStart));
	return true;
}

bool IsInt(const string& value)
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
	return true;
}

bool IsStr(const string& str)
{
	if (str.length() < 2)
		return false;
	return str[0] == '\"' && str[str.length()-1] == '\"' || str[0] == '\'' && str[str.length()-1] == '\'';
}

string WordToStr(const string& str)
{
	assert(IsStr(str));
	return str.substr(1, str.length()-2);
}


//for SysManager
//若是SysManager对应的命令，则返回结果string，且置flag:
//flag == 0 该命令不是SysManager范围的mingl
//flag == 1 该命令是SysManager范围的命令且处理成功
//flag == -1 该命令是sysManager范围的命令，但处理失败
string SynAnalyze::SysAnalyze(const std::string& cstr, int & flag) {
	flag = 0;
	if (cstr[cstr.length()-1] != ';') {
		errh->ErrorHandle("COMMAND", "syntax", "command should end with \";\"");
		return "";
	}
	string str(cstr.substr(0, cstr.length()-1)+" ");
	if (StripStr(str).length() == 0) {
		cout << "emtpy cmd." << endl;
		return "";
	}
	bool inWord = false;
	int start = 0;
	string word[2];
	string remainStr = "";
	int wordCount = 0;
	for (int i = 0; i < str.length(); i++) {
		if (IsBlank(str[i])) {
			if (inWord) {
				inWord = false;
				string tempWord = ToLowerCase(str.substr(start, i-start));
				remainStr = str.substr(i, str.length()-i);
				word[wordCount] = tempWord;
				wordCount++;
				if (wordCount == 2) {
					break;
				}
			}
		} else {
			if (!inWord) {
				inWord = true;
				start = i;
			}
		}
	}
	cout << "#" << word[0] << "#" << word[1] << "#" << remainStr << endl;
	if (word[0] == "create" && word[1] == "table") { //单独处理建表
		flag = -1;
		int ans = createTBAnalyze(remainStr);
		//cout << "ans:" << ans << endl;
		if (ans == -2) {
			return "";
		} else if (ans == 0) {
			cout << "Table already exisits! " << endl;
			return "";
		} else if (ans == -1) {
			cout << "no selected database! " << endl;
			return "";
		}

		flag = 1;
		return "";
	}

	if  (word[0] == "create" && word[1] == "database") { //建数据库
		flag = -1;
		int ans = sm->createDatabase( ToLowerCase(GetWord(remainStr)));
		if (ans == 0) {
			cout << "database " <<    ToLowerCase(GetWord(remainStr)) << " already exists! " << endl;
			return "";
		} else if (ans == -1) {
			cout << "unknown error in function createDatabase() " << endl;
			return "";
		}
		flag = 1;
		return "";
	}

	//删除数据库
	if (word[0] == "drop" && word[1] == "database") {
		flag = -1;
		int ans = sm->dropDatabase( ToLowerCase(GetWord(remainStr)));
		if (ans == 0) {
			cout << "database " <<   ToLowerCase(GetWord(remainStr)) << " not exists! " << endl;
			return "";
		} else if (ans == -1) {
			cout << "unknown error in function dropDatabase() " << endl;
			return "";
		}
		flag = 1;
		return "";
	}

	//删除索引
	if (word[0] == "drop" && word[1] == "index") {
		flag = -1;
		vector<string> ans = SplitStr(remainStr, ' ');
		vector<string> symbol;
		for (int i=0; i<ans.size(); i++) {
			if (ans[i] == "" || ans[i] == " ") {
				continue;
			}
			symbol.push_back( ToLowerCase(GetWord(ans[i])));
		}

		if (symbol.size() != 2) {
			cout << "rong index operation statement! " << endl;
			return "";
		}
		int ansFlag = im->dropIndex(symbol[0], symbol[1]);
		if (ansFlag == 0) {
			cout << "index " << symbol[1] << " not exists!" << endl;
			return "";
		} else if (ansFlag == -1) {
			cout << "no selected database! " << endl;
			return "";
		} else if (ansFlag == -2) {
			cout << "database not exists!" << endl;
			return "";
		}

		flag = 1;
		return "";
	}

	//创建索引
	if (word[0] == "create" && (word[1] == "unique" || word[1] == "nonunique")) {
		flag = -1;
		vector<string> ans = SplitStr(remainStr, ' ');
		vector<string> symbol;
		for (int i=0; i<ans.size(); i++) {
			if (ans[i] == "" || ans[i] == " ") {
				continue;
			}
			symbol.push_back( ToLowerCase(GetWord(ans[i])));
		}

		if (symbol.size() != 4) {
			cout << "rong index operation statement in creating index! " << endl;
			return "";
		}

		string indexName = symbol[1];
		//获取表名和列名
		int start = symbol[3].find("(");

		string fieldName = symbol[3].substr(start+1, symbol[3].length()-start-2);
		string tableName = symbol[3].substr(0,start);

		//cout << "tableName:#" << tableName << "# indexName:#" << indexName << "# fieldName:#" << fieldName << "#" << endl;
		IndexInfo indexinfo;
		indexinfo.indexName = indexName;
		indexinfo.tableName = tableName;
		indexinfo.fieldName = fieldName;
		if (word[1] == "unique") {
			indexinfo.indexType = 0;
		} else {
			indexinfo.indexType = 1;
		}
		indexinfo.ifNull = 0;
		TableInfo tb = sm->getTableInfo(tableName);

		//找到是第几个字段
		int which = -1;
		for (int i=0; i<tb.FN; i++) {
			if (fieldName == tb.Fname[i]) {
				which = i;
				break;
			}
		}

		if (which == -1) {
			for (int i=0; i<tb.VN; i++) {
				if (tb.Vname[i] == fieldName) {
					which = tb.FN + i;
					break;
				}
			}
		}

		if (which == -1) {
			cout << "Fitled " << fieldName << " not int table " << tableName << endl;
			flag = -1;
			return "";
		}

		int whichByte = which/8;
		int whichBit = which%8;

		if (((tb.nullMap[whichByte]>>whichBit) & 1) == 1) {
			cout << "index fitled can't be null!"  << endl;
			flag = -1;
			return "";
		}


		if (RecordTool::isVCol(tb, fieldName)) {
			indexinfo.ifFixed = 0;
			indexinfo.fieldLen = 0;
			indexinfo.fieldType = 1;
		} else { //定长
			indexinfo.ifFixed = 1;
			indexinfo.fieldType = 0;
			indexinfo.fieldLen = 4;
		}
		indexinfo.legal = true;
		cout << "lalala2" << endl;
		//im->setIndex(tableName, indexName);
		cout << "lalala3" << endl;
		int ansFlag = im->createIndex(indexinfo);
		if (ansFlag == 0) {
			cout << "index " << indexName << " already exists!" << endl;
			return "";
		} else if (ansFlag == -1) {
			cout << "no selected database! " << endl;
			return "";
		} else if (ansFlag == -2) {
			cout << "database not exists!" << endl;
			return "";
		} else if (ansFlag == -4) {
			cout << "There has been an index in field " << fieldName << " of table " << tableName << endl;
			return "";
		} else if (ansFlag == -3) {
			cout << "Unknown error in IndexManager!" << endl;
			return "";
		}

		flag = 1;
		return "";

	}





	//user database
	if (word[0] == "use") {
		flag = -1;
		string para = ToLowerCase(GetWord(word[1]));
		int ans = sm->useDatabase(para);
		im->setDataBase(para);
		if (ans == 0) {
			cout << "database " <<  para << " not exists! " << endl;
			return "";
		} else if (ans == -1) {
			cout << "unknown error in function useDatabase() " << endl;
			return "";
		}
		flag = 1;
		return "";
	}

	//删除表
	if (word[0] == "drop" && word[1] == "table") {
		flag = -1;
		string para = ToLowerCase(GetWord(remainStr));
		int ans = sm->dropTable(para);
		if (ans == 0) {
			cout << "table " <<   para << " not exists! " << endl;
			return "";
		} else if (ans == -1) {
			cout << "no selected database! " << endl;
			return "";
		}
		flag = 1;
		return "";
	}

	//显示数据库中所有表
	if (word[0] == "show" && word[1] == "tables") {
		flag = -1;
		int ansflag = 0;
		vector<string> ans = sm->showTables(ansflag);
		if (ansflag == 0) {
			cout << "The database is empty! " << endl;
			return "";
		} else if (ansflag == -1) {
			cout << "no selected database! " << endl;
			return "";
		} else if (ansflag == -2) {
			cout << "unknown error in function showTables() " << endl;
			return "";
		}

		string finalAns = "";
		int border = 20;
		for (int i=0; i<border; i++) {
			finalAns += "_";
		}
		finalAns += "\n";

		for (int i=0; i<ans.size(); i++) {
			finalAns +=  ans[i];
			for (int j=ans[i].length()+1; j<border*1.3; j++) {
				finalAns += " ";
			}
			finalAns += "\n";
			for (int j=0; j<border; j++) {
				finalAns += "_";
			}
			finalAns += "\n";
		}

		flag = 1;
		return finalAns;

	}

	//显示某个表的所有信息
	if (word[0] == "desc") {

		//显示pos信息
		vector<int> posInfo = sm->getPosInfo(word[1]);
		cout << "pos info: " << endl;
		for (int i=0; i<posInfo.size(); i++) {
			cout << posInfo[i] << endl;
		}

		flag = -1;
		int ansflag = 0;
		vector<FieldInfo> ans = sm->descTable(word[1], ansflag);
		if (ansflag == 0) {
			cout << "Table: " << word[1] << " not exisits! " << endl;
			return "";
		} else if (ansflag == -1) {
			cout << "no selected database! " << endl;
			return "";
		}

		flag = 1;
		string finalAns = "";
		int border = 50;
		for (int i=0; i<border*1.2; i++) {
			finalAns += "_";
		}
		finalAns += "\n";

		finalAns += "|FieldName";
		for (int i=10; i<border/5; i++) {
			finalAns += " ";
		}

		finalAns += "|FieldType";
		for (int i=10; i<border/5; i++) {
			finalAns += " ";
		}

		finalAns += "|FieldSize";
		for (int i=10; i<border/5; i++) {
			finalAns += " ";
		}

		finalAns += "|IfNull";
		for (int i=7; i<border/5; i++) {
				finalAns += " ";
		}

		finalAns += "|key";
		for (int i=4; i<border/5; i++) {
				finalAns += " ";
		}
		finalAns += " |\n";

		for (int i=0; i<border*1.2; i++) {
				finalAns += "_";
		}
		finalAns += "\n";

		for (int i=0; i<ans.size(); i++) {
			FieldInfo info = ans[i];
			//字段名
			finalAns += "|" + info.fieldName;
			for (int j=info.fieldName.length()+1; j<border/5; j++) {
				finalAns += " ";
			}
			//字段类型
			string typeStr = "";
			if (info.fieldType == 0) {
				typeStr = "int";
			} else if (info.fieldType == 1) {
				typeStr = "char";
			} else if (info.fieldType == 2) {
				typeStr = "varchar";
			}
			finalAns += "|" + typeStr;
			for (int j=typeStr.length()+1; j<border/5; j++) {
				finalAns += " ";
			}

			//字段长度
			stringstream ss;
			ss.clear();
			ss << info.fieldSize;
			string sizeStr = "";
			ss >> sizeStr;
			finalAns += "|" + sizeStr;
			for (int j=sizeStr.length()+1; j<border/5; j++) {
				finalAns += " ";
			}

			//是否为null
			string nullStr = "";
			if (info.ifNull) {
				nullStr = "true";
			} else {
				nullStr = "false";
			}

			finalAns += "|" + nullStr;
			for (int j=nullStr.length()+1; j<border/5; j++) {
				finalAns += " ";
			}

			//key类型
			string keyStr = "";
			if (info.key == 0) {
				keyStr = "no key";
			} else if (info.key == 1) {
				keyStr = "normal";
			} else if (info.key == 2) {
				keyStr = "primary";
			}

			finalAns += "|" + keyStr;
			for (int j=keyStr.length()+1; j<border/5; j++) {
				finalAns += " ";
			}

			finalAns += "|\n";

			for (int j=0; j<border*1.2; j++) {
				finalAns += "_";
			}
			finalAns += "\n";
		}


		flag = 1;
		return finalAns;


	}
	if (flag != 0) {
		cout << "command not found." << endl;
	}
	return "";
}

//处理建表
//return -2 该函数内错误错误 1 成功 0 表已存在 -1 未选中database
int SynAnalyze::createTBAnalyze(const std::string& cstr) {
	string str = cstr.substr(0,cstr.length()-2);
	//cout << "remain str: " << str << endl;
	int start = str.find("(");
	string name = str.substr(0, start);
	string tableName = GetWord(name);
	//cout << "tableName:#" << tableName << "#" << endl;
	str = str.substr(start+1, str.length()-start-1);
	//cout << str << endl;

	vector<string> states = SplitStr(str,',');
	vector<FieldInfo> tbInfo;

	int stateSize = states.size() ;
	//cout << stateSize << endl;


	for (int i=0; i<stateSize; i++) {
		//cout << "#" << states[i] << "#" << endl;
		//对每一句
		vector<string> eachStr = SplitStr(states[i],' ');
		vector<string> symbol;
		for (unsigned j = 0; j<eachStr.size(); j++) {
			if (eachStr[j] == "" || eachStr[j] == " ") {
				continue;
			}
			symbol.push_back(GetWord(eachStr[j]));
		}

		start = symbol[1].find("(");
		int end = symbol[1].find(")");
		//cout << "start:" << start << " end: " << end << endl;
		string sizeStr = ToLowerCase(symbol[1].substr(start+1,end-start-1));
		string typeStr = ToLowerCase(symbol[1].substr(0,start));
		cout << "type:" << typeStr << "#  size:" << sizeStr << "#" << endl;

		if (typeStr == "key") {
			int templen = symbol[2].length();
			string need = symbol[2].substr(1, templen-2);
			//cout << "need; " << need << endl;
			for (unsigned j=0; j<tbInfo.size(); j++) {
				if (tbInfo[j].fieldName == need) {
					if (tbInfo[j].ifNull) {
						cout << "The primary key can't be null!" << endl;
						return -2;
					}
					tbInfo[j].key = 2;
					break;
				}
			}

		} else {
			FieldInfo info;
			info.fieldName = symbol[0];
			if (typeStr == "int") {
				info.fieldType = 0;
			} else if (typeStr == "char") {
				info.fieldType = 1;
			} else if (typeStr == "varchar") {
				info.fieldType = 2;
			} else {
				cout << "Type Error, " << typeStr << " is not a legal type! " << endl;
				return -2;
			}

			//key
			info.key = 0;
			//if null
			//cout << "lalala0" << endl;
			if (symbol.size() > 2 && ToLowerCase(symbol[2]) == "not") {
				info.ifNull = false;
			} else {
				info.ifNull = true;
			}
			//cout << "lalala1" << endl;
			// size
			stringstream ss;
			ss.clear();
			ss << sizeStr;
			int size = 0;
			ss >> size;
			info.fieldSize = size;
			tbInfo.push_back(info);
			//cout << "lalala" << endl;
		}
	}
	//建表
	return sm->createTable(tableName, tbInfo);

}
//end of SysManager

//check one tableName(not include renaming)
bool SynAnalyze::OneTableAnalyze(const string& cstr, vector<TableAlias>& tableAliasList, const string& opType)
{

	tableAliasList.clear();
	string tableName = StripStr(cstr);

	int flag;
	sm->descTable(tableName, flag);
	if (flag == -1)
	{
		errh->ErrorHandle(opType, "database", "No Database selected.");
		return false;
	}
	else if (flag == 0)
	{
		errh->ErrorHandle(opType, "database", "Table "+tableName+" does not exist.");
		return false;
	}
	TableAlias tableAlias(tableName, tableName);
	tableAliasList.push_back(tableAlias);

	return true;
}

bool SynAnalyze::FromAnalyze(const string& str, vector<TableAlias>& tableAliasList)
{

	tableAliasList.clear();
	//naming analyze
	vector<string> strVec = SplitStr(str, ',');
	for (int i = 0; i < strVec.size(); i++)
	{
		//cout << i << ":" << strVec[i] << endl;
		string temp = StripStr(strVec[i]);
		vector<string> tempVec = SplitStrNonEmpty(temp, ' ');
		if (tempVec.size() == 0)
		{
			errh->ErrorHandle("FROM", "sematic", "empty table name");
			return false;
		}
		else if (tempVec.size() == 1 || tempVec.size() == 3)
		{
			string tableName, tableNick;
			tableName = StripStr(tempVec[0]);
			if (tableName.length() == 0)
			{
					errh->ErrorHandle("FROM", "sematic", "empty table name:"+temp);
					return false;
			}
			int flag;
			sm->descTable(tableName, flag);
			if (flag == -1)
			{
				errh->ErrorHandle("FROM", "database", "No Database selected.");
				return false;
			}
			else if (flag == 0)
			{
				errh->ErrorHandle("FROM", "database", "Table "+tableName+" does not exist.");
				return false;
			}
			
			if (tempVec.size() == 3)
			{
				string token = ToLowerCase(StripStr(tempVec[1]));
				if (token != "as")
				{
					errh->ErrorHandle("FROM", "sematic", "invalid table naming format:"+temp);
					return false;
				}
				tableNick = StripStr(tempVec[2]);
				if (tableNick.length() == 0)
				{
					errh->ErrorHandle("FROM", "sematic", "emtpy nickname for table "+tableName);
					return false;
				}
				if (tableNick == tableName)
				{
					errh->ErrorHandle("FROM", "sematic", "table nickname is same as tablename:"+tableName);
					return false;
				}
			}
			else
			{
				tableNick = tableName;
			}
			tableAliasList.push_back(TableAlias(tableName, tableNick));
		}
		else
		{
			errh->ErrorHandle("FROM", "sematic", "invalid table naming format:"+temp);
			return false;
		}
	}
	/*
	cout << "Naming:" << endl;
	for (int i = 0; i < tableAliasList.size(); i++)
		cout << tableAliasList[i].first << ":" << tableAliasList[i].second << endl;
	*/
	//check naming conflict
	int size = tableAliasList.size();
	for (int i = 0; i < size; i++)
		if (tableAliasList[i].second != tableAliasList[i].first)
			for (int j = 0; j < size; j++)
				if (tableAliasList[i].second == tableAliasList[j].first)
				{
					errh->ErrorHandle("FROM", "sematic", "table alias:"+tableAliasList[i].second+" conflicts with table name:"+tableAliasList[j].first);
					return false;
				}
	for (int i = 0; i < size; i++)
		if (tableAliasList[i].first == tableAliasList[i].second)
			for (int j = 0; j < size; j++)
				if (i != j && tableAliasList[j].first == tableAliasList[j].second && tableAliasList[i].first == tableAliasList[j].first)
				{
					errh->ErrorHandle("FROM", "sematic", "table:"+tableAliasList[i].first+" needs renaming");
					return false;
				}
	for (int i = 0; i < size; i++)
		if (tableAliasList[i].second != tableAliasList[i].first)
			for (int j = 0; j < size; j++)
				if (i != j && tableAliasList[j].second != tableAliasList[j].first && tableAliasList[i].second == tableAliasList[j].second)
				{
					errh->ErrorHandle("FROM", "sematic", "table alias:"+tableAliasList[i].second+" have confliction");
					return false;
				}

	return true;
}


vector<string> SynAnalyze::WhereLexAnalyze(const string& cstr)
{

	string str = cstr+"(";
	int len = str.length();
	bool inWord = false, inLock = false, inChange = false;
	int start = 0;
	vector<string> strBuf, result;

	for (int i = 0; i < len; i++)
	{
		if (IsBlank(str[i]) && !inLock)
		{
			if (inWord)
			{
				inWord = false;
				string tempWord = str.substr(start, i-start);
				string lowerWord = ToLowerCase(tempWord);
				if (lowerWord == "and" || lowerWord == "or")
				{
					string tempSen = CatStr(strBuf);
					if (tempSen.length() != 0)
						result.push_back(tempSen);
					result.push_back(lowerWord);
					strBuf.clear();
				}
				else
					strBuf.push_back(tempWord);
			}
		}
		else if ((str[i] == '(' || str[i] == ')') && !inLock)
		{
			string tempSen;
			if (!inWord)
			{
				tempSen = CatStr(strBuf);
				if (tempSen.length() != 0)
					result.push_back(tempSen);
				strBuf.clear();
				result.push_back((str[i] == '(') ? "(" : ")");
			}
			else
			{
				string tempWord = str.substr(start, i-start);
				string lowerWord = ToLowerCase(tempWord);
				if (lowerWord != "and" && lowerWord != "or")
					strBuf.push_back(tempWord);
				tempSen = CatStr(strBuf);
				if (tempSen.length() != 0)
					result.push_back(tempSen);
				strBuf.clear();
				if (lowerWord == "and" || lowerWord == "or")
					result.push_back(lowerWord);
				result.push_back((str[i] == '(') ? "(" : ")");
				inWord = false;
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}

			if (inLock)
			{
				if (inChange)
				{
					inChange = false;
				}
				else
				{
					if (str[i] == '\\')
						inChange = true;
					else if (str[i] == '\'')
						inLock = false;
				}
			}
			else
			{
				if (str[i] == '\'')
					inLock = true;
			}
		}

	} 

	if (!inLock)
		result.pop_back();
	else
	{
		string tempWord = str.substr(start, str.length()-1-start);
		strBuf.push_back(tempWord);
		string tempSen = CatStr(strBuf);
		if (tempSen.length() != 0)
			result.push_back(tempSen);
	}
	return result;
}


bool SynAnalyze::CheckWhereSyntax(const vector<string>& tokens)
{

	int size = tokens.size();
	int parenCnt = 0;
	for (int i = 0; i < size; i++)
	{
		if (tokens[i] == "(")
			parenCnt++;
		else if (tokens[i] == ")")
		{
			parenCnt--;
			if (parenCnt < 0)
			{
				errh->ErrorHandle("WHERE", "syntax", "Parentheses mismatch.");
				return false;
			}
		}
	}
	if (parenCnt != 0)
	{
		errh->ErrorHandle("WHERE", "syntax", "Parentheses mismatch.");
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		if (IsOperator(tokens[i]))
		{
			if (i == 0 || i == size-1 || IsOperator(tokens[i-1]) || tokens[i-1] == "(" || IsOperator(tokens[i+1]) || tokens[i+1] == ")")
			{
				string msg;
				msg += (i == 0) ? "" : tokens[i-1]+" ";
				msg += tokens[i];
				msg += (i == size-1) ? "" : " "+tokens[i+1];
				errh->ErrorHandle("WHERE", "syntax", "operator \""+tokens[i]+"\" needs two operand:"+msg);
				return false;
			}
		}
		else if (!IsParenthesis(tokens[i]))
		{
			bool valid = true;
			if (!(i == 0 || IsOperator(tokens[i-1]) || tokens[i-1] == "("))
				valid = false;
			if (!(i == size-1 || IsOperator(tokens[i+1]) || tokens[i+1] == ")"))
				valid = false;
			if (!valid)
			{
				string msg;
				msg += (i == 0) ? "" : tokens[i-1]+" ";
				msg += tokens[i];
				msg += (i == size-1) ? "" : " "+tokens[i+1];
				errh->ErrorHandle("WHERE", "syntax", "invalid syntax:"+msg);
				return false;
			}
		}
		else if (tokens[i] == ")")
		{
			assert(i > 0);
			if (tokens[i-1] == "(")
			{
				errh->ErrorHandle("WHERE", "syntax", "emtpy content in parenthesis.");
				return false;
			}
		}
	}
	return true;
}

vector<string> SynAnalyze::WhereSynAnalyze(const vector<string>& words)
{
	map<string, int> priority;
	vector<string> symbols, result;

	symbols.push_back("#");
	priority["#"] = 0;
	priority["or"] = 1;
	priority["and"] = 2;
	priority["("] = 0;
	priority[")"] = 0;
	int size = words.size();
	
	for (int i = 0; i < size; i++)
	{
		if (words[i] == "(")
		{
			symbols.push_back(words[i]);
		}
		else if (words[i] == ")")
		{
			int index = symbols.size()-1;
			while (symbols[index] != "(")
			{
				result.push_back(symbols[index]);
				symbols.pop_back();
				index--;
			}
			symbols.pop_back();
		}
		else if (IsOperator(words[i]))
		{
			int index = symbols.size()-1;
			while(priority[symbols[index]] >= priority[words[i]])
			{
				result.push_back(symbols[index]);
				symbols.pop_back();
				index--;
			}
			symbols.push_back(words[i]);
		}
		else
		{
			result.push_back(words[i]);
		}
	}
	int index = symbols.size()-1;
	while (symbols[index] != "#")
	{
		result.push_back(symbols[index]);
		symbols.pop_back();
		index--;
	}

	return result;
}

bool SynAnalyze::TableFieldAnalyze(const string& cstr, const vector<TableAlias>& tableAliasList, vector<string>& result, const string& opType)
{
	result.clear();
	string str = StripStr(cstr);
	assert(tableAliasList.size() > 0);
	bool isSingle = (tableAliasList.size() == 1) ? true : false;

	for (int i = 0; i < str.length(); i++)
		if (IsBlank(str[i]))
		{
			errh->ErrorHandle(opType, "sematic", "blank character shouldn't exist:"+str);
			return false;
		}

	vector<string> words = SplitStr(str, '.');
	if (words.size() == 1 && !isSingle)
	{
		errh->ErrorHandle(opType, "sematic", "\".\" not found:"+str);
		return false;
	}
	else if (words.size() >= 3)
	{
		errh->ErrorHandle(opType, "sematic", "more than one \".\" are found:"+str);
		return false;
	}

	string table;
	string field;
	if (isSingle && words.size() == 1)
	{
		table = tableAliasList[0].second;
		field = words[0];
	}
	else
	{
		table = words[0];
		field = words[1];
	}

	if (table.length() == 0)
	{
		errh->ErrorHandle(opType, "sematic", "empty table name:"+str);
		return false;
	}
	if (field.length() == 0)
	{
		errh->ErrorHandle(opType, "sematic", "empty field name:"+str);
		return false;
	}
	bool found = false;
	for (int i = 0; i < tableAliasList.size(); i++)
		if (table == tableAliasList[i].second)
		{
			found = true;
			break;
		}
	if (!found)
	{
		errh->ErrorHandle(opType, "sematic", "table name \""+table+"\" not found:"+str);
		return false;
	}
	result.push_back(table);
	result.push_back(field);
	return true;
}

//type: 1:table field 2:str or int const 0:operator -1:empty str
bool SynAnalyze::WhereWordAnalyze(const string& cstr, const vector<TableAlias>& tableAliasList, vector<WordInfo>& wordInfoList)
{

	wordInfoList.clear();
	string str(cstr+" ");
	bool inWord = false, inLock = false, inChange = false;
	int start = 0, leftEnd = -1, rightStart = -1;
	vector<string> strBuf;
	for (int i = 0; i < str.length(); i++)
	{
		/*
		cout << str[i] << ":";
		cout << ((inWord) ? "inWord" : "") << " ";
		cout << ((inLock) ? "inLock" : "") << " ";
		cout << ((inChange) ? "inChange" : "") << endl;
		*/
		if (IsBlank(str[i]) && !inLock)
		{
			if (inWord)
			{
				inWord = false;
				string tempWord = ToLowerCase(str.substr(start, i-start));
				if (tempWord == "is")
				{
					strBuf.clear();
					strBuf.push_back(tempWord);
					leftEnd = start;
				}
				else if (tempWord == "not")
				{
					if (strBuf[strBuf.size()-1] != "is")
						strBuf.clear();
					else
						strBuf.push_back(tempWord);
				}
				else if (tempWord == "null")
				{
					string topWord = strBuf[strBuf.size()-1];
					if (topWord == "not" || topWord == "is")
					{
						strBuf.push_back(tempWord);
						rightStart = i;
						break;
					}
					else
						strBuf.clear();
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}

			if (!inLock)
			{
				if (str[i] == '\'')
					inLock = true;
				else if (str[i] == '=')
				{
					leftEnd = i;
					rightStart = i+1;
					strBuf.clear();
					strBuf.push_back("=");
					break;
				}
				else if (str[i] == '!')
				{
					if (str[i+1] == '=')
					{
						leftEnd = i;
						rightStart = i+2;
						strBuf.clear();
						strBuf.push_back("!=");
						break;
					}
				}
				else if (str[i] == '>' || str[i] == '<')
				{
					if (str[i+1] == '=')
					{
						leftEnd = i;
						rightStart = i+2;
						strBuf.clear();
						strBuf.push_back((str[i] == '>') ? ">=" : "<=");
						break;
					}
					else
					{
						leftEnd = i;
						rightStart = i+1;
						strBuf.clear();
						strBuf.push_back((str[i] == '>') ? ">" : "<");
						break;
					}
				}
			}
			else
			{
				if (inChange)
					inChange = false;
				else
				{
					if (str[i] == '\\')
						inChange = true;
					else if (str[i] == '\'')
						inLock = false;
				}
			}
		}
	}

	if (inLock)
	{
		errh->ErrorHandle("WHERE", "sematic", "\' mismatch:"+cstr);
		return false;
	}
	if (leftEnd == -1 || rightStart == -1)
	{
		errh->ErrorHandle("WHERE", "sematic", "operator not found:"+cstr);
		return false;
	}
	
	string op = CatStr(strBuf);
	//cout << "Operator:" << op << endl;
	string leftOperand = StripStr(str.substr(0, leftEnd));
	string rightOperand = StripStr(str.substr(rightStart, str.length()-rightStart));
	if (op == "is null" || op == "is not null")
	{ 
		if (rightOperand.length() != 0)
		{
			errh->ErrorHandle("WHERE", "sematic", "invalid character after \""+op+"\":"+cstr);
			return false;
		}
		if (leftOperand.length() == 0)
		{
			errh->ErrorHandle("WHERE", "sematic", "empty operand for \""+op + "\":"+cstr);
			return false;
		}
		if (IsInt(leftOperand) || IsStr(leftOperand))
		{
			errh->ErrorHandle("WHERE", "sematic", "table field is needed:"+cstr);
			return false;
		}
		vector<string> tempVec;
		if (!TableFieldAnalyze(leftOperand, tableAliasList, tempVec, "WHERE"))
			return false;
		WordInfo wordInfo = WordInfo(tempVec[0], tempVec[1], "", 1);
		wordInfoList.push_back(wordInfo);
		wordInfo = WordInfo("", "", (op == "is null") ? "Null" : "not Null", 0);
		wordInfoList.push_back(wordInfo);
		wordInfo = WordInfo();
		wordInfoList.push_back(wordInfo);
	}
	else
	{
		if (leftOperand.length() == 0 || rightOperand.length() == 0)
		{
			errh->ErrorHandle("WHERE", "sematic", "emtpy operand for \""+op+"\":"+cstr);
			return false;
		}
		bool leftIsConst = true, rightIsConst = true;
		vector<string> tempVec;
		if (IsStr(leftOperand))
			leftOperand = WordToStr(leftOperand);
		else if (!IsInt(leftOperand))
			leftIsConst = false;
		if (IsStr(rightOperand))
			rightOperand = WordToStr(rightOperand);
		else if (!IsInt(rightOperand))
			rightIsConst = false;
		/*
		cout << ((leftIsConst)?"const":"not const") << ":";
		cout << leftOperand << endl;
		cout << ((rightIsConst)?"const":"not const") << ":";
		cout << rightOperand << endl;
		*/
		if (leftIsConst && rightIsConst)
		{
			errh->ErrorHandle("WHERE", "sematic", "table field is need for \"" + op + "\":"+cstr);
			return false;
		}
		if (leftIsConst && !rightIsConst)
		{
			string tempStr = rightOperand;
			rightOperand = leftOperand;
			leftOperand = tempStr;
			leftIsConst = false;
			rightIsConst = true;
			if (op == ">")
				op = "<";
			else if (op == "<")
				op = ">";
			else if (op == ">=")
				op = "<=";
			else if (op == "<=")
				op = ">=";
		}
		
		vector<string> leftVec, rightVec;
		WordInfo wordInfo;
		if (!leftIsConst)
		{
			if (!TableFieldAnalyze(leftOperand, tableAliasList, leftVec, "WHERE"))
				return false;
			wordInfo = WordInfo(leftVec[0], leftVec[1], "", 1);
		}
		else
			wordInfo = WordInfo("", "", leftOperand, 2);
		wordInfoList.push_back(wordInfo);
		wordInfo = WordInfo("", "", op, 0);
		wordInfoList.push_back(wordInfo);
		if (!rightIsConst)
		{
			if (!TableFieldAnalyze(rightOperand, tableAliasList, rightVec, "WHERE"))
				return false;
			wordInfo = WordInfo(rightVec[0], rightVec[1], "", 1);
		}
		else
			wordInfo = WordInfo("", "", rightOperand, 2);
		wordInfoList.push_back(wordInfo);
	}
	return true;
}

bool SynAnalyze::WhereAnalyze(const string& str, const vector<TableAlias>& tableAliasList, Relation& relation)
{

	int tableCnt = tableAliasList.size();
	assert(tableCnt > 0);
	vector< vector<FieldInfo> > tableInfoList;
	int flag;
	for (int i = 0; i < tableCnt; i++)
	{
			vector<FieldInfo> tempFieldInfo = sm->descTable(tableAliasList[i].first, flag);
			assert(flag == 1);
			tableInfoList.push_back(tempFieldInfo);
	}

	relation.first.clear();
	relation.second.clear();

	vector<string> token = WhereLexAnalyze(str);
	if (!CheckWhereSyntax(token))
		return false;
	
	token = WhereSynAnalyze(token);
	
	/*
	cout << "Token:" << endl;
	for (int i = 0; i < token.size(); i++)
		cout << token[i] << endl;
	*/
	vector<Relation*> relVec;
	if (token.size() == 0)
	{
		Relation* tempRel = new Relation();
		relVec.push_back(tempRel);
	}
	for (int i = 0; i < token.size(); i++)
	{
		vector<WordInfo> wordInfoList;
		if (IsOperator(token[i]))
		{
			Relation* leftRel = relVec.back();
			relVec.pop_back();
			Relation* rightRel = relVec.back();
			relVec.pop_back();
			Relation* tempRel = new Relation();
			if (token[i] == "and")
				*tempRel = qp->AndOp(*leftRel, *rightRel);
			else if (token[i] == "or")
				*tempRel = qp->OrOp(*leftRel, *rightRel);
			delete leftRel;
			delete rightRel;
			relVec.push_back(tempRel); 
		}
		else
		{
			if (!WhereWordAnalyze(token[i], tableAliasList, wordInfoList))
				return false;
			Relation* tempRel = new Relation();
			int leftIndex = -1, rightIndex = -1;
			if (wordInfoList[0].type == 1)
				for (int j = 0; j < tableCnt; j++)
					if (wordInfoList[0].table == tableAliasList[j].second)
					{
						leftIndex = j;
						break;
					}
			if (wordInfoList[2].type == 1)
				for (int j = 0; j < tableCnt; j++)
					if (wordInfoList[2].table == tableAliasList[j].second)
					{
						rightIndex = j;
						break;
					}

			if (wordInfoList[2].type != 1)
			{
				if (!qp->ConditionFilter(tableAliasList[leftIndex].first, tableInfoList[leftIndex], 
							wordInfoList[0].field, wordInfoList[2].constStr, wordInfoList[1].constStr, *tempRel))
					return false;
			}
			else
			{
				if (!qp->LinkFilter(tableAliasList[leftIndex].first, tableAliasList[rightIndex].first, tableInfoList[leftIndex], tableInfoList[rightIndex],
							wordInfoList[0].field, wordInfoList[2].field, wordInfoList[1].constStr, *tempRel))
					return false;
			}
			
			relVec.push_back(tempRel);
		}
	}
	
	Relation* resultRel = relVec.back();
	//qp->PrintRelation(*resultRel);
	vector<string> lackAttr;
	for (int i = 0; i < tableCnt; i++)
	{
		bool found = false;
		for (int j = 0; j < resultRel->first.size(); j++)
		{
			if (resultRel->first[j] == tableAliasList[i].first)
			{
				found = true;
				break;
			}
		}
		if (!found)
			lackAttr.push_back(tableAliasList[i].first);
	}
	/*
	cout << "LackAttr" << endl;
	for (int i = 0; i < lackAttr.size(); i++)
		cout << lackAttr[i] << endl;
	*/
	if (lackAttr.size() > 0)
		*resultRel = qp->ExtendRelation(*resultRel, lackAttr);

	relation = *resultRel;
	delete resultRel;
	return true;
}

bool SynAnalyze::ShadowAnalyze(const string& cstr, const vector<TableAlias>& tableAliasList, vector<Shadow>& shadowList)
{

	shadowList.clear();
	string str(cstr);
	if (StripStr(str) == "*")
		return true;

	vector<string> words = SplitStrNonEmpty(str, ',');
	if (words.size() == 0)
	{
		errh->ErrorHandle("SELECT", "syntax", "empty shadow content.");
		return false;
	}
 
	if (tableAliasList.size() == 1)
	{
		Shadow shadow;
		shadow.first = tableAliasList[0].second;
		for (int i = 0; i < words.size(); i++)
		{
			vector<string> tempVec = SplitStr(words[i], '.');
			if (tempVec.size() > 1)
			{
				if (!TableFieldAnalyze(StripStr(words[i]), tableAliasList, tempVec, "SELECT"))
					return false;
				shadow.second.push_back(tempVec[1]);
			}
			else
			{
				shadow.second.push_back(StripStr(words[i]));
			}
		}
		shadowList.push_back(shadow);
	}
	else
	{
		for (int i = 0; i < words.size(); i++)
		{
			vector<string> tempVec;
			if (!TableFieldAnalyze(StripStr(words[i]), tableAliasList, tempVec, "SELECT"))
				return false;
			bool found = false;
			for (int j = 0; j < shadowList.size(); j++)
			{
				if (shadowList[j].first == tempVec[0])
				{
					shadowList[j].second.push_back(tempVec[1]);
					found = true;
					break;
				}
			}
			if (!found)
			{
				Shadow shadow;
				shadow.first = tempVec[0];
				shadow.second.push_back(tempVec[1]);
				shadowList.push_back(shadow);
			}
		}
	}

	return true;
}

bool SynAnalyze::InsertAnalyze(const string& cstr)
{

	if (StripStr(cstr).length() == 0)
	{
		errh->ErrorHandle("INSERT", "syntax", "empty command content.");
		return false;
	}

	string str(cstr+" ");
	bool inWord = false;
	int start = 0, leftEnd = -1, rightStart = -1;
	for (int i = 0; i < str.length(); i++)
	{
		if (IsBlank(str[i]))
		{
			if (inWord)
			{
				inWord = false;
				string tempWord = ToLowerCase(str.substr(start, i-start));
				if (tempWord == "values")
				{
					leftEnd = start;
					rightStart = i;
					break;
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}
		}
	}

	if (leftEnd == -1 && rightStart == -1)
	{
		errh->ErrorHandle("INSERT", "syntax", "keyword \"VALUES\" not found.");
		return false;
	}

	string tableStr = str.substr(0, leftEnd);
	string valueStr = str.substr(rightStart, str.length()-rightStart);
	vector<TableAlias> tableAliasList;
	if (!OneTableAnalyze(tableStr, tableAliasList, "INSERT"))
		return false;
	string tableName = tableAliasList[0].first;

	vector<char>leftChar, rightChar;
	leftChar.push_back('(');
	rightChar.push_back(')');
	vector<string> valList = SplitStrWithLimit(valueStr, ',', leftChar, rightChar);
	valList = RemoveEmptyVector(valList);
	int valCnt = valList.size();
	leftChar.clear();
	rightChar.clear();
	for (int i = 0; i < valCnt; i++)
	{
		string valStr = StripStr(valList[i]);
		if (valStr.size() == 0)
		{
			errh->ErrorHandle("INSERT", "syntax", "empty value entry.");
			return false;
		}
		if (valStr[0] != '(' || valStr[valStr.length()-1] != ')')
		{
			errh->ErrorHandle("INSERT", "syntax", "parentheses should surround value:"+valStr);
			return false;
		}
		valStr = valStr.substr(1, valStr.length()-2);
		//cout << valStr << endl;
		vector<string> fieldValList = SplitStrWithLimit(valStr, ',', leftChar, rightChar);
		fieldValList = StripVector(fieldValList);
		vector<bool> isNull;
		vector<string> fieldList;
		for (int j = 0; j < fieldValList.size(); j++)
		{
			//cout << fieldValList[j] << endl;
			if (fieldValList[j].length() == 0)
			{
				errh->ErrorHandle("INSERT", "syntax", "empty field value exists:"+valStr);
				return false;
			}
			if (ToLowerCase(fieldValList[j]) == "null")
			{
				isNull.push_back(true);
				fieldValList[j] = "";
			}
			else
			{
				isNull.push_back(false);
				if (IsStr(fieldValList[j]))
					fieldValList[j] = WordToStr(fieldValList[j]);
				else if (!IsInt(fieldValList[j]))
				{
					errh->ErrorHandle("INSERT", "syntax", "invalid field value:"+fieldValList[j]);
					return false;	
				}
			}
		}
		/*
		cout << "No." << i << ":" << endl;
		cout << "TableName:" << tableName << endl;
		for (int j = 0; j < fieldValList.size(); j++)
			cout << fieldValList[j] << " ";
		cout << endl;
		for (int j = 0; j < isNull.size(); j++)
		{
			if (isNull[j])
				cout << "null" << " ";
			else
				cout << "not" << " ";
		}
		cout << endl;
		*/
		if(!(qp->InsertRecord(tableName, fieldList, fieldValList, isNull)))
			return false;

	}
	return true;
}

bool SynAnalyze::DeleteAnalyze(const string& cstr)
{

	if (StripStr(cstr).length() == 0)
	{
		errh->ErrorHandle("DELETE", "syntax", "empty command content.");
		return false;
	}
		 
	string str(cstr);
	string tempStr = GetWord(str);
	if (ToLowerCase(tempStr) != "from")
	{
		errh->ErrorHandle("DELETE", "syntax", "keyword\"from\" not found or not in right place.");
		return false;
	}
	vector<string> strVec;
	string tableStr = "", whereStr = "";
	if (!FindWordAndSplit(str, "where", strVec))
	{
		tableStr = StripStr(str);
	}
	else
	{
		tableStr = StripStr(strVec[0]);
		whereStr = StripStr(strVec[1]);
	}
	vector<TableAlias> tableAliasList;
	if (!OneTableAnalyze(tableStr, tableAliasList, "DELETE"))
		return false;
	string tableName = tableAliasList[0].first;
	//cout << "tableName:" << tableName << endl;
	//cout << "whereStr:" << whereStr << endl;
	
	if (whereStr == "")
	{
		if (!(qp->DeleteAllRecord(tableName)))
			return false;
	}
	else
	{
		Relation relation;
		if (!WhereAnalyze(whereStr, tableAliasList, relation))
			return false;
		if (!(qp->DeleteRecord(tableName, relation)))
			return false;
	}
	
	return true;
}

bool SynAnalyze::UpdateAnalyze(const string& cstr)
{

	if (StripStr(cstr).length() == 0)
	{
		errh->ErrorHandle("UPDATE", "syntax", "empty command content.");
		return false;
	}
	string str(cstr);

	vector<string> strVec;
	if (!FindWordAndSplit(str, "set", strVec))
	{
		errh->ErrorHandle("SELECT", "syntax", "keyword\"set\" not found.");
		return false;
	}
	string tableStr = StripStr(strVec[0]);
	string remainStr = StripStr(strVec[1]);
	string setStr, whereStr;
	if (!FindWordAndSplitWithLimit(remainStr, "where", strVec))
	{
		whereStr = "";
		setStr = remainStr;
	}
	else
	{
		setStr = StripStr(strVec[0]);
		whereStr = StripStr(strVec[1]);
	}

	vector<TableAlias> tableAliasList;
	if (!OneTableAnalyze(tableStr, tableAliasList, "UPDATE"))
		return false;
	string tableName = tableAliasList[0].first;
	
	vector<char> leftChar, rightChar;
	vector<string> tempVec = SplitStrWithLimit(setStr, '=', leftChar, rightChar);
	if (tempVec.size() <= 1)
	{
		errh->ErrorHandle("SET", "syntax", "operator \"=\" not found");
		return false;
	}
	else if (tempVec.size() >= 3)
	{
		errh->ErrorHandle("SET", "syntax", "too much operator \"=\"");
		return false;
	}

	string fieldName = StripStr(tempVec[0]);
	string target = StripStr(tempVec[1]);
	if (target.length() == 0)
	{
		errh->ErrorHandle("SET", "sematic", "empty field value:"+setStr);
		return false;
	}
	bool isNull = false;
	if (ToLowerCase(target) == "null")
	{
		isNull = true;
		target = "";
	}
	else if (IsStr(target))
	{
		target = WordToStr(target);
	}
	else if (!IsInt(target))
	{
		errh->ErrorHandle("SET", "sematic", "invalid field value:"+target);
		return false;
	}

/*
	cout << "TableName:" << tableName << endl;
	cout << "FieldName:" << fieldName << endl;
	cout << "target:" << target << endl;
	if (isNull)
		cout << "null" << endl;
	else
		cout << "not null" << endl;
	cout << "whereStr:" << whereStr << endl;
*/
	Relation relation;
	if (!WhereAnalyze(whereStr, tableAliasList, relation))
		return false;

	vector<LP> posList;
	for (int i = 0 ; i < relation.second.size(); i++)
		posList.push_back(relation.second[i][0]);

	if (!(qp->UpdateRecord(tableName, fieldName, target, isNull, posList)))
		return false;

	return true;
}

bool SynAnalyze::SelectAnalyze(const string& cstr, ViewTable& viewTable)
{

	if (StripStr(cstr).length() == 0)
	{
		errh->ErrorHandle("SELECT", "syntax", "empty command content.");
		return false;
	}
	string str(cstr);

	vector<string> strVec;
	if (!FindWordAndSplit(str, "from", strVec))
	{
		errh->ErrorHandle("SELECT", "syntax", "keyword\"from\" not found.");
		return false;
	}
	
	string shadowStr = StripStr(strVec[0]);
	string remainStr = StripStr(strVec[1]);
	string fromStr, whereStr;
	if (!FindWordAndSplit(remainStr, "where", strVec))
	{
		fromStr = remainStr;
		whereStr = "";
	}
	else
	{
		fromStr = StripStr(strVec[0]);
		whereStr = StripStr(strVec[1]);
	}
	/*
	cout << "shadowStr:" << shadowStr << endl;
	cout << "fromStr:" << fromStr << endl;
	cout << "whereStr:" << whereStr << endl;
	*/
	vector<TableAlias> tableAliasList;
	vector<Shadow> shadowList;
	Relation relation;
	if (!FromAnalyze(fromStr, tableAliasList))
		return false;
	if (!ShadowAnalyze(shadowStr, tableAliasList, shadowList))
		return false;
	//cout << "WhereStart:" << endl;
	if (!WhereAnalyze(whereStr, tableAliasList, relation))
		return false;
	/*
	qp->PrintRelation(relation);
	qp->PrintShadowList(shadowList);
	*/
	if (!(qp->SelectRecord(shadowList, tableAliasList, relation, viewTable)))
		return false;

	return true;
}

bool SynAnalyze::CmdAnalyze(const string& cstr, ViewTable& viewTable, bool& hasView, string& sysStr, bool& hasStr)
{
	hasView = false;
	hasStr = false;
	if (cstr[cstr.length()-1] != ';')
	{
		errh->ErrorHandle("COMMAND", "syntax", "command should end with \";\"");
		return false;
	}
	string str(cstr.substr(0, cstr.length()-1)+" ");
	if (StripStr(str).length() == 0)
	{
		errh->ErrorHandle("COMMAND", "syntax", "emtpy cmd.");
		return false;
	}

	//cout << cstr << endl;
	int sysFlag;
	sysStr = SysAnalyze(cstr, sysFlag);
	if (sysFlag == -1)
		return false;
	else if (sysFlag == 1)
	{
		hasStr = true;		
		return true;
	}

	bool inWord = false, hasInsert = false;
	int start = 0;
	for (int i = 0; i < str.length(); i++)
	{
		if (IsBlank(str[i]))
		{
			if (inWord)
			{
				inWord = false;
				string tempWord = ToLowerCase(str.substr(start, i-start));
				if (tempWord == "insert")
				{
					hasInsert = true;
				}
				else
				{
					string remainStr = str.substr(i, str.length()-i);
					if (tempWord == "into" && hasInsert)
						return InsertAnalyze(remainStr);
					else if (tempWord == "delete")
						return DeleteAnalyze(remainStr);
					else if (tempWord == "update")
						return UpdateAnalyze(remainStr);
					else if (tempWord == "select")
					{
						bool success = SelectAnalyze(remainStr, viewTable);
						if (success)
							hasView = true;
						return success;
					}
					else
						break;
				}
			}
		}
		else 
		{
			if (!inWord)
			{
				inWord = true;
				start = i;
			}
		}
	}
	errh->ErrorHandle("COMMAND", "syntax", "command not found.");

	return false;
}
