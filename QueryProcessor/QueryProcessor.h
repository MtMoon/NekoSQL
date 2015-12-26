#ifndef QUERY_PROCESSOR_H_
#define QUERY_PROCESSOR_H_

#include "../datamanager/DataManager.h"
#include "../sysmanager/SysManager.h"
#include "../ErrorHandler/ErrorHandler.h"
#include <string>
#include <vector>
#include <utility>

typedef std::vector<LP> Tuple;
typedef std::vector<Tuple> Collect;
typedef std::vector<std::string> AttrInfo; //misnomer: each string in vector is a tablename.
typedef std::pair<AttrInfo, Collect> Relation;

typedef std::pair<std::string, std::string> TableAlias;

typedef std::pair<std::string, std::vector<std::string> > Shadow;

typedef std::vector<std::string> ViewTitle;
typedef std::vector<std::string> ViewEntry;
typedef std::pair<ViewTitle, std::vector<ViewEntry> > ViewTable;

class QueryProcessor
{
public:
	QueryProcessor(DataManager* dm, SysManager* sm, ErrorHandler* errh);
	~QueryProcessor();
	bool SelectRecord(const std::vector<Shadow>& cShadowList, const std::vector<TableAlias>& tableAliasList, const Relation& relation, ViewTable& viewTable);
	bool DeleteAllRecord(const std::string& tableName);
	bool DeleteRecord(const std::string& tableName, const Relation& relation);
	bool UpdateRecord(const std::string& tableName, const std::string& fieldName, const std::string& target, bool isNull, const std::vector<LP>& pos);
	bool InsertRecord(const std::string& tableName, const std::vector<std::string>& fieldNames, const std::vector<std::string>& values, const std::vector<bool>& isNulls);
	bool ConditionFilter(const std::string& tableName, const std::vector<FieldInfo>& fieldInfoList, const std::string& fieldName, const std::string& target, const std::string& op, Relation& relation);
	bool LinkFilter(const std::string& leftTable, const std::string& rightTable, const std::vector<FieldInfo>& leftInfo, const std::vector<FieldInfo>& rightInfo, const std::string& leftField, const std::string& rightField, const std::string& op, Relation& relation);
	Relation AndOp(const Relation& leftRel, const Relation& rightRel);
	Relation OrOp(const Relation& leftRel, const Relation& rightRel);

//private:
	DataManager* dataManager;
	SysManager* sysManager;
	ErrorHandler* errHandler;

	bool intOverflowFlag;
	bool stringOverflowFlag;

	bool IsInt(const std::string& value);
	bool MakeData(const FieldInfo& fieldInfo, const std::string& str, const std::string& opType, Data& data);
	std::vector<LP> RemainLP(const std::vector<LP>& father, const std::vector<LP>& son);
	bool IncCounter(std::vector<int>& counter, const std::vector<int>& limit);
	Relation Linkage(const Relation& leftRel, const Relation& rightRel);
	Relation MultiLink(std::vector<Relation*> relations);
	Relation ExtendRelation(const Relation& relation, const std::vector<std::string>& lackAttr);

	void PrintRelation(const Relation& relation);
	void PrintViewTable(const ViewTable& viewTable);
	void PrintShadowList(const std::vector<Shadow>& shadowList);
};
#endif
