#ifndef ERROR_HANDLER_H_
#define ERROR_HANDLER_H_

#include <string>

class ErrorHandler
{
public:
	ErrorHandler();
	~ErrorHandler();
	bool ErrorHandle(const std::string& src, const std::string& type, const std::string& msg); 
	bool WarningHandle(const std::string& src, const std::string& type, const std::string& msg);
private:
};

#endif
