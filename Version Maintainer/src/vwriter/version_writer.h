#pragma once

#include "version_types.h"
#include "config.h"
#include <windows.h>
#include <string>
#include <cstring>

#define STD_OUT 0
#define FILE_OUT 1

#define SPACE_STRING "\r                                                                      "

extern std::string path;

class VersionWriter
{
public:
	VersionWriter() { ;}
	~VersionWriter() { ;}

	bool Init(const char* root);
	int GetModuleCount();
	bool Write(int nModuleId);

private:
	bool GetModuleFiles(int nModuleId);
	std::string CheckModuleFiles();
	bool HasVersion(const std::string& file);
	bool CreateOMVersionFile(const char* path);

private:
	std::string m_root;
	Configure m_config;
	std::vector<std::string> m_moduleFiles;
};