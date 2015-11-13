/*
 * SysManager.h
 *
 *  Created on: 2015-11-12
 *      Author: yxy
 */

#ifndef SYSMANAGER_H_
#define SYSMANAGER_H_


#include "../datamanager/DataManager.h"
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <assert.h>

/**
 * 数据管理模块
 * 实现基本的DDL语句功能
 * 向下调用记录管理模块和索引模块
 * 向上供命令解析模块调用
 */
class SysManager {
public:
	//注意，由于记录管理模块和索引模块会被查询解析模块与系统管理模块都调用
	//所以在更上层生成实例并传入指针，供两个模块同时使用
	SysManager(DataManager* dm);
	~SysManager();

	//数据库操作

	/**
	 * para: database name
	 * return: 1 create successful; 0 database exists;-1 other error
	 */
	int createDatabase(string dbName);

	/**
	 * para: database name
	 * return: 1 drop successful; 0 database not exists;-1 other error
	 */
	int dropDatabase(string dbName);

	/**
	 * para: database name
	 * return: 1 change successful; 0 database not exists;-1 other error
	 */
	int useDatabase(string dbName);


	/**
	 * 列出所有Database的名字
	 * return: 当无dataBase时，返回为vector 内数据为空
	 */
	vector<string> showDatabases();

	//表操作
	/**
	  * 列出所有表的名字
	  * return: 当无表时，返回vector 内数据为空
	  * flag:1 成功 0 当前database中无表 -1 尚未选中任何database -2 其他错误
	  */
	vector<string> showTables(int& flag);

	/**
	  * para: table name
	  * return: 1 drop successful; 0 table not exists;-1 no selected database
	  */
	int dropTable(string tbName);

	/**
	 * 显示一个表的字段信息
	 * 当表不存在时，返回的vector 内数据为空
	 * flag:1 成功 0 表不存在 -1 尚未选中任何database
	 */
	vector<FieldInfo> descTable(string tableName, int& flag);

	/*
	 * 创建表
	 * 返回1为创建成功，0为表已存在，-1为尚未选中任何database
	 */
	int createTable(string tableName, vector<FieldInfo> tb);



private:
	DataManager* dataManager;

	/**
	  * 获取所有database名，存于dataBaseDic
      */
	void readDatabases();
	int is_dir_exist(const char* filepath);
	int is_file_exist(const char* filepath);

	map<string,int> dataBaseDic; //存储所有的database信息


};





#endif /* SYSMANAGER_H_ */
