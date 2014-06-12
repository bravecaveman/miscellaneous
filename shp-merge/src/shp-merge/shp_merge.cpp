#include "shp_merge.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <fstream>

using namespace std;

#ifdef MAX_PATH
#undef MAX_PATH
#endif

#define MAX_PATH 256

#define MAX_FIELD_NAME 64
#define MIN_LONG 60.0
#define MIN_LAT 0.0
#define MESH_LONG_RANGE 1.0
#define MESH_LAT_RANGE 0.666666666666668
#define MESH_LAT_RANGE_IN_MINUTE 40.0

void DistrictShapeMerger::set_file_path( const char* src_dir, const char* out_dir )
{
	src_dir_ = src_dir;
	out_dir_ = out_dir;
}

bool DistrictShapeMerger::create_merged_files( SHPHandle& hSHP, DBFHandle& hDBF )
{
	string shp_path = out_dir_ + "\\merged_dist_area.shp";
	string dbf_path = out_dir_ + "\\merged_dist_area.dbf";
	hSHP = SHPCreate(shp_path.c_str(), SHPT_POLYGON);
	hDBF = DBFCreate(dbf_path.c_str());

	if (hSHP == NULL || hDBF == NULL) {
		return false;
	}

	DBFAddField(hDBF, "ID", FTInteger, 11, 0);
	DBFAddField(hDBF, "NAME", FTString, 50, 0);
	DBFAddField(hDBF, "PYNAME", FTString, 150, 0);
	DBFAddField(hDBF, "DISTID", FTInteger, 11, 0);

	new_shape_index_ = 0;

	return true;
}

bool DistrictShapeMerger::find_mesh(const char* dir)
{
	const char* filter = "district_area.dbf";

	if (dir == NULL) {
		all_mesh_.clear();
		return false;
	}

	char regex[MAX_PATH] = {0};
	sprintf(regex, "%s\\*", dir);

	struct _finddata_t file_info;
	intptr_t handle;

	handle = _findfirst(regex, &file_info);
	if (-1L == handle) {
		all_mesh_.clear();
		return false;
	}

	while (0L == _findnext(handle, &file_info)) {
		if (!strcmp(filter, file_info.name)) {
			char file[MAX_PATH] = {0};
			sprintf(file, "%s\\%s", dir, filter);

			string path(file);
			size_t pos1 = path.rfind('\\');
			size_t pos2 = path.rfind('\\', pos1 - 1);
			int mesh = atoi(path.substr(pos2 + 1, pos1 - 1).c_str());

			all_mesh_.push_back(mesh);
		}

		// if object is directory ?
		if (file_info.attrib & _A_SUBDIR) {
			if (!strcmp(file_info.name, ".") || !strcmp(file_info.name, "..")) {
				continue;			// if directory is "." or ".." ?
			} else {
				char sub_dir[MAX_PATH] = {0};
				sprintf(sub_dir, "%s\\%s", dir, file_info.name);			// recursively find file in sub-directories
				find_mesh(sub_dir);
			}
		}
	}

	_findclose(handle);

	return true;
}

bool DistrictShapeMerger::arrange_mesh()
{
	for (int i = 0; i < all_mesh_.size(); ++i) {
		Boundary bound;
		get_mesh_boundary(bound, all_mesh_[i]);
		mesh_bound_[all_mesh_[i]] = bound;

		char dbf_path[MAX_PATH] = {0};
		sprintf(dbf_path, "%s\\%d\\district_area.dbf", src_dir_.c_str(), all_mesh_[i]);
		DBFHandle hDBF = DBFOpen(dbf_path, "r");
		if (NULL == hDBF) {
			return false;
		}

		uint32 record_count = DBFGetRecordCount(hDBF);
		uint32 field_index = 0;

		for (int j = 0; j < record_count; ++j) {
			field_index = DBFGetFieldIndex(hDBF, "DISTID");
			uint32 dist_id = atoi((const char*)DBFReadStringAttribute(hDBF, j, field_index));
			if (dist_id == 0) {
				continue;
			}

			hash_map<int32, vector<int32> >::iterator district_itr;
			district_itr = dist_mesh_.find(dist_id);
			if (district_itr == dist_mesh_.end()) {
				dist_mesh_[dist_id].push_back(all_mesh_[i]);
			} else {
				vector<int32>::iterator mesh_itr;
				mesh_itr = find(dist_mesh_[dist_id].begin(), dist_mesh_[dist_id].end(), all_mesh_[i]);
				if (mesh_itr == dist_mesh_[dist_id].end()) {
					dist_mesh_[dist_id].push_back(all_mesh_[i]);
				}
			}
		}

		DBFClose(hDBF);
	}
}

