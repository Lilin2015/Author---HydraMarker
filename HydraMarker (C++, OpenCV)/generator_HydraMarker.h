#pragma once
#include <windows.h>
#include <Psapi.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>

#define TIME_START(X)	double X = (double)getTickCount();
#define TIME_END(X)		X = 1000 * ((double)getTickCount() - X) / getTickFrequency();

enum class METHOD { FBWFC, BWFC, DOF, GENETIC};

using namespace cv;
using namespace std;

struct ComparePoint
{
	bool operator () (const Point& a, const Point& b) const
	{
		return (a.x < b.x) || (a.x == b.x && a.y < b.y);
	}
};

class generator_HydraMarker
{
public:
	// set the marker field
	// marker field must be a Mat1b, composed of four states:
	// 0, 1, 2(unknown), 3(hollow)
	void set_field(const Mat1b& input_field);
	void set_field(const string path);

	// set the tag shapes
	// tag shape must be a Mat1b, which is composed of two states:
	// 0 (hollow), 1 (solid)
	// multiple tag shapes can be supported simutaneously
	void set_tagShape(const vector<Mat1b>& input_tagShape);
	void set_tagShape(const Mat1b& input_tagShape);

	// generate marker field (filling the unknowns "state = 2" in the current marker field)
	// method:
	// "FBWFC", fast, it can finish most tasks in a very short period of time
	// "BWFC", best step efficiency, but very time-consuming, not-recommended, for experiments
	// "DOF", fast and lightweight, but can only finish the easiest task, for experiments
	// "GENETIC", very time-consuming (days, even weeks), but it can finish most tasks eventually
	void generate(const METHOD method = METHOD::FBWFC, const double max_ms = 3600000000, const int max_trial = INT_MAX, const bool show_process = true, const string path = "process.log");

	// show current marker field
	// 0 - black, 1 - white, 2 - red, 3 - gray
	Mat3b show();

	// save current marker field to the given path
	// the suffix of the file must be ".field"
	// the file is organized in the following way:
	// - 1st row: the row and column number of the marker field
	// - 2nd row: the elements of marker field, column-major order
	// - 3rd row: the number of tag shapes
	// - 4th row: the row and column number of the 1st tag shape
	// - 5th row: the elements of tag shape, column-major order
	// - ...: information of other tag shapes, if exist
	void save(const string path);

	// calculate the hamming distance histogram of current marker field
	// only support 3-order and 4-order ordinary maker field yet
	vector<int> HDhist(const int order = 3);

private:

	void generate_BWFC(const bool show_process = true, const string path = "process.log");
	void generate_FBWFC(const bool show_process = true, const string path = "process.log");
	void generate_DOF(const bool show_process = true, const string path = "process.log");
	void generate_GENETIC(const bool show_process = true, const string path = "process.log");

	ofstream log;
	Mat1b field;
	vector<Mat1b> tag_shape;
	double time_start;
	double max_ms;
	double max_trial;

	// the multi-way tree used to manager the filling process
	// collapse suggestion, Vec3i: x, y, assigned_value;
	vector<vector<Vec3i>> tree;
	int d = 0;

	vector<int> rot_prop;
	vector<Mat1i> tag2X, tag2Y;
	vector<Mat1b> tag_pool;	// only for bWFC
	vector<Mat> tagMap;

	// table: XY of state -> tag information
	// the first-layer vector corresponds to different tag shapes
	// the second-layer vector corresponds to the 1, 2, or 4 orientations 
	// each Mat1i is a Nx2 matrix, for each row: the index of the tag containing XY, the index of XY in this tag (left to right)
	map<Point, vector<vector<Mat1i>>, ComparePoint>	XY2tag;

	void initial();
	bool has_conflict(const vector<Mat1b>& state_complete, const vector<Mat1b>& complete);

	// return the XY of a state with largest DOF risk
	// return Point(-1,-1) if there is no unknown
	Point DOF(const vector<Mat1b>& state_byTag, const vector<Mat1b>& incomplete, vector<Mat1i>& freedom);

	// check the risks of assigning 0 and 1 at focused point f
	// return 0, if field(f)<-0 takes less risk
	// return 1, if field(f)<-1 takes less risk
	bool risk(const vector<Mat1b>& state_byTag, const vector<Mat1b>& complete, const Point f);

	void hit(const vector<Mat1b>& state_byTag, vector<Vec3i>& suggest);

	void build_table();	// compose tag2X, tag2Y and XY2tag;
	void build_pool();	// compose tag_pool
	void read_state(vector<Mat1b>& state_byTag);	// read current state based on tag2X and tag2Y;
	void get_complete(const vector<Mat1b>& state_byTag, vector<Mat1b>& complete);	  // filter out complete tags in state_byTag
	void get_incomplete(const vector<Mat1b>& state_byTag, vector<Mat1b>& incomplete, vector<Mat1i>& freedom); // filter out incomplete tags in state_byTag		 

	void meshgrid(const Range xgv, const Range ygv, Mat1i& X, Mat1i& Y);

	// EXP: genetic
	int calc_conflict(const Mat1b& field, Mat1b& conflict_region);
};

