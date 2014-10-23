#include "config.h"

void Configure::Init( const std::string& file )
{
	XmlFile = file;

	m_sCopyright = GetCopyright();
	m_nMajorVersion = GetMajorVersion();
	InitModules();
	m_nMapCompanyId = GetMapCompanyId();
	m_nBuildYear = GetBuildYear();
	m_nBuildMonth = GetBuildMonth();
	m_nBuildDay = GetBuildDay();
	m_nBuildNumber = GetBuildNumber();

	m_nEIdVersion = GetEIdVersion();
	m_nNodeIdVersion = GetNodeIdVersion();

	m_nOMMajorVersion = GetOMMajorVersion();
	m_nOMMinorVersion = GetOMMinorVersion();
}

void Configure::GetVersion( Version& version, int nModuleId )
{
	std::string sCopyright = GetCopyright();
	__Version DataVersion = __GetVersion(nModuleId);
	__Version EIdVersion = __GetEIdVersion();
	__Version NodeIdVersion = __GetNodeIdVersion();

	strncpy(version.m_csCopyright, sCopyright.c_str(), COPYRIGHT_SIZE);
	version.m_verData = DataVersion;
	version.m_verEId = EIdVersion;
	version.m_verNodeId = NodeIdVersion;
}

int Configure::GetModuleCount()
{
	return m_modules.size();
}

void Configure::GetModuleFiles( std::vector<std::string>& files, int nModuleId )
{
	files.clear();
	files.insert(files.end(), m_modules[nModuleId - 1].m_fileList.begin(), m_modules[nModuleId - 1].m_fileList.end());
}

OriginalMapVersion Configure::GetOMVersion()
{
	OriginalMapVersion OMVersion;
	OMVersion.m_nMapCompanyId = m_nMapCompanyId;
	OMVersion.m_nMajorVersion = m_nOMMajorVersion;
	OMVersion.m_nMinorVersion = m_nOMMajorVersion;

	return OMVersion;
}

__Version Configure::__GetVersion( int nModuleId )
{
	__Version version;

	version.m_nModuleId = m_modules[nModuleId - 1].m_nModuleId;
	version.m_nMapCompanyId = m_nMapCompanyId;
	version.m_nMajorVersion = m_nMajorVersion;
	version.m_nMinorVersion = m_modules[nModuleId - 1].m_nMinorVersion;
	version.m_nBuildYear = m_nBuildYear;
	version.m_nBuildMonth = m_nBuildMonth;
	version.m_nBuildDay = m_nBuildDay;
	version.m_nBuildNumber = m_nBuildNumber;

	return version;
}

__Version Configure::__GetEIdVersion()
{
	__Version version;

	version.m_nModuleId = EID_MODULE;
	version.m_nMapCompanyId = m_nMapCompanyId;
	version.m_nMajorVersion = m_nMajorVersion;
	version.m_nMinorVersion = m_nEIdVersion;
	version.m_nBuildYear = m_nBuildYear;
	version.m_nBuildMonth = m_nBuildMonth;
	version.m_nBuildDay = m_nBuildDay;
	version.m_nBuildNumber = m_nBuildNumber;

	return version;
}

__Version Configure::__GetNodeIdVersion()
{
	__Version version;

	version.m_nModuleId = NODEID_MODULE;
	version.m_nMapCompanyId = m_nMapCompanyId;
	version.m_nMajorVersion = m_nMajorVersion;
	version.m_nMinorVersion = m_nNodeIdVersion;
	version.m_nBuildYear = m_nBuildYear;
	version.m_nBuildMonth = m_nBuildMonth;
	version.m_nBuildDay = m_nBuildDay;
	version.m_nBuildNumber = m_nBuildNumber;

	return version;
}