void DistrictShapeMerger::get_all_district( vector<int>& dist )
{
	hash_map<int32, vector<int32> >::iterator itr;
	for (itr = dist_mesh_.begin(); itr != dist_mesh_.end(); ++itr) {
		dist.push_back(itr->first);
	}
}

bool DistrictShapeMerger::merge( int32 _dist_id )
{
	curr_merged_shape_.clear();
	vector<vector<Vertex> > separated_shape;

	vector<int> mesh = dist_mesh_[_dist_id];

	for (int i = 0; i < mesh.size(); ++i) {
		cout << "searching shape of district " << _dist_id << " from mesh " << mesh[i] << endl;

		char dbffil[MAX_PATH] = {0};
		sprintf(dbffil, "%s\\%d\\district_area.dbf", src_dir_.c_str(), mesh[i]);
		DBFHandle hDBF = DBFOpen(dbffil, "r");
		if (hDBF == NULL) {
			return false;
		}

		uint32 record_count = DBFGetRecordCount(hDBF);
		uint32 field_index = 0;
		vector<uint32> record_index, in_mesh;
		for (int j = 0; j < record_count; ++j) {
			field_index = DBFGetFieldIndex(hDBF, "DISTID");
			uint32 dist_id = atoi((const char*)DBFReadStringAttribute(hDBF, j, field_index));
			if (dist_id == _dist_id) {
				record_index.push_back(j);
			}
		}
		DBFClose(hDBF);

		char shpfil[MAX_PATH] = {0};
		sprintf(shpfil, "%s\\%d\\district_area.shp", src_dir_.c_str(), mesh[i]);
		SHPHandle hSHP = SHPOpen(shpfil, "r");
		if (hSHP == NULL) {
			return false;
		}

		/* get mesh boundary */
		Boundary bound = mesh_bound_[mesh[i]];

		cout << "extracting shape of district " << _dist_id << " from mesh " << mesh[i] << endl;

		/* get all shape in current mesh */
		for (int j = 0; j < record_index.size(); ++j) {
			vector<Vertex> shape;

			// read one single shape from shapefile
			SHPObject* shp = SHPReadObject(hSHP, record_index[j]);
			for (int32 k = 0; k < shp->nVertices - 1; ++k) {
				Vertex vertex(*(shp->padfX + k), *(shp->padfY + k));
				// skip corner vertex if exist any
				if (is_corner_vertex(vertex, bound) && mesh.size() > 1) {
					continue;
				}
				shape.push_back(vertex);
			}

			// district holds entire mesh
			if (shape.empty()) {
				continue;
			}

			// check whether the polygon intersect with mesh bound
			// if it doesn't, insert current polygon into merged shape result directly
			bool intersected = false;
			for (int k = 0; k < shape.size(); ++k) {
				if (is_intersection(shape[k], bound)) {
					intersected = true;
					break;
				}
			}
			if (!intersected) {
				curr_merged_shape_.push_back(shape);
				continue;
			}

			// get position of first intersection (with mesh-bound)
			int32 pos = 0;
			int32 prev = shape.size() - 1;
			for (int curr = 0; curr < shape.size(); ++curr) {
				if (is_intersection(shape[prev], bound) && !is_intersection(shape[curr], bound)) {
					pos = prev;
					break;
				}
				prev = curr;
			}
	
			/* make shape start from a intersection */
			vector<Vertex> swapper(shape);
			shape.clear();
			shape.insert(shape.end(), swapper.begin() + pos, swapper.end());
			shape.insert(shape.end(), swapper.begin(), swapper.begin() + pos);
	
			/* split single shape to multiple shapes (which both start-point and end-point are intersections) */ 
			vector<Vertex>::iterator first, last;
			first = shape.begin();
			last = first + 1;
			for ( ; last != shape.end(); ++last) {
				if (is_intersection(*last, bound)) {
					separated_shape.push_back(vector<Vertex>(first, ++last));
					first = last;
					if (last == shape.end()) {
						break;
					}
				}
			}
		}
		SHPClose(hSHP);
	}

	cout << "merging shapes of district " << _dist_id << endl;
	while (!separated_shape.empty()) {
		shpcat(curr_merged_shape_, separated_shape);
	}

	curr_dist_attr_.dist_code_ = _dist_id;
	cout << "Done" << endl;

	return true;
}

bool DistrictShapeMerger::save( SHPHandle& hSHP, DBFHandle& hDBF )
{
	for (int i = 0; i < curr_merged_shape_.size(); ++i) {
		write_shp(hSHP, curr_merged_shape_[i]);
		write_dbf(hDBF, curr_dist_attr_);
		++new_shape_index_;
	}

	return true;
}

bool DistrictShapeMerger::get_mesh_boundary( Boundary& bound, const int32 mesh )
{
	char shp_path[MAX_PATH] = {0};
	sprintf(shp_path, "%s\\%d\\district_area.shp", src_dir_.c_str(), mesh);
	SHPHandle hSHP = SHPOpen(shp_path, "r");
	if (hSHP == NULL) {
		return false;
	}

	double* pmin_bound = new double[4];
	double* pmax_bound = new double[4];
	SHPGetInfo(hSHP, NULL, NULL, pmin_bound, pmax_bound);
	bound.bl_ = Vertex(pmin_bound[0], pmin_bound[1]);
	bound.tr_ = Vertex(pmax_bound[0], pmax_bound[1]);

	delete[] pmin_bound;
	delete[] pmax_bound;

	SHPClose(hSHP);

	if (bound.tr_.x_ - bound.bl_.x_ != MESH_LONG_RANGE) {
		bound.bl_.x_ = (mesh % 100) + MIN_LONG;
		bound.tr_.x_ = bound.bl_.x_ + MESH_LONG_RANGE;
	}

	if (bound.tr_.y_ - bound.bl_.y_ != MESH_LAT_RANGE) {
		bound.bl_.y_ = ((mesh / 100) * MESH_LAT_RANGE_IN_MINUTE) / 60.0;
		bound.tr_.y_ = bound.bl_.y_ + MESH_LAT_RANGE;
	}

	return true;
}

bool DistrictShapeMerger::is_intersection( const Vertex& vertex, const Boundary& bound )
{
	return ((std::abs(vertex.x_ - bound.tr_.x_) <= epsilon) || 
				(std::abs(vertex.x_ - bound.bl_.x_) <= epsilon) || 
				(std::abs(vertex.y_ - bound.tr_.y_) <= epsilon) || 
				(std::abs(vertex.y_ - bound.bl_.y_) <= epsilon));
}

bool DistrictShapeMerger::is_corner_vertex( const Vertex& vertex, const Boundary& bound )
{
	return (vertex == bound.top_left() || 
				vertex == bound.top_right() || 
				vertex == bound.bottom_left() || 
				vertex == bound.bottom_right());
}

bool DistrictShapeMerger::shpcat( vector<vector<Vertex> >& merged_shape, vector<vector<Vertex> >& separated_shape )
{
	//create new merged shape
	merged_shape.push_back(vector<Vertex>(0, Vertex(0, 0)));

	merged_shape.rbegin()->insert(merged_shape.rbegin()->end(), separated_shape[0].begin(), separated_shape[0].end());
	separated_shape.erase(separated_shape.begin());

	vector<vector<Vertex> >::iterator itr = separated_shape.begin();
	while (!separated_shape.empty()) {
		//notice: reverse iterator, '+' means go back to the previous
		if ((*itr)[0] == *(merged_shape.rbegin()->rbegin()) && (*itr)[1] != *(merged_shape.rbegin()->rbegin() + 1)) {
			if (*((*itr).rbegin()) == (*(merged_shape.rbegin()))[0]) {
				merged_shape.rbegin()->insert(merged_shape.rbegin()->end(), itr->begin() + 1, itr->end() - 1);
				separated_shape.erase(itr);
				break;
			} else {
				merged_shape.rbegin()->insert(merged_shape.rbegin()->end(), itr->begin() + 1, itr->end());
				itr = separated_shape.erase(itr);
				if (itr == separated_shape.end()) {
					itr = separated_shape.begin();
				}
				continue;
			}
		}

		++itr;
		if (itr == separated_shape.end()) {
			itr = separated_shape.begin();
		}
	}

	return true;
}

void DistrictShapeMerger::write_shp( SHPHandle hSHP, vector<Vertex>& shape )
{
	int vertex_count = shape.size();

	double* padfX = new double[vertex_count];
	double* padfY = new double[vertex_count];
	for (int i = 0; i < vertex_count; ++i) {
		padfX[i] = shape[i].x_;
		padfY[i] = shape[i].y_;
	}

	SHPObject* shpobj = SHPCreateObject(SHPT_POLYGON, new_shape_index_, 0, NULL, NULL, vertex_count, padfX, padfY, NULL, NULL);
	SHPComputeExtents(shpobj);
	SHPWriteObject(hSHP, -1, shpobj);
	SHPDestroyObject(shpobj);

	delete[] padfX;
	delete[] padfY;
}

void DistrictShapeMerger::write_dbf( DBFHandle hDBF, const DistrictAttribute& dist_attr )
{
	DBFWriteIntegerAttribute(hDBF, new_shape_index_, 0, new_shape_index_);
	DBFWriteStringAttribute(hDBF, new_shape_index_, 1, dist_attr.dist_name_.c_str());
	DBFWriteStringAttribute(hDBF, new_shape_index_, 2, dist_attr.dist_pinyin_.c_str());
	DBFWriteIntegerAttribute(hDBF, new_shape_index_, 3, dist_attr.dist_code_);
}

bool extract_shape_from_national_map( vector<vector<Vertex> >& district_shape, int32 _district_id )
{
	char* dbffil = "E:\\Data\\Shapefile\\ruitu_18\\country\\town_dist_area.dbf";
	DBFHandle hDBF = DBFOpen(dbffil, "r");
	if (hDBF == NULL) {
		return false;
	}

	uint32 record_count = DBFGetRecordCount(hDBF);
	uint32 field_index = 0;
	vector<uint32> record_index;
	for (int j = 0; j < record_count; ++j) {
		field_index = DBFGetFieldIndex(hDBF, "DISTID");
		uint32 dist_id = atoi((const char*)DBFReadStringAttribute(hDBF, j, field_index));
		if (dist_id == _district_id) {
			record_index.push_back(j);
		}
	}

	char* shpfil = "E:\\Data\\Shapefile\\ruitu_18\\country\\town_dist_area.shp";
	SHPHandle hSHP = SHPOpen(shpfil, "r");
	if (hSHP == NULL) {
		return false;
	}

	district_shape.resize(record_index.size());
	for (int j = 0; j < record_index.size(); ++j) {
		SHPObject* shp = SHPReadObject(hSHP, record_index[j]);
		for (int32 k = 0; k < shp->nVertices - 1; ++k) {
			district_shape[j].push_back(Vertex(*(shp->padfX + k), *(shp->padfY + k)));
		}
	}
}

void print_shape( vector<vector<Vertex> >& shape, int district_code, bool is_merged )
{
	char filename[MAX_PATH] = {0};
	sprintf(filename, "%d_%s.txt", district_code, (is_merged? "merged": "original"));

	ofstream fs(filename);
	if (fs.is_open()) {
		for (int i = 0; i < shape.size(); ++i) {
			fs << "Part " << i << endl;
			fs << "Total Vertices: " << shape[i].size() << endl;
			for (int j = 0; j < shape[i].size(); ++j) {
				fs << setprecision(6) << shape[i][j] << endl;
			}
		}
		fs.close();
	}
}

void get_dbf_field_info(DBFHandle hDBF)
{
	int field_count = DBFGetFieldCount(hDBF);
	for (int i = 0; i < field_count; ++i) {
		char field_name[MAX_FIELD_NAME] = {0};
		int width = 0;
		int decimal = 0;
		DBFFieldType field_info = DBFGetFieldInfo(hDBF, i, field_name, &width, &decimal);

		cout << field_name << "|" << width << "|" << decimal << endl;
	}
}
