#include "pjson.h"

using std::string;

// 获取json串的某个值
string PJson::GetValue(const char* cjson,const char* ckey)
{
	if ((cjson == NULL) || (ckey == NULL))
		return "";

	string json = cjson;
	string key = ckey;

	int pos = string.npos;
	int type = 0;
	// 查找开始位置
	if ((type = 1, (pos = json.find("{"+key+":")) == string.npos) &&
		(type = 2, (pos = json.find(","+key+":")) == string.npos) &&
		(type = 3, (pos = json.find("\""+key+"\":")) == string.npos) &&
		(type = 4, (pos = json.find("\'"+key+"\':")) == string.npos)
		)
	{
		return "";
	}

	int plusPos = 0;
	if (type < 3)
		plusPos = 2;
	else
		plusPos = 3;
	int beginPos = pos+key.length()+plusPos; // 开始的字符串位置

	int endPos = string.npos;
	if (((endPos = json.find(",",beginPos)) == string.npos) &&
		((endPos = json.find("}",beginPos)) == string.npos)
		)
	{
		return "";
	}

	return json.substr(beginPos,endPos-beginPos);
}
