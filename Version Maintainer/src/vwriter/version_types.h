#pragma once

#include <cstring>
#include <ostream>
#include <string>
#include <vector>

#define COPYRIGHT_SIZE 128
#define COPYRIGHT_STRING "Mesada.Technology.Company.Ltd...CopyRight"

#define VERSION_SIZE (sizeof(Version))

typedef unsigned __int8 UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;

struct __Version
{
	UINT64 m_nModuleId: 8;
	UINT64 m_nReserved1: 4;
	UINT64 m_nMapCompanyId: 7;
	UINT64 m_nMajorVersion: 7;
	UINT64 m_nMinorVersion: 8;
	UINT64 m_nReserved2: 7;
	UINT64 m_nBuildYear: 7;
	UINT64 m_nBuildMonth: 4;
	UINT64 m_nBuildDay: 5;
	UINT64 m_nBuildNumber: 7;

	__Version()
	{
		m_nModuleId = 0;
		m_nReserved1 = 0;
		m_nMapCompanyId = 0;
		m_nMajorVersion = 0;
		m_nMinorVersion = 0;
		m_nReserved2 = 0;
		m_nBuildYear = 0;
		m_nBuildMonth = 0;
		m_nBuildDay = 0;
		m_nBuildNumber = 0;
	}

	__Version(int nModuleId, int nMapCompanyId, int nMajorVersion, int nMinorVersion)
	{
		m_nModuleId = nModuleId;
		m_nReserved1 = 0;
		m_nMapCompanyId = nMapCompanyId;
		m_nMajorVersion = nMajorVersion;
		m_nMinorVersion = nMinorVersion;
		m_nReserved2 = 0;
		m_nBuildYear = 0;
		m_nBuildMonth = 0;
		m_nBuildDay = 0;
		m_nBuildNumber = 0;
	}

	__Version& operator = (const __Version& v)
	{
		if (this == &v) {
			return *this;
		} else {
			this->m_nModuleId = v.m_nModuleId;
			this->m_nReserved1 = v.m_nReserved1;
			this->m_nMapCompanyId = v.m_nMapCompanyId;
			this->m_nMajorVersion = v.m_nMajorVersion;
			this->m_nMinorVersion = v.m_nMinorVersion;
			this->m_nReserved2 = v.m_nReserved2;
			this->m_nBuildYear = v.m_nBuildYear;
			this->m_nBuildMonth = v.m_nBuildMonth;
			this->m_nBuildDay = v.m_nBuildDay;
			this->m_nBuildNumber = v.m_nBuildNumber;

			return *this;
		}
	}

	void clear() {
		m_nModuleId = 0;
		m_nReserved1 = 0;
		m_nMapCompanyId = 0;
		m_nMajorVersion = 0;
		m_nMinorVersion = 0;
		m_nReserved2 = 0;
		m_nBuildYear = 0;
		m_nBuildMonth = 0;
		m_nBuildDay = 0;
		m_nBuildNumber = 0;
	}

	friend std::ostream& operator <<(std::ostream& os, const __Version v) {
		os << "Module Id\t" << v.m_nModuleId << std::endl;
		os << "Map Company Id\t" << v.m_nMapCompanyId << std::endl;
		os << "Major Version\t" << v.m_nMajorVersion << std::endl;
		os << "Minor Version\t" << v.m_nMinorVersion << std::endl;
		os << "Build Data\t" << "20" << v.m_nBuildDay << "/" << v.m_nBuildMonth << "/" << v.m_nBuildDay << std::endl;
		os << "Build Number\t" << v.m_nBuildNumber << std::endl;

		return os;
	}
};

struct Version
{
	char m_csCopyright[COPYRIGHT_SIZE];
	__Version m_verData;
	__Version m_verEId;
	__Version m_verNodeId;
	
	Version() {
		memset(m_csCopyright, 0, COPYRIGHT_SIZE);
	}

	void clear() {
		memset(m_csCopyright, 0, COPYRIGHT_SIZE);
		m_verData.clear();
		m_verEId.clear();
		m_verNodeId.clear();
	}

	friend std::ostream& operator <<(std::ostream& os, const Version v) {
		os << "Copyright(c)\t" << v.m_csCopyright << std::endl;
		os << v.m_verData << std::endl;
		os << v.m_verEId << std::endl;
		os << v.m_verNodeId << std::endl;

		return os;
	}
};

struct OriginalMapVersion
{
	UINT8 m_nMapCompanyId;					// 1: ritu
	UINT16 m_nMajorVersion;
	UINT8 m_nMinorVersion;
	UINT16 m_nReserved1;
	UINT16 m_nReserved2;

	OriginalMapVersion() {
		m_nMapCompanyId = 0;
		m_nMajorVersion = 0;
		m_nMinorVersion = 0;
		m_nReserved1 = 0;
		m_nReserved2 = 0;
	}

	OriginalMapVersion& operator = (const OriginalMapVersion& v)
	{
		if (this == &v) {
			return *this;
		} else {
			this->m_nMapCompanyId = v.m_nMapCompanyId;
			this->m_nMajorVersion = v.m_nMajorVersion;
			this->m_nMinorVersion = v.m_nMinorVersion;
			this->m_nReserved1 = v.m_nReserved1;
			this->m_nReserved2 = v.m_nReserved2;

			return *this;
		}
	}
};

struct Module {
	int m_nModuleId;
	int m_nMinorVersion;
	std::vector<std::string> m_fileList;

	Module(): m_nModuleId(0), m_nMinorVersion(0) { ;}
};