#ifndef _SHPCAT_H_
#define _SHPCAT_H_

#include "shapelib/shapefil.h"
#include <io.h>
#include <vector>
#include <string>
#include <hash_map>
#include <cmath>
#include <iostream>

using namespace std;
using namespace stdext;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

const double epsilon = 1e-6;

struct Vertex {
	double x_;
	double y_;

	Vertex() { ;}

	Vertex(double x, double y) {
		x_ = x;
		y_ = y;
	}

	bool operator ==(const Vertex& vertex) const {
		return ((std::abs(vertex.x_ - x_) <= epsilon) && (std::abs(vertex.y_ - y_) <= epsilon));
	}

	bool operator !=(const Vertex& vertex) const {
		return ((std::abs(vertex.x_ - x_) > epsilon) || (std::abs(vertex.y_ - y_) > epsilon));
	}

	Vertex& operator =(const Vertex& vertex) {
		if (&vertex == this) {
			return *this;
		} else {
			x_ = vertex.x_;
			y_	= vertex.y_;
			return *this;
		}
	}

	friend ostream& operator <<(ostream& stream, const Vertex vertex) {
		stream << "(" << vertex.x_ << ", " << vertex.y_ << ")";
		return stream;
	}
};

struct MeshShape {
	vector<Vertex> shape_;

	MeshShape() { ;}
};

struct Boundary {
	Vertex tr_;
	Vertex bl_;

	Boundary() { ;}

	Boundary(const Vertex& tr, const Vertex& bl) {
		tr_ = tr;
		bl_ = bl;
	}

	Vertex top_left() const {
		return Vertex(bl_.x_, tr_.y_);
	}

	Vertex top_right() const {
		return tr_;
	}

	Vertex bottom_left() const {
		return bl_;
	}

	Vertex bottom_right() const {
		return Vertex(tr_.x_, bl_.y_);
	}
};

struct DistrictAttribute {
	string dist_name_;
	string dist_pinyin_;
	int dist_code_;
};

class DistrictShapeMerger
{
public:
	DistrictShapeMerger() { ;}
	~DistrictShapeMerger() { ;}

	void set_file_path(const char* src_dir, const char* out_dir);
	bool create_merged_files(SHPHandle& hSHP, DBFHandle& hDBF);
	bool find_mesh(const char* dir);
	bool arrange_mesh();
	void get_all_district(vector<int>& dist);
	bool merge(int32 dist_id);
	bool save(SHPHandle& hSHP, DBFHandle& hDBF);

private:
	bool get_mesh_boundary(Boundary& bound, const int32 mesh);
	bool is_intersection(const Vertex& vertex, const Boundary& bound);
	bool is_corner_vertex(const Vertex& vertex, const Boundary& bound);
	bool shpcat(vector<vector<Vertex> >& merged_shape, vector<vector<Vertex> >& separated_shape);
	void write_shp(SHPHandle hSHP, vector<Vertex>& shape);
	void write_dbf(DBFHandle hDBF, const DistrictAttribute& dist_attr);

private:
	string src_dir_;
	string out_dir_;

	vector<int> all_mesh_;
	hash_map<int32, vector<int32> > dist_mesh_;
	hash_map<int32, Boundary> mesh_bound_;

	vector<vector<Vertex> > curr_merged_shape_;
	DistrictAttribute curr_dist_attr_;

	int new_shape_index_;
};

bool extract_shape_from_national_map(vector<vector<Vertex> >& shape, int32 district_id);
void print_shape(vector<vector<Vertex> >& shape, int district_code, bool is_merged);
void get_dbf_field_info(DBFHandle hDBF);

#endif