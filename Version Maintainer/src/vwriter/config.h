#pragma once

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "version_types.h"
#include <windows.h>

#define EID_MODULE 7
#define NODEID_MODULE 8

class Configure
{
public:
	Configure() { ;}
	Configure(const std::string& file) { XmlFile = file;}
	~Configure() { ;}

	void Init(const std::string& file);
	void GetVersion(Version& version, int nModuleId);
	int GetModuleCount();
	void GetModuleFiles(std::vector<std::string>& files, int nModuleId);
	OriginalMapVersion GetOMVersion();

private:
	__Version __GetVersion(int nModuleId);
	__Version __GetEIdVersion();
	__Version __GetNodeIdVersion();

	std::string GetCopyright();
	int GetMajorVersion();
	void InitModules();
	int GetMapCompanyId();
	int GetBuildYear();
	int GetBuildMonth();
	int GetBuildDay();
	int GetBuildNumber();

	int GetEIdVersion();
	int GetNodeIdVersion();

	int GetOMMajorVersion();
	int GetOMMinorVersion();

	rapidxml::xml_node<>* last_node(rapidxml::xml_node<>* root);
	rapidxml::xml_node<>* find(rapidxml::xml_node<>* root, const std::string& filter);
	std::string value(const std::string& tag);

private:
	std::string XmlFile;
	std::string m_sCopyright;
	int m_nMajorVersion;
	std::vector<Module> m_modules;
	int m_nMapCompanyId;
	int m_nBuildYear;
	int m_nBuildMonth;
	int m_nBuildDay;
	int m_nBuildNumber;

	int m_nEIdVersion;
	int m_nNodeIdVersion;

	int m_nOMMajorVersion;
	int m_nOMMinorVersion;
};