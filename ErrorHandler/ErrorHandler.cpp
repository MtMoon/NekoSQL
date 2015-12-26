#include "ErrorHandler.h"
#include <iostream>

using namespace std;

ErrorHandler::ErrorHandler() {}

ErrorHandler::~ErrorHandler() {}

bool ErrorHandler::ErrorHandle(const string& src, const string& type, const string& msg)
{
	cout << "Error:" << src << ":" << type << " fault" << endl;
	cout << msg << endl;
	return true;
}

bool ErrorHandler::WarningHandle(const string& src, const string& type, const string& msg)
{
	cout << "Warning:" << src << ":" << type << " fault" << endl;
	cout << msg << endl;
	return true;
}