void Configure::InitModules()
{
	rapidxml::file<> ConfigFile(XmlFile.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(ConfigFile.data());
	rapidxml::xml_node<>* root = doc.first_node();

	rapidxml::xml_node<>* moduleNode = find(root, "Modules")->first_node();
	int nModuleId = 0, nMinorVersion = 0;
	rapidxml::xml_node<>* fileNode;
	while (moduleNode) {
		nModuleId = atoi(find(moduleNode, "ModuleId")->value());
		nMinorVersion = atoi(find(moduleNode, "MinorVersion")->value());
		if (nModuleId > m_modules.size()) {
			m_modules.resize(nModuleId);
			m_modules[nModuleId - 1].m_nModuleId = nModuleId;
			m_modules[nModuleId - 1].m_nMinorVersion = nMinorVersion;
		}

		fileNode = find(moduleNode, "Files")->first_node();
		while (fileNode) {
			m_modules[nModuleId - 1].m_fileList.push_back(fileNode->name());
			fileNode = fileNode->next_sibling();
		}
		moduleNode = moduleNode->next_sibling();
	}
}

std::string Configure::GetCopyright()
{
	rapidxml::file<> ConfigFile(XmlFile.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(ConfigFile.data());
	rapidxml::xml_node<>* root = doc.first_node();

	rapidxml::xml_node<>* node = find(root, "CopyrightString");

	if (node) {
		return std::string(node->value());
	} else {
		return std::string("");
	}
}

int Configure::GetMajorVersion()
{
	int nMajorVersion;
	std::string str = value("MajorVersion");
	if (str.empty()) {
		nMajorVersion = 0;
	} else {
		nMajorVersion = atoi(str.c_str());
	}

	return nMajorVersion;
}

int Configure::GetMapCompanyId()
{
	std::string s = value("MapCompanyId");
	return s.empty()? 0: atoi(s.c_str());
}

int Configure::GetBuildYear()
{
	std::string str = value("BuildYear");
	if (str.empty()) {
		SYSTEMTIME SysTime; 
		GetLocalTime(&SysTime); 
		return SysTime.wYear % 100;
	} else {
		return atoi(str.c_str());
	}
}

int Configure::GetBuildMonth()
{
	std::string str = value("BuildMonth");
	if (str.empty()) {
		SYSTEMTIME SysTime; 
		GetLocalTime(&SysTime); 
		return SysTime.wMonth;
	} else {
		return atoi(str.c_str());
	}
}

int Configure::GetBuildDay()
{
	std::string s = value("BuildDay");
	if (s.empty()) {
		SYSTEMTIME SysTime; 
		GetLocalTime(&SysTime); 
		return SysTime.wDay;
	} else {
		return atoi(s.c_str());
	}
}

int Configure::GetBuildNumber()
{
	std::string s = value("BuildNumber");
	return s.empty()? 0: atoi(s.c_str());
}

int Configure::GetEIdVersion()
{
	std::string s = value("EIdVersion");
	return s.empty()? 0: atoi(s.c_str());
}

int Configure::GetNodeIdVersion()
{
	std::string s = value("NodeIdVersion");
	return s.empty()? 0: atoi(s.c_str());
}

int Configure::GetOMMajorVersion()
{
	std::string s = value("OMMajorVersion");
	return s.empty()? 0: atoi(s.c_str());
}

int Configure::GetOMMinorVersion()
{
	std::string s = value("OMMinorVersion");
	return s.empty()? 0: atoi(s.c_str());
}

rapidxml::xml_node<>* Configure::last_node(rapidxml::xml_node<>* root)
{
	rapidxml::xml_node<>* LastNode;

	if (root->first_node()) {
		LastNode = root->last_node();
	} else {
		exit(EXIT_FAILURE);
	}

	while (1) {
		if (LastNode->name()) {
			if (LastNode->first_node()) {
				char* ptr = LastNode->first_node()->name();
				if (strcmp(ptr, "")) {
					LastNode = LastNode->last_node();
				} else {
					break;
				}
			} else {
				break;
			}
		} else {
			LastNode = LastNode->parent();
			break;
		}
	}

	return LastNode;
}

rapidxml::xml_node<>* Configure::find( rapidxml::xml_node<>* root, const std::string& filter )
{
	rapidxml::xml_node<>* node = root->first_node();
	rapidxml::xml_node<>* terminal = last_node(root);

	// search for 'filter'
	while (node != terminal) {
		// if the node is 'filter'?
		if (!strcmp(node->name(), filter.c_str())) {			// if it is
			return node;
		} else {			// if not
			// if this node has a sub-node
			if (node->first_node()) {			// if it has
				char* ptr = node->first_node()->name();
				// check this sub-node is a real node or just a value?
				if (strcmp(ptr, "")) {			// if it's a real sub-node
					node = node->first_node();
				} else {			// it's just value
					// if 
					if (node->next_sibling()) {
						node = node->next_sibling();
					} else {
						do {
							node = node->parent();

							if (node->next_sibling()) {
								node = node->next_sibling();
								break;
							}
						} while (1);
					}
				}
			} else {
				if (node->next_sibling()) {
					node = node->next_sibling();
				} else {
					do {
						node = node->parent();

						if (node->next_sibling()) {
							node = node->next_sibling();
							break;
						}
					} while (1);
				}
			}
		}
	}
}

std::string Configure::value( const std::string& tag )
{
	rapidxml::file<> ConfigFile(XmlFile.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(ConfigFile.data());
	rapidxml::xml_node<>* root = doc.first_node();

	rapidxml::xml_node<>* node = find(root, tag);

	if (node) {
		return node->value();
	} else {
		return std::string("");
	}
}