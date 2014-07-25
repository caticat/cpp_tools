#pragma once
#include <string>

#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif // DLL_EXPORT

class DLL_API PJson
{
public:
	static std::string GetValue(const char* cjson,const char* ckey); // 获取json串的某个值
};