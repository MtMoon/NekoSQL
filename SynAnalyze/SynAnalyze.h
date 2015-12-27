#ifndef SYN_ANALYZE_H_
#define SYN_ANALYZE_H_

#include "../ErrorHandler/ErrorHandler.h"
#include "../sysmanager/SysManager.h"
#include "../QueryProcessor/QueryProcessor.h"
#include "../indexmanager/IndexManager.h"
#include <string>
#include <vector>
#include <utility>

struct WordInfo
{
	string table;
	string field;
	string constStr;
	int type;
	WordInfo(const std::string& table_ = "", const std::string& field_= "", const std::string& constStr_ = "", int type_ = -1) :
	 table(table_), field(field_), constStr(constStr_), type(type_) {} 
};


class SynAnalyze
{
public:
	SynAnalyze(SysManager* smm, QueryProcessor* qpp, ErrorHandler* errorh,  IndexManager* imm);
	~SynAnalyze();
//private:
	SysManager* sm;
	IndexManager* im;
	QueryProcessor* qp;
	ErrorHandler* errh;

	//for SysManager
	string SysAnalyze(const std::string& cstr, int & flag);
	int createTBAnalyze(const std::string& str);
	//end of SysManager

	bool OneTableAnalyze(const std::string& cstr, std::vector<TableAlias>& tableAliasList, const std::string& opType);
	bool FromAnalyze(const std::string& str, std::vector<TableAlias>& tableAliasList);
	std::vector<std::string> WhereLexAnalyze(const std::string& cstr);
	bool CheckWhereSyntax(const std::vector<std::string>& tokens);
	std::vector<std::string> WhereSynAnalyze(const std::vector<std::string>& words);
	bool TableFieldAnalyze(const std::string& cstr, const std::vector<TableAlias>& tableAliasList, std::vector<std::string>& result, const std::string& opType);
	bool WhereWordAnalyze(const std::string& cstr, const std::vector<TableAlias>& tableAliasList, std::vector<WordInfo>& wordInfoList);
	bool WhereAnalyze(const std::string& str, const std::vector<TableAlias>& tableAliasList, Relation& relation);
	bool ShadowAnalyze(const std::string& cstr, const std::vector<TableAlias>& tableAliasList, std::vector<Shadow>& shadowList);
	bool InsertAnalyze(const std::string& cstr);
	bool DeleteAnalyze(const std::string& cstr);
	bool UpdateAnalyze(const std::string& cstr);
	bool SelectAnalyze(const std::string& cstr, ViewTable& viewTable);
	bool CmdAnalyze(const std::string& cstr, ViewTable& viewTable, bool& hasView, std::string& sysStr, bool& hasStr);
	//std::vector<std::string> SplitStrWithLimit(const std::string& str, char c, const std::vector<char>& leftChar, const std::vector<char>& rightChar);
};

#endif
