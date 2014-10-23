#include "version_writer.h"
#include <algorithm>
#include <locale.h> 
#include <iostream>

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring s2ws(const std::string& s)
{  
	setlocale(LC_ALL, "chs");   
	const char* _Source = s.c_str();  
	size_t _Dsize = s.size() + 1;  
	wchar_t* _Dest = new wchar_t[_Dsize];  
	wmemset(_Dest, 0, _Dsize);  
	mbstowcs(_Dest, _Source, _Dsize);  
	std::wstring result = _Dest;  
	delete[] _Dest;  
	setlocale(LC_ALL, "C");  
	return result;  
}

bool VersionWriter::Init( const char* root )
{
	m_root = root;
	m_config.Init("config.xml");

	return true;
}

int VersionWriter::GetModuleCount()
{
	return m_config.GetModuleCount();
}

bool VersionWriter::Write( int nModuleId )
{
	Version version;
	m_config.GetVersion(version, nModuleId);

	GetModuleFiles(nModuleId);
	std::string missingFile = CheckModuleFiles();
	if (!missingFile.empty()) {
		std::cout << "\"" << missingFile << "\"" << " was missed. Please check files of module " << nModuleId << std::endl;
		std::cout << "Press ANY KEY to exit ..." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < m_moduleFiles.size(); ++i) {
		bool bHasVersion = HasVersion(m_moduleFiles[i]);

		FILE* fp = fopen(m_moduleFiles[i].c_str(), "rb+");
		if (fp == NULL) {
			std::cout << "Open file failed: " << m_moduleFiles[i] << ". Press ANY KEY to exit ..." << std::endl;
			getchar();
			exit(EXIT_FAILURE);
		}

		if (bHasVersion) {
			fseek(fp, -VERSION_SIZE, SEEK_END);
			int nWrite = fwrite(&version, sizeof(Version), 1, fp);
			if (nWrite != 1) {
				fclose(fp);
				return false;
			}
		} else {
			fseek(fp, 0, SEEK_END);
			int nWrite = fwrite(&version, sizeof(Version), 1, fp);
			if (nWrite != 1) {
				fclose(fp);
				return false;
			}
		}
		fclose(fp);
	}

	return true;
}

bool VersionWriter::GetModuleFiles( int nModuleId )
{
	m_moduleFiles.clear();
	m_config.GetModuleFiles(m_moduleFiles, nModuleId);
	for (int i = 0; i < m_moduleFiles.size(); ++i) {
		m_moduleFiles[i] = m_root + "\\" + m_moduleFiles[i];
	}

	return true;
}

std::string VersionWriter::CheckModuleFiles()
{
	std::string sMissingFile;
	for (int i = 0; i < m_moduleFiles.size(); ++i) {
		if (!FileExists(s2ws(m_moduleFiles[i].c_str()).c_str())) {
			std::string basename = m_moduleFiles[i].substr(m_moduleFiles[i].rfind("\\") + 1);
			if (basename != "MapCompanyDataVer") {
				sMissingFile = basename;
			} else {
				// create original-map-version-file
				CreateOMVersionFile(m_moduleFiles[i].c_str());
			}
		}
	}

	return sMissingFile;
}

bool VersionWriter::HasVersion( const std::string& file )
{
	WIN32_FIND_DATA FileData;
	DWORD dwAttr;

	dwAttr = GetFileAttributes(s2ws(file).c_str());
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) {
		std::cout << "\"" << file << "\"" <<  " is a directory, which is not allowed." << std::endl;
		std::cout << "Press ANY KEY to exit ..." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	FILE* fp = fopen(file.c_str(), "rb");
	if (fp == NULL) {
		std::cout << "Open file failed: " << file << ". Press ANY KEY to exit ..." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_END);
	int nSize = ftell(fp);
	if (nSize < VERSION_SIZE) {
		fclose(fp);
		return false;
	} else {
		fseek(fp, -VERSION_SIZE, SEEK_END);
		Version version;
		fread(&version, VERSION_SIZE, 1, fp);
		if (strcmp(version.m_csCopyright, COPYRIGHT_STRING)) {
			fclose(fp);
			return false;
		} else {
			fclose(fp);
			return true;
		}
	}
}

bool VersionWriter::CreateOMVersionFile( const char* path )
{
	FILE* fp = fopen(path, "wb");
	if (fp == NULL) {
		std::cout << "Create Original Map Version File failed. Press ANY KEY to exit ..." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	OriginalMapVersion OMVersion = m_config.GetOMVersion();
	fwrite(&OMVersion, sizeof(OriginalMapVersion), 1, fp);
	fclose(fp);
}
