#include "version_types.h"
#include "wingetopt/wingetopt.h"
#include "version_writer.h"
#include <iostream>
#include <string>

std::string path;

void usage(const std::string& prg)
{
	size_t pos = prg.rfind('\\');
	std::string appName = prg.substr(++pos, std::string::npos);
	std::cerr << "usage: " << appName << " -d directory [-m module]" << std::endl;
}

int main(int argc, char* argv[])
{
	std::string root;
	int nModuleId = 0;

	while (1)
	{
		char ch = (char)getopt(argc, argv, "d:m:");
		if (ch == -1) break;
		switch (ch)
		{
		case 'd':
			{
				path.clear(); path = optarg;
				if (path.empty()) {
					std::cerr << "Empty directory is not allowed." << std::endl;
					std::cout << "Press ANY KEY to exit ..." << std::endl;
					getchar();
					exit(EXIT_FAILURE);
				}
				root = path;
			}
			break;
		case 'm':
			{
				path.clear(); path = optarg;
				if (path.empty()) {
					std::cerr << "Module Id is missing. Press ANY KEY to eixt ..." << std::endl;
					getchar();
					exit(EXIT_FAILURE);
				}
				nModuleId = atoi(path.c_str());
			}
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (root.empty()) {
		std::cout << "Please specify the data file root. Press ANY KEY to exit ... " << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	VersionWriter writer;
	writer.Init(root.c_str());

	int nModuleCount = writer.GetModuleCount();
	if (nModuleId > nModuleCount || nModuleId < 0) {
		std::cout << "Module Id should be in the range of [1, " << nModuleCount << "]" << std::endl;
		std::cout << "Press ANY KEY to exit ... " << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	if (nModuleId == 0) {
		// default situation: write all module's version
		for (int i = 0; i < nModuleCount; ++i) {
			writer.Write(i + 1);
		}
	} else {
		writer.Write(nModuleId);
	}

	return 0;
}