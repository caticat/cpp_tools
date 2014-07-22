#pragma once

struct DLL_API DBConfig
{
	DBConfig(std::string host, int port, std::string database, std::string name, std::string password, std::string charSet)
	{
		m_host = host;
		m_port = port;
		m_database = database;
		m_name = name;
		m_password = password;
		m_charset = charSet;
	}
	std::string m_host;
	int m_port;
	std::string m_database;
	std::string m_name;
	std::string m_password;
	std::string m_charset;
};