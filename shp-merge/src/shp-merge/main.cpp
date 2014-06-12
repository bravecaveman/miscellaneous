#include "shp_merge.h"
#include "wingetopt/wingetopt.h"
#include <fstream>
#include <sstream>

void usage(const char* arg) {
	string str(arg);
	size_t pos = str.rfind('\\');
	cout << str.substr(++pos) << " -s source-shapefile-directory -o output-directory" << endl;
}

int main(int argc, char* argv[]) {
	string src_dir, out_dir;

	while (1) {
		char ch = (char)getopt(argc, argv, "s:o:");
		if (ch == -1) break;
		switch (ch) {
		case 's':
			{
				src_dir.clear(); src_dir = optarg;
			}
			break;
		case 'o':
			{
				out_dir.clear(); out_dir = optarg;
			}
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (src_dir.empty()) {
		cout << "invalid source map directories." << endl;
		return false;
	}

	if (out_dir.empty()) {
		out_dir = ".";
	}

	DistrictShapeMerger dist_shp_merger;
	dist_shp_merger.set_file_path(src_dir.c_str(), out_dir.c_str());
	dist_shp_merger.find_mesh(src_dir.c_str());
	dist_shp_merger.arrange_mesh();

	SHPHandle hSHP;
	DBFHandle hDBF;
	dist_shp_merger.create_merged_files(hSHP, hDBF);

	vector<int> dist;
	dist_shp_merger.get_all_district(dist);

	vector<vector<Vertex> > merged_shape;
	for (int i = 0; i < dist.size(); ++i) {
		dist_shp_merger.merge(dist[i]);
		dist_shp_merger.save(hSHP, hDBF);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);

	return 0;
}