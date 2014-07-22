#pragma once
#include <iostream>
#include <WinSock.h>
#include <string>
#include <vector>
#include "mysql.h"

#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif