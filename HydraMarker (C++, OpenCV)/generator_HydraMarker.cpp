#include "generator_HydraMarker.h"

void generator_HydraMarker::set_field(const Mat1b& input_field)
{
	input_field.copyTo(this->field);
}

void generator_HydraMarker::set_field(const string path)
{
	// write in the marker field
	ifstream input_file(path);
	int row, col;
	input_file >> row >> col;
	Mat1i tfield = Mat1i::zeros(row, col);
	for (int i = 0; i < tfield.total(); i++)	input_file >> tfield(i);
	this->field = Mat1b(tfield);
	// write tagShape
	this->tag_shape.clear();
	int shape_num;
	input_file >> shape_num;
	for (size_t i = 0; i < shape_num; i++)
	{
		int srow, scol;
		input_file >> srow >> scol;
		Mat1i shape = Mat1i::zeros(srow, scol);
		for (int t = 0; t < shape.total(); t++)	input_file >> shape(t);
		this->tag_shape.push_back(Mat1b(shape));
	}
}

void generator_HydraMarker::set_tagShape(const vector<Mat1b>& input_tagShape)
{
	this->tag_shape.clear();
	this->tag_shape.resize(input_tagShape.size());
	for (size_t i = 0; i < input_tagShape.size(); i++)
	{
		input_tagShape[i].copyTo(this->tag_shape[i]);
	}
}

void generator_HydraMarker::set_tagShape(const Mat1b& input_tagShape)
{
	this->tag_shape.clear();
	this->tag_shape.push_back(input_tagShape);
}

void generator_HydraMarker::generate(const METHOD method, const double max_ms, const int max_trial, const bool show_process, const string path)
{
	// check input
	if (this->field.empty())		throw __FUNCTION__ + string(", ") + ("empty field! \n");
	if (this->tag_shape.empty())	throw __FUNCTION__ + string(", ") + ("empty tag shape! \n");

	// terminal condition
	this->max_ms = max_ms;
	this->max_trial = max_trial;

	// check log suffix
	string suffix = ".log";
	int idx = path.find(suffix, path.size() - suffix.size());
	if (idx == string::npos)	throw __FUNCTION__ + string(", ") + ("the suffix of the log file must be .log");
	std::cout << "log file will be refreshed, path: " + path << endl;

	// reset log file
	this->log = ofstream(path, ofstream::trunc);

	// cout info
	std::cout << "generate marker field by ";
	if (method == METHOD::DOF) std::cout << "DOF-filling";
	if (method == METHOD::BWFC) std::cout << "bWFC";
	if (method == METHOD::FBWFC) std::cout << "fast-bWFC";
	if (method == METHOD::GENETIC) std::cout << "Genetic: Uniform Marker Fields, Camera Localization By Orientable De Bruijn Tori (we fail to run the published codes and fail to contact the authors till 2022.7.21, thus this is the genetic algorithm we implemented based on the paper)";
	std::cout << endl;
	std::cout << "in " << this->max_ms << " ms" << endl;
	
	// jump to methods
	this->time_start = (double)getTickCount();
	if (method == METHOD::DOF)		generate_DOF(show_process, path);
	if (method == METHOD::BWFC)		generate_BWFC(show_process, path);
	if (method == METHOD::FBWFC)	generate_FBWFC(show_process, path);
	if (method == METHOD::GENETIC)	generate_GENETIC(show_process, path);
}

Mat3b generator_HydraMarker::show()
{
	// build look-up table
	Mat1b tableR = Mat1b::zeros(1, 256);
	Mat1b tableG = Mat1b::zeros(1, 256);
	Mat1b tableB = Mat1b::zeros(1, 256);
	tableR(1) = 255;	tableR(2) =   0;	tableR(3) = 128;
	tableG(1) = 255;	tableG(2) =   0;	tableG(3) = 128;
	tableB(1) = 255;	tableB(2) = 255;	tableB(3) = 128;

	// transfer
	Mat1b stateR, stateG, stateB;
	LUT(field, tableR, stateR);
	LUT(field, tableG, stateG);
	LUT(field, tableB, stateB);

	// merge
	Mat3b stateShow;
	merge(vector<Mat1b>{stateR, stateG, stateB}, stateShow);

	// show
	namedWindow("state", WINDOW_NORMAL);
	imshow("state", stateShow);

	return stateShow;
}

void generator_HydraMarker::save(const string path)
{
	// check suffix
	string suffix = ".field";
	int idx = path.find(suffix, path.size() - suffix.size());
	if (idx == string::npos)	throw __FUNCTION__ + string(", ") + ("the suffix of the saved file must be .field");

	// write in the marker field
	ofstream output_file(path);
	output_file << this->field.rows << " " << this->field.cols << endl;
	for (int i : this->field)	output_file << i << " ";
	output_file << endl;
	
	// write tagShape
	output_file << this->tag_shape.size() << endl;
	for (Mat1b mat : this->tag_shape)
	{
		output_file << mat.rows << " " << mat.cols << endl;
		for (int i : mat)	output_file << i << " ";
		output_file << endl;
	}
	
	cout << "marker field has been saved in path: " + path << endl;
}

vector<int> generator_HydraMarker::HDhist(const int order)
{
	// build tag2X, tag2Y
	Mat1b mat = Mat1b::ones(order, order);
	Mat1i Xtable, Ytable;

	// get the XY of the states hit by current tag shape on top-left
	Mat1i X, Y;
	meshgrid(Range(0, mat.cols), Range(0, mat.rows), X, Y);
	X = X.setTo(-1, mat != 1).reshape(1, 1);
	Y = Y.setTo(-1, mat != 1).reshape(1, 1);

	// rotate the tag shape by 4 orientations
	for (size_t rot = 0; rot < 4; rot++)
	{
		// get the XY from rotated view
		double minX, minY, maxX, maxY;
		if (rot > 0)
		{
			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			minMaxLoc(Y, &minY, &maxY, NULL, NULL);
			X = -X + minX + maxX;
		}
		else
		{
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			minMaxLoc(Y, &minY, &maxY, NULL, NULL);
		}

		Mat1i ind = Y * this->field.cols + X;

		// for each orientation, scan the marker field
		for (int y = -minY; y < this->field.rows - maxY; y++)
		{
			for (int x = -minX; x < this->field.cols - maxX; x++)
			{
				// ignore tags containing hollows
				Mat c_state;
				remap(this->field, c_state, Mat1f(X + x), Mat1f(Y + y), INTER_NEAREST);
				if (countNonZero(c_state == 3) <= 0)
				{
					Xtable.push_back(X + x);
					Ytable.push_back(Y + y);
				}
			}
		}
	}

	// build map1 and map2 for fast read
	Mat tagMap, t_map2;
	convertMaps(Mat1f(Xtable), Mat1f(Ytable), tagMap, t_map2, CV_16SC2, true);

	int maxnum = 32766;	// the maximum side length of opencv remap function is 32767

	int block_num = tagMap.rows / maxnum + 1;
	Mat1b state_byTag;
	for (int im = 0; im < block_num; im++)
	{
		Mat1b b_state;
		remap(this->field, b_state,
			tagMap.rowRange(im * maxnum, min((im + 1) * maxnum, tagMap.rows)),
			Mat(),
			INTER_NEAREST);
		state_byTag.push_back(b_state);
	}

	// hamming distance histogram
	int* hist3 = new int[10]();
	int* hist4 = new int[17]();
	int* hist5 = new int[26]();

	if (order == 3)
	{
		for (size_t a = 0; a < state_byTag.rows - 1; a++)
		{
			for (size_t b = a + 1; b < state_byTag.rows; b++)
			{
				Mat diff = (state_byTag.row(a) != state_byTag.row(b))/255;
				int dist = sum(diff)(0);
				if (dist==0)
				{
					int c = 1;
				}
				hist3[dist]++;
			}
		}
		cout << "\nHamming distance historgram:\n";
		for (size_t i = 0; i < 10; i++)
		{
			cout << hist3[i] << " ";
		}
		cout << endl;
		vector<int> vec(hist3, hist3+10);
		return vec;
	}
	if (order == 4)
	{
		for (size_t a = 0; a < state_byTag.rows - 1; a++)
		{
			for (size_t b = a + 1; b < state_byTag.rows; b++)
			{
				Mat diff = (state_byTag.row(a) != state_byTag.row(b)) / 255;
				int dist = sum(diff)(0);
				hist4[dist]++;
			}
		}
		cout << "\nHamming distance historgram:\n";
		for (size_t i = 0; i < 17; i++)
		{
			cout << hist4[i] << " ";
		}
		cout << endl;
		vector<int> vec(hist4, hist4 + 17);
		return vec;
	}
	if (order == 5)
	{
		for (size_t a = 0; a < state_byTag.rows - 1; a++)
		{
			for (size_t b = a + 1; b < state_byTag.rows; b++)
			{
				Mat diff = (state_byTag.row(a) != state_byTag.row(b)) / 255;
				int dist = sum(diff)(0);
				hist5[dist]++;
			}
		}
		cout << "\nHamming distance historgram:\n";
		for (size_t i = 0; i < 26; i++)
		{
			cout << hist5[i] << " ";
		}
		cout << endl;
		vector<int> vec(hist5, hist5 + 26);
		return vec;
	}
}

void generator_HydraMarker::generate_BWFC(const bool show_process, const string path)
{
	// initial
	this->d = 0;
	this->tree.clear();
	for (size_t i = 0; i < this->field.rows * this->field.cols; i++)
		this->tree.push_back(vector<Vec3i>{});

	build_table();
	build_pool();

	// upper bound estimation
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		double tag_num = pow(2, sum((this->tag_shape[shape] == 1) / 255)(0)-sqrt(this->rot_prop[shape]));
		Mat need_fill;
		erode(this->field == 2, need_fill, this->tag_shape[shape], Point(-1, -1), 1, BORDER_CONSTANT, 0);
		double need_num = sum((need_fill == 255) / 255)(0);
		if (tag_num < need_num)
		{
			std::cout << "\nthe request marker field is larger than possible!" << endl;
			break;
		}
	}

	// get the number of overall tags
	int num_tag = 0;		
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
		num_tag += this->tag2X[shape].rows;

	// start!
	int step_count = 1;
	double progress = 0;
	double time_first_stuck = (double)getTickCount();
	while (true)
	{
		// update marker field info
		vector<Mat1b> state_byTag;
		read_state(state_byTag);

		vector<Mat1b> complete;
		get_complete(state_byTag, complete);

		// update log info
		ostringstream log_info;
		log_info << "step: " << step_count;

		double time_step = 1000 * ((double)getTickCount() - this->time_start) / getTickFrequency();
		log_info << fixed << setprecision(0) << ", time: " << time_step << "ms";

		PROCESS_MEMORY_COUNTERS memCounter;
		BOOL result = K32GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
		log_info << ", mem: " << memCounter.WorkingSetSize / 1048576 << "MB";

		int num_complete = 0;
		for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
			num_complete += complete[shape].rows;

		// get stuck longer than 0.5T will terminate the process
		double cur_progress = 100.0 * num_complete / num_tag;
		if (cur_progress <= progress)
		{
			double time_stuck = 1000 * ((double)getTickCount() - time_first_stuck) / getTickFrequency();
			if (cur_progress > 0 && time_stuck > 60000 && time_stuck > 0.5 * time_step)
			{
				std::cout << "\nget stuck!" << endl;
				return;
			}
		}
		else
		{
			time_first_stuck = (double)getTickCount();
		}
		progress = max(progress, cur_progress);
		log_info << fixed << setprecision(2) << ", progress: " << progress << "%";

		// write log file and display on console
		this->log << step_count << " " << time_step << " " << progress << " " << memCounter.WorkingSetSize << endl;
		std::cout << log_info.str() << "\r";

		
		if (time_step > this->max_ms || step_count >= this->max_trial)
		{
			std::cout << "\ntime/step out!" << endl;
			return;
		}

		// paper: if conflict happens, then
		if (has_conflict(state_byTag, complete))
		{
			do
			{
				if (d == 0)	throw __FUNCTION__ + string(", ") + ("can not generate such a marker field, conflict inevitable! \n");
				d--;
				this->field(Point(tree[d][0][0], tree[d][0][1])) = 2;	// reset the last assignment, which is wrong
				tree[d].erase(tree[d].begin());							// remove the wrong assignment
			} while (tree[d].empty());	// paper: do until non-empty
		}
		else
		{
			if (countNonZero(this->field == 2) == 0)	break;
			vector<Vec3i> suggest;
			hit(state_byTag, suggest);
			if (suggest.empty())
			{
				do
				{
					if (d == 0)	throw __FUNCTION__ + string(", ") + ("can not generate such a marker field, conflict inevitable! \n");
					d--;
					this->field(Point(tree[d][0][0], tree[d][0][1])) = 2;	// reset the last assignment, which is wrong
					tree[d].erase(tree[d].begin());							// remove the wrong assignment
					if (show_process)	show();
					waitKey(1);
				} while (tree[d].empty());	// paper: do until non-empty
			}
			else
			{
				this->tree[d] = suggest;
			}
		}
		this->field(Point(tree[d][0][0], tree[d][0][1])) = tree[d][0][2];	// always apply the first suggestion
		d++;

		step_count++;
		if (show_process) {
			show();
			waitKey(1);
		}
	}
	std::cout << "\ncomplete!" << endl;
}

void generator_HydraMarker::generate_FBWFC(const bool show_process, const string path)
{
	// prepare
	this->d = 0;
	this->tree.clear();
	for (size_t i = 0; i < this->field.rows * this->field.cols; i++)
		this->tree.push_back(vector<Vec3i>{});

	build_table();

	// upper bound estimation
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		double tag_num = pow(2, sum((this->tag_shape[shape] == 1) / 255)(0) - sqrt(this->rot_prop[shape]));
		Mat need_fill;
		erode(this->field == 2, need_fill, this->tag_shape[shape], Point(-1, -1), 1, BORDER_CONSTANT, 0);
		double need_num = sum((need_fill == 255) / 255)(0);
		if (tag_num < need_num)
		{
			std::cout << "\nthe request marker field is larger than possible!" << endl;
			break;
		}
	}

	// get the number of overall tags
	int num_tag = 0;		
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
		num_tag += this->tag2X[shape].rows;

	// start!
	int step_count = 1;
	double progress = 0;
	double time_first_stuck = (double)getTickCount();
	while (true)
	{
		// update marker field info
		vector<Mat1b> state_byTag;
		read_state(state_byTag);

		vector<Mat1b> complete;
		get_complete(state_byTag, complete);

		vector<Mat1b> incomplete;
		vector<Mat1i> incomplete_info;
		get_incomplete(state_byTag, incomplete, incomplete_info);

		// update log info
		ostringstream log_info;
		log_info << "step: " << step_count;

		double time_step = 1000 * ((double)getTickCount() - this->time_start) / getTickFrequency();
		log_info << fixed << setprecision(0) << ", time: " << time_step << "ms";

		PROCESS_MEMORY_COUNTERS memCounter;
		BOOL result = K32GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
		log_info << ", mem: " << memCounter.WorkingSetSize / 1048576 << "MB";

		int num_complete = 0;
		for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
			num_complete += complete[shape].rows;
		// get stuck longer than 0.5T will terminate the process
		double cur_progress = 100.0 * num_complete / num_tag;
		if (cur_progress <= progress)
		{
			double time_stuck = 1000 * ((double)getTickCount() - time_first_stuck) / getTickFrequency();
			if (cur_progress > 110 && time_stuck > 60000 && time_stuck > 0.5 * time_step)
			{
				std::cout << "\nget stuck!" << endl;
				return;
			}
		}
		else
		{
			time_first_stuck = (double)getTickCount();
		}
		progress = max(progress, cur_progress);
		log_info << fixed << setprecision(2) << ", progress: " << progress << "%";

		// write log file and display on console
		this->log << step_count << " " << time_step << " " << progress << " " << memCounter.WorkingSetSize << endl;
		cout << log_info.str() << "\r";

		
		if (time_step > this->max_ms || step_count >= this->max_trial)
		{
			cout << "\ntime/step out!" << endl;
			return;
		}

		// paper: if conflict happens, then
		if (has_conflict(state_byTag, complete))
		{
			do
			{
				if (d == 0)	throw __FUNCTION__ + string(", ") + ("can not generate such a marker field, conflict inevitable! \n");
				d--;
				this->field(Point(tree[d][0][0], tree[d][0][1])) = 2;	// reset the last assignment, which is wrong
				tree[d].erase(tree[d].begin());							// remove the wrong assignment
			} while (tree[d].empty());	// paper: do until non-empty
		}
		else
		{
			if (countNonZero(this->field == 2) == 0)	break;
			Point focus = DOF(state_byTag, incomplete, incomplete_info);
			if (d==6)
			{
				int a = 1;
			}
			bool value_suggest = risk(state_byTag, complete, focus);
			this->tree[d] = vector<Vec3i>{ Vec3i(focus.x, focus.y, value_suggest), Vec3i(focus.x, focus.y, 1 - value_suggest) };
		}
		this->field(Point(tree[d][0][0], tree[d][0][1])) = tree[d][0][2];	// always apply the first suggestion
		d++;

		step_count++;
		if (show_process) {
			show();
			waitKey(1);
		}
	}
	std::cout << "\ncomplete!" << endl;
}

void generator_HydraMarker::generate_DOF(const bool show_process, const string path)
{
	// prepare
	this->d = 0;
	this->tree.clear();
	for (size_t i = 0; i < this->field.rows * this->field.cols; i++)
		this->tree.push_back(vector<Vec3i>{});

	build_table();

	// upper bound estimation
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		double tag_num = pow(2, sum((this->tag_shape[shape] == 1) / 255)(0) - sqrt(this->rot_prop[shape]));
		Mat need_fill;
		erode(this->field == 2, need_fill, this->tag_shape[shape], Point(-1, -1), 1, BORDER_CONSTANT, 0);
		double need_num = sum((need_fill == 255) / 255)(0);
		if (tag_num < need_num)
		{
			std::cout << "\nthe request marker field is larger than possible!" << endl;
			break;
		}
	}

	// get the number of overall tags
	int num_tag = 0;		
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
		num_tag += this->tag2X[shape].rows;

	// start!
	int step_count = 1;
	double progress = 0;
	double time_first_stuck = (double)getTickCount();
	while (true)
	{
		// update marker field info
		vector<Mat1b> state_byTag;
		read_state(state_byTag);

		vector<Mat1b> complete;
		get_complete(state_byTag, complete);

		vector<Mat1b> incomplete;
		vector<Mat1i> incomplete_info;
		get_incomplete(state_byTag, incomplete, incomplete_info);

		// update log info
		ostringstream log_info;
		log_info << "step: " << step_count;

		double time_step = 1000 * ((double)getTickCount() - this->time_start) / getTickFrequency();
		log_info << fixed << setprecision(0) << ", time: " << time_step << "ms";

		PROCESS_MEMORY_COUNTERS memCounter;
		BOOL result = K32GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
		log_info << ", mem: " << memCounter.WorkingSetSize / 1048576 << "MB";

		int num_complete = 0;
		for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
			num_complete += complete[shape].rows;
		// get stuck longer than 0.5T will terminate the process
		double cur_progress = 100.0 * num_complete / num_tag;
		if (cur_progress <= progress)
		{
			double time_stuck = 1000 * ((double)getTickCount() - time_first_stuck) / getTickFrequency();
			if (cur_progress > 0 && time_stuck > 60000 && time_stuck > 0.5 * time_step)
			{
				std::cout << "\nget stuck!" << endl;
				return;
			}
		}
		else
		{
			time_first_stuck = (double)getTickCount();
		}
		progress = max(progress, cur_progress);
		log_info << fixed << setprecision(2) << ", progress: " << progress << "%";

		// write log file and display on console
		this->log << step_count << " " << time_step << " " << progress << " " << memCounter.WorkingSetSize << endl;
		cout << log_info.str() << "\r";

		
		if (time_step > this->max_ms || step_count >= this->max_trial)
		{
			cout << "\ntime out!" << endl;
			return;
		}

		// paper: if conflict happens, then
		if (has_conflict(state_byTag, complete))
		{
			do
			{
				if (d == 0)	throw __FUNCTION__ + string(", ") + ("can not generate such a marker field, conflict inevitable! \n");
				d--;
				this->field(Point(tree[d][0][0], tree[d][0][1])) = 2;	// reset the last assignment, which is wrong
				tree[d].erase(tree[d].begin());							// remove the wrong assignment
			} while (tree[d].empty());	// paper: do until non-empty
		}
		else
		{
			if (countNonZero(this->field == 2) == 0)	break;
			Point focus = DOF(state_byTag, incomplete, incomplete_info);
			this->tree[d] = vector<Vec3i>{ Vec3i(focus.x, focus.y, 0), Vec3i(focus.x, focus.y, 1) };
		}
		this->field(Point(tree[d][0][0], tree[d][0][1])) = tree[d][0][2];	// always apply the first suggestion
		d++;

		step_count++;
		if (show_process) {
			show();
			waitKey(1);
		}
	}
	cout << "\ncomplete!" << endl;
}

void generator_HydraMarker::generate_GENETIC(const bool show_process, const string path)
{
	if (countNonZero(this->field == 2) != this->field.total())	throw __FUNCTION__ + string(", ") + ("GENETIC method only accept full empty marker field as input! \n");
	if (this->tag_shape.size() != 1)		throw __FUNCTION__ + string(", ") + ("GENETIC method only accept single tag shape! \n");
	if (this->tag_shape[0].rows != this->tag_shape[0].cols)	throw __FUNCTION__ + string(", ") + ("GENETIC method only accept square tag shape! \n");
	if (countNonZero(this->tag_shape[0] == 0))	throw __FUNCTION__ + string(", ") + ("GENETIC method only accept full solid tag shape! \n");
	if (this->field.rows != this->field.cols)	throw __FUNCTION__ + string(", ") + ("GENETIC method only accept square field! (this is not the limitation of the reference method, but is for the convience of experiment) \n");
	
	initial();
	int order = this->tag_shape[0].rows;

	// initial population
	vector<Mat1b> P;
	for (size_t i = 0; i < 1000; i++)
	{
		Mat1b temp(this->field.size());
		randu(temp, Scalar(0), Scalar(2));
		P.push_back(temp);
	}

	// start!
	int step = 1;
	srand(time(0));
	double progress = 0;
	while (true)
	{
		// calc fitness of each individual
		Mat1f S = Mat1f::zeros(1000, 1);
		Mat1f ratio = Mat1f::zeros(1000, 1);
		for (size_t i = 0; i < 1000; i++)
		{
			Mat1b C_Region;
			S(i) = calc_conflict(P[i], C_Region);
			// dilate(C_Region, C_Region, Mat1b::ones(order, order));
			float good_state = this->field.total() - sum(C_Region)(0);
			ratio(i) = good_state / this->field.total();
		}
		
		// rank the population
		Mat1i sort_idx;
		cv::sortIdx(S, sort_idx, SORT_EVERY_COLUMN | SORT_ASCENDING);
		this->field = P[sort_idx(0)];

		// update log info
		ostringstream log_info;
		log_info << "step: " << step;

		double time_step = 1000 * ((double)getTickCount() - this->time_start) / getTickFrequency();
		log_info << fixed << setprecision(0) << ", time: " << time_step << "ms";

		PROCESS_MEMORY_COUNTERS memCounter;
		BOOL result = K32GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
		log_info << ", mem: " << memCounter.WorkingSetSize / 1048576 << "MB";

		// get stuck longer than 0.5T will terminate the process
		double cur_progress = 100.0 * ratio(sort_idx(0));
		progress = max(progress, cur_progress);
		log_info << fixed << setprecision(2) << ", progress: " << progress << "%";

		// write log file and display on console
		this->log << step << " " << time_step << " " << progress << " " << memCounter.WorkingSetSize << endl;
		cout << log_info.str() << "\r";

		if (show_process) {
			show();
			waitKey(1);
		}

		if (S(sort_idx(0)) == 0)	break;
		if (time_step > this->max_ms)
		{
			cout << "\ntime/step out!" << endl;
			return;
		}
		// rank select
		vector<Mat1b> P_parent;
		for (int i = 0; i < 1000; i++)
			if ((rand() % max(1, i)) == 0)	// at least two individuals
				P_parent.push_back(P[sort_idx(i)]);

		// one-point Crossover
		vector<Mat1b> P_child;
		for (size_t i = 0; i < 1000; i++)
		{
			int A = rand() % P_parent.size();
			int B = rand() % P_parent.size();
			int p = rand() % (this->field.rows - 1) + 1;

			Mat1b child = Mat1b::zeros(this->field.size());
			P_parent[A].rowRange(0, p).copyTo(child.rowRange(0, p));
			P_parent[B].rowRange(p, this->field.rows).copyTo(child.rowRange(p, this->field.rows));
			P_child.push_back(child);
		}

		// mutation
		bool complete_flag = false;
		for (size_t i = 0; i < 1000; i++)
		{
			Mat1b C_Region;
			int c_num = calc_conflict(P_child[i], C_Region);
			if (c_num == 0)	break;
			erode(C_Region, C_Region, getStructuringElement(MORPH_RECT, Size(order, order)), Point(-1, -1), 1, 0, 0);


			int num_1 = countNonZero(C_Region);
			int num_0 = C_Region.total() - num_1;

			Mat1i c_idx;
			cv::sortIdx(C_Region.reshape(1, 1), c_idx, SORT_EVERY_ROW | SORT_DESCENDING);
			int p = rand() % 100;
			int mutate_idx;
	
			if (p < 95)	// mutate conflict region
			{
				p = rand() % num_1;
				mutate_idx = c_idx(p);
			}
			else	// mutate non-conflict region
			{
				p = rand() % num_0;
				mutate_idx = c_idx(num_1 + p);
			}

			// mutate the selected window
			Mat1b mask = Mat1b::zeros(this->field.size());
			mask(mutate_idx) = 1;
			dilate(mask, mask, Mat1b::ones(order, order));
			vector<Point> mutate_loc;
			findNonZero(mask, mutate_loc);

			for (Point pt : mutate_loc)
				P_child[i](pt) = rand() % 2;
		}

		// next round
		swap(P, P_child);
		step++;
	}
	cout << "\ncomplete!" << endl;
}

void generator_HydraMarker::initial()
{
	// reset tree
	this->d = 0;
	this->tree.clear();
	for (size_t i = 0; i < this->field.rows * this->field.cols; i++)
		this->tree.push_back(vector<Vec3i>{});

	build_table();
	build_pool();
}

bool generator_HydraMarker::has_conflict(const vector<Mat1b>& state_byTag, const vector<Mat1b>& complete)
{
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		if (complete[shape].empty())	continue;

		// calc key of each complete tag
		Mat1f key = Mat1f::zeros(complete[shape].rows, 1);
		for (size_t in = 0; in < complete[shape].cols; in++)
			key += pow(2.0, in) * (Mat1f)complete[shape].col(in);

		// sort key to check dumplicate tags
		Mat1f key_sort;
		cv::sort(key, key_sort, SORT_EVERY_COLUMN | SORT_ASCENDING);
		for (size_t im = 0; im < key_sort.rows - 1; im++)
			if (key_sort(im) == key_sort(im + 1))	
				return true;
	}
	return false;
}

Point generator_HydraMarker::DOF(const vector<Mat1b>& state_byTag, const vector<Mat1b>& incomplete, vector<Mat1i>& freedom)
{
	// select a tag with largest DOF-risk
	int minFreedom = INT_MAX;
	Vec2i minTag;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		if (incomplete[shape].empty())	continue;

		// get the first incomplete tag with least freedom
		double minVal;	Point minLoc;
		minMaxLoc(freedom[shape].col(1), &minVal, NULL, &minLoc, NULL);

		// record the tag with less freedom than global minimum
		if (minVal < minFreedom)
		{
			minFreedom = minVal;
			minTag = Vec2i(shape, freedom[shape](minLoc.y, 0));
		}
	}
	if (minFreedom ==INT_MAX)	return Point(-1, -1);

	// get the XY of the first unknown in the most risky tag
	Mat1b c_state = state_byTag[minTag(0)].row(minTag(1));
	Mat unknown;
	cv::findNonZero(c_state == 2, unknown);
	int X = this->tag2X[minTag(0)](minTag(1), unknown.at<Vec2i>(0)[0]);
	int Y = this->tag2Y[minTag(0)](minTag(1), unknown.at<Vec2i>(0)[0]);

	return Point(X, Y);
}

bool generator_HydraMarker::risk(const vector<Mat1b>& state_byTag, const vector<Mat1b>& complete, const Point f)
{
	// find out all the tags containing f
	vector<vector<Mat1i>> relate_info = this->XY2tag[f];
	vector<Mat1b> relate;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat1b relate_table;
		for (size_t rot = 0; rot < relate_info[shape].size(); rot++)
		{
			Mat1b relate_table_rot(relate_info[shape][rot].rows, state_byTag[shape].cols);
			for (size_t i = 0; i < relate_info[shape][rot].rows; i++)
				state_byTag[shape].row(relate_info[shape][rot](i, 0)).copyTo(relate_table_rot.row(i));
			relate_table.push_back(relate_table_rot);
		}
		relate.push_back(relate_table);
	}

	// calc the risk
	int RISK0 = 0;
	int RISK1 = 0;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		// check all the tags containing f, including complete and relate tags
		Mat1b compare;
		if (relate[shape].empty())	compare = complete[shape];
		else		vconcat(relate[shape], complete[shape], compare);
;
		for (size_t i = 0; i < relate_info[shape][0].rows; i++)
		{
			int f_ind = relate_info[shape][0](i, 1);

			Mat1b f_tag;
			repeat(state_byTag[shape].row(relate_info[shape][0](i, 0)), compare.rows, 1, f_tag);

			Mat1i match_sum;
			Mat1b match;
			
			match = (f_tag == 2 | compare == 2 | f_tag == compare);
			cv::reduce(match, match_sum, 1, REDUCE_SUM);

			Mat1b compare_col; compare.col(f_ind).copyTo(compare_col);
			RISK0 += countNonZero(match_sum == 255 * f_tag.cols & compare_col == 0);
			RISK1 += countNonZero(match_sum == 255 * f_tag.cols & compare_col == 1);
		}
	}

	return RISK0 > RISK1;
}

void generator_HydraMarker::hit(const vector<Mat1b>& state_byTag, vector<Vec3i>& suggest)
{
	// check full hit to update tag pool
	vector<Mat1b> used_tag;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat1b used = Mat1b::zeros(this->tag_pool[shape].rows, 1);
		for (size_t im = 0; im < state_byTag[shape].rows; im++)
		{
			Mat1b tag = state_byTag[shape].row(im);
			Mat1b pool = this->tag_pool[shape];
			Mat1b tag_rep;
			repeat(tag, pool.rows, 1, tag_rep);

			Mat1b f_hit = (tag_rep == pool);

			Mat1i f_hit_sum;
			cv::reduce(f_hit, f_hit_sum, 1, REDUCE_SUM);

			f_hit = (f_hit_sum == 255 * pool.cols);
			used.setTo(1, f_hit > 0);
		}
		Mat1b used_full;
		repeat(used, 1, this->tag_pool[shape].cols, used_full);
		used_tag.push_back(used_full);
	}

	suggest.clear();
	vector<Mat1i> H0, H1;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat1i p_hit0_byTag = Mat1i::zeros(state_byTag[shape].size());
		Mat1i p_hit1_byTag = Mat1i::zeros(state_byTag[shape].size());
		for (size_t im = 0; im < state_byTag[shape].rows; im++)
		{
			Mat1b tag = state_byTag[shape].row(im);
			Mat1b pool = this->tag_pool[shape];
			Mat1b tag_rep;
			repeat(tag, pool.rows, 1, tag_rep);
			
			Mat1b p_hit = (used_tag[shape] == 0 & (tag_rep == 2 | tag_rep == pool));	// partial hit - unknown states can fit everything

			Mat1i p_hit_sum;
			cv::reduce(p_hit, p_hit_sum, 1, REDUCE_SUM);

			p_hit = (p_hit_sum == 255 * pool.cols);

			vector<Point> p_hit_loc;
			findNonZero(p_hit, p_hit_loc);
			Mat1b p_hit_pool(p_hit_loc.size(), pool.cols);
			for (size_t i = 0; i < p_hit_loc.size(); i++)
				pool.row(p_hit_loc[i].y).copyTo(p_hit_pool.row(i));

			if (p_hit_pool.empty())	continue;

			Mat1i p_hit0, p_hit1;
			cv::reduce(p_hit_pool, p_hit1, 0, REDUCE_SUM);
			p_hit0 = p_hit_pool.rows - p_hit1;

			p_hit0.copyTo(p_hit0_byTag.row(im));
			p_hit1.copyTo(p_hit1_byTag.row(im));

		}

		H0.push_back(p_hit0_byTag);
		H1.push_back(p_hit1_byTag);
	}

	// calc L and C
	Mat1i C(this->field.size(), INT_MAX);
	Mat1f P0(this->field.size(), 1);
	Mat1f P1(this->field.size(), 1);
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		int block_size = state_byTag[shape].rows / this->rot_prop[shape];
		for (size_t im = 0; im < block_size; im++)
		{
			Mat1i N0(this->field.size(), INT_MAX);
			Mat1i N1(this->field.size(), INT_MAX);
			for (size_t rot = 0; rot < this->rot_prop[shape]; rot++)
			{
				for (size_t in = 0; in < H0[shape].cols; in++)
				{
					int X = this->tag2X[shape](im + rot * block_size, in);
					int Y = this->tag2Y[shape](im + rot * block_size, in);
					if (N0(Point(X, Y)) == INT_MAX)	N0(Point(X, Y)) = 0;
					if (N1(Point(X, Y)) == INT_MAX)	N1(Point(X, Y)) = 0;
					N0(Point(X, Y)) += H0[shape](im + rot * block_size, in);
					N1(Point(X, Y)) += H1[shape](im + rot * block_size, in);
				}
			}
			C = min(C, N0);
			C = min(C, N1);
			
			Mat1f Nsum = Mat1f(N0) + Mat1f(N1);
			if (countNonZero(Nsum <= 0 & this->field == 2))
			{
				return;
			}

			P0 = P0.mul(Mat1f(N0) / Nsum);
			P1 = P1.mul(Mat1f(N1) / Nsum);
			
			Mat1f Psum = P0 + P1;

			Mat1f P0temp = P0 / (P0 + P1);
			Mat1f P1temp = P1 / (P0 + P1);
			P0 = P0temp;
			P1 = P1temp;

		}
	}

	P0.setTo(10, this->field != 2);
	P1.setTo(10, this->field != 2);

	if (countNonZero(P0 == P0)!=P0.total())
	{
		return;
	}
	if (countNonZero(P1 == P1)!=P1.total()) return;

	// suggest
	C.setTo(INT_MAX, this->field <= 1);
	double minVal;
	minMaxLoc(C, &minVal, NULL, NULL, NULL);

	vector<Point> minLoc;
	findNonZero(C == minVal, minLoc);
	Point pt = minLoc[0];
	if (P0(pt) == 0 && P1(pt) == 0)		return;
	else if (P0(pt) == 0)	suggest.push_back(Vec3i(pt.x, pt.y, 1));
	else if (P1(pt) == 0)	suggest.push_back(Vec3i(pt.x, pt.y, 0));
	else
	{
		suggest.push_back(Vec3i(pt.x, pt.y, P0(pt) < P1(pt)));
		suggest.push_back(Vec3i(pt.x, pt.y, P0(pt) >= P1(pt)));
	}

}

void generator_HydraMarker::build_table()
{
	// reset tables
	this->rot_prop.clear();
	this->tag2X.clear();
	this->tag2Y.clear();
	this->XY2tag.clear();

	// investigate the rot_prop of each shape
	for (Mat1b mat : this->tag_shape)
	{
		Mat1b mat_rot;

		rotate(mat, mat_rot, ROTATE_90_CLOCKWISE);
		if (mat.size() == mat_rot.size() && countNonZero(mat != mat_rot) == 0)	// 90 degree repeat
		{
			this->rot_prop.push_back(4);
			continue;
		}	

		rotate(mat_rot, mat_rot, ROTATE_90_CLOCKWISE);
		if (mat.size() == mat_rot.size() && countNonZero(mat != mat_rot) == 0)	// 180 degree repeat
		{
			this->rot_prop.push_back(2);
			continue;
		}

		this->rot_prop.push_back(1);	// non-repeat
	}

	// build tag2X, tag2Y
	for (Mat1b mat : this->tag_shape)
	{
		Mat1i Xtable, Ytable;

		// get the XY of the states hit by current tag shape on top-left
		Mat1i X, Y;
		meshgrid(Range(0, mat.cols), Range(0, mat.rows), X, Y);
		X = X.setTo(-1, mat != 1).reshape(1, 1);
		Y = Y.setTo(-1, mat != 1).reshape(1, 1);

		// remove hollows of tag shape
		vector<int> Xvec(X), Yvec(Y);
		vector<int>::iterator iterX = Xvec.begin();
		vector<int>::iterator iterY = Yvec.begin();
		for (; iterX < Xvec.end();)
		{
			if (*iterX == -1)
			{
				iterX = Xvec.erase(iterX);
				iterY = Yvec.erase(iterY);
				continue;
			}
			iterX++;
			iterY++;
		}
		X = Mat1i(Xvec).reshape(1, 1);
		Y = Mat1i(Yvec).reshape(1, 1);

		// rotate the tag shape by 4 orientations
		for (size_t rot = 0; rot < 4; rot++)
		{
			// get the XY from rotated view
			double minX, minY, maxX, maxY;
			if (rot > 0)
			{
				swap(X, Y);
				minMaxLoc(X, &minX, &maxX, NULL, NULL);
				minMaxLoc(Y, &minY, &maxY, NULL, NULL);
				X = -X + minX + maxX;
			}
			else
			{
				minMaxLoc(X, &minX, &maxX, NULL, NULL);
				minMaxLoc(Y, &minY, &maxY, NULL, NULL);
			}

			Mat1i ind = Y * this->field.cols + X;

			// for each orientation, scan the marker field
			for (int y = -minY; y < this->field.rows - maxY; y++)
			{
				for (int x = -minX; x < this->field.cols - maxX; x++)
				{
					// ignore tags containing hollows
					Mat c_state;
					remap(this->field, c_state, Mat1f(X + x), Mat1f(Y + y), INTER_NEAREST);
					if (countNonZero(c_state == 3) <= 0)
					{
						Xtable.push_back(X + x);
						Ytable.push_back(Y + y);
					}
				}
			}
		}
		// push the table for current tag shape into tag2ind
		this->tag2X.push_back(Xtable); 
		this->tag2Y.push_back(Ytable);
	}

	// build map1 and map2 for fast read
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat t_map1, t_map2;
		convertMaps(Mat1f(this->tag2X[shape]), Mat1f(this->tag2Y[shape]), t_map1, t_map2, CV_16SC2, true);
		this->tagMap.push_back(t_map1);
	}

	// allocate XY2tag
	vector<vector<Mat1i>> vec_empty;
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		if (this->rot_prop[shape] == 4)	vec_empty.push_back(vector<Mat1i>{Mat1i(), Mat1i(), Mat1i(), Mat1i()});
		if (this->rot_prop[shape] == 2)	vec_empty.push_back(vector<Mat1i>{Mat1i(), Mat1i()});
		if (this->rot_prop[shape] == 1)	vec_empty.push_back(vector<Mat1i>{Mat1i()});
	}
	for (size_t ix = 0; ix < this->field.cols; ix++)
		for (size_t iy = 0; iy < this->field.rows; iy++)
			this->XY2tag.insert(pair<Point, vector<vector<Mat1i>>>(Point(ix, iy), vec_empty));

	// build XY2tag
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat1i Xtable = this->tag2X[shape];
		Mat1i Ytable = this->tag2Y[shape];
		
		// restore to different orientations based on rot_prop
		for (size_t rot = 0; rot < this->rot_prop[shape]; rot++)
		{
			for (size_t im = rot * Xtable.rows / this->rot_prop[shape]; im < (rot + 1) * Xtable.rows / this->rot_prop[shape]; im++)
			{
				Mat1i Xrow = Xtable.row(im);
				Mat1i Yrow = Ytable.row(im);
				for (size_t in = 0; in < Xrow.cols; in++)
				{
					// restore the shape and row of every XY in tag2XY
					int x = Xrow(0, in);
					int y = Yrow(0, in);
					Mat1i info = (Mat1i(1, 2) << (int)im, (int)in);
					XY2tag[Point(x, y)][shape][rot].push_back(info);
				}
			}
		}
	}
}

void generator_HydraMarker::build_pool()
{
	this->tag_pool.clear();
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		Mat1b mat = this->tag_shape[shape];

		Mat1i X, Y;
		meshgrid(Range(0, mat.cols), Range(0, mat.rows), X, Y);
		X = X.setTo(-1, mat != 1).reshape(1, 1);
		Y = Y.setTo(-1, mat != 1).reshape(1, 1);

		// remove hollows of tag shape
		vector<int> Xvec(X), Yvec(Y);
		vector<int>::iterator iterX = Xvec.begin();
		vector<int>::iterator iterY = Yvec.begin();
		for (; iterX < Xvec.end();)
		{
			if (*iterX == -1)
			{
				iterX = Xvec.erase(iterX);
				iterY = Yvec.erase(iterY);
				continue;
			}
			iterX++;
			iterY++;
		}
		X = Mat1i(Xvec).reshape(1, 1);
		Y = Mat1i(Yvec).reshape(1, 1);

		// transfer to index
		Mat1i index = Y * mat.cols + X;
		if (this->rot_prop[shape] == 4)
		{
			double minX, maxX;	
			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL); 
			X = -X + minX + maxX;
			index.push_back(Y * mat.cols + X);
			
			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			X = -X + minX + maxX;
			index.push_back(Y * mat.cols + X);

			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			X = -X + minX + maxX;
			index.push_back(Y * mat.cols + X);
		}
		if (this->rot_prop[shape] == 2)
		{
			double minX, maxX;
			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			X = -X + minX + maxX;
			swap(X, Y);
			minMaxLoc(X, &minX, &maxX, NULL, NULL);
			X = -X + minX + maxX;
			index.push_back(Y * mat.cols + X);
		}
		for (size_t im = 0; im < index.rows; im++)
		{
			Mat1i ind;
			cv::sortIdx(index.row(im), ind, SORT_EVERY_ROW | SORT_ASCENDING);
			for (size_t i = 0; i < ind.cols; i++)
				index(im, ind(i)) = i;
		}

		// initial tag pool
		Mat1i pool(pow(2, index.cols), index.cols, 0);
		for (size_t im = 1; im < pool.rows; im++)
		{
			int dec = im, in = 0;
			while (dec != 0) 
			{ 
				pool(im, in++) = (dec % 2 == 0 ? 0 : 1);
				dec /= 2; 
			}
		}
		if (this->rot_prop[shape] == 1)	// non-repeat tag shape has no conflict
		{
			this->tag_pool.push_back(pool);
			continue;
		}

		// remove self-conflict
		Mat1i self_conflict;
		Mat1i pool_all;
		if (this->rot_prop[shape] == 4)
		{
			Mat1i pool_rot1(pool.size());
			Mat1i pool_rot2(pool.size());
			Mat1i pool_rot3(pool.size());
			for (size_t in = 0; in < index.cols; in++)
			{ 
				pool.col(index(1, in)).copyTo(pool_rot1.col(in));
				pool.col(index(2, in)).copyTo(pool_rot2.col(in));
				pool.col(index(3, in)).copyTo(pool_rot3.col(in));
			}
				
			Mat1i match_sum1;
			Mat1i match_sum2;
			Mat1i match_sum3;
			cv::reduce(pool == pool_rot1, match_sum1, 1, REDUCE_SUM);
			cv::reduce(pool == pool_rot2, match_sum2, 1, REDUCE_SUM);
			cv::reduce(pool == pool_rot3, match_sum3, 1, REDUCE_SUM);
			
			self_conflict = ((match_sum1 == 255 * pool.cols) | (match_sum2 == 255 * pool.cols) | (match_sum3 == 255 * pool.cols));
			
			vector<Mat1i> pool_ori{pool, pool_rot1, pool_rot2, pool_rot3};
			hconcat(pool_ori, pool_all);
		}
		if (this->rot_prop[shape] == 2)
		{
			Mat1i pool_rot(pool.size());
			for (size_t in = 0; in < index.cols; in++)
				pool.col(index(1, in)).copyTo(pool_rot.col(in));

			Mat1i match_sum;
			cv::reduce(pool == pool_rot, match_sum, 1, REDUCE_SUM);

			self_conflict = (match_sum == 255 * pool.cols);
			vector<Mat1i> pool_ori{ pool, pool_rot };
			hconcat(pool_ori, pool_all);
		}
		Mat1i pool2;
		for (size_t im = 0; im < pool_all.rows; im++)
			if (self_conflict(im) == 0)
				pool2.push_back(pool_all.row(im));

		if (this->rot_prop[shape] == 4)	pool2 = pool2.reshape(1, pool2.rows * 4);
		if (this->rot_prop[shape] == 2)	pool2 = pool2.reshape(1, pool2.rows * 2);
		
		// remove cross-conflict
		Mat1f key = Mat1f::zeros(pool2.rows, 1);
		for (size_t in = 0; in < pool2.cols; in++)
			key += pow(2.0, in) * (Mat1f)pool2.col(in);

		// sort key to check dumplicate tags
		Mat1i key_ind;
		Mat1b key_conflict = Mat1b::zeros(key.size());
		cv::sortIdx(key, key_ind, SORT_EVERY_COLUMN | SORT_ASCENDING);
		for (size_t a = 0; a < key_ind.rows - 1; a++)	// select the one with smallest index from cross-conflict tags 
		{
			int index_min = key_ind(a);
			size_t b = a + 1;
			while (key(key_ind(a)) == key(key_ind(b)))
			{
				if (key_ind(b) > index_min)
				{
					key_conflict(key_ind(b)) = 1;
				}
				else
				{
					key_conflict(index_min) = 1;
					index_min = key_ind(b);
				}
				b++;
				if (b >= key_ind.rows)	break;
			}
			a = b - 1;
		}

		Mat1i pool_legal;
		for (size_t i = 0; i < key_conflict.rows; i += this->rot_prop[shape])
		{
			if (key_conflict(i) == 0)
				pool_legal.push_back(pool2.row(i));
		}

		this->tag_pool.push_back(pool_legal);
	}
}

void generator_HydraMarker::read_state(vector<Mat1b>& state_byTag)
{
	int maxnum = 32766;	// the maximum side length of opencv remap function is 32767
	state_byTag.clear();
	for (size_t shape = 0; shape < this->tag_shape.size(); shape++)
	{
		int block_num = this->tagMap[shape].rows / maxnum + 1;
		Mat1b c_state;
		for (int im = 0; im < block_num; im++)
		{
			//Mat1b a_state;
			//remap(this->field, a_state,
			//	Mat1f(this->tag2X[shape].rowRange(im * maxnum, min((im + 1) * maxnum, this->tag2X[shape].rows))),
			//	Mat1f(this->tag2Y[shape].rowRange(im * maxnum, min((im + 1) * maxnum, this->tag2Y[shape].rows))),
			//	INTER_NEAREST);

			Mat1b b_state;
			remap(this->field, b_state,
				this->tagMap[shape].rowRange(im * maxnum, min((im + 1) * maxnum, this->tagMap[shape].rows)),
				Mat(),
				INTER_NEAREST);
			c_state.push_back(b_state);
		}
		state_byTag.push_back(c_state);
	}
}

void generator_HydraMarker::get_complete(const vector<Mat1b>& state_byTag, vector<Mat1b>& complete)
{
	complete.clear();
	for (Mat1b state_table : state_byTag)
	{
		// find completed tags 
		Mat1i freedom;
		cv::reduce(state_table == 2, freedom, 1, REDUCE_SUM);
		vector<Point> complete_loc;
		cv::findNonZero(freedom == 0, complete_loc);

		// filter our completed tags
		Mat1b complete_tags(complete_loc.size(), state_table.cols);
		for (size_t im = 0; im < complete_loc.size(); im++)
			state_table.row(complete_loc[im].y).copyTo(complete_tags.row(im));

		complete.push_back(complete_tags);
	}
}

void generator_HydraMarker::get_incomplete(const vector<Mat1b>& state_byTag, vector<Mat1b>& incomplete, vector<Mat1i>& info)
{
	incomplete.clear();
	info.clear();
	for (Mat1b state_table : state_byTag)
	{
		// find completed tags 
		Mat1i unknown_num;
		reduce(state_table == 2, unknown_num, 1, REDUCE_SUM);
		vector<Point> incomplete_loc;
		findNonZero(unknown_num > 0, incomplete_loc);

		// filter our completed tags
		Mat1b incomplete_tags(incomplete_loc.size(), state_table.cols);
		Mat1i Finfo(incomplete_loc.size(), 2);
		for (size_t im = 0; im < incomplete_loc.size(); im++)
		{
			state_table.row(incomplete_loc[im].y).copyTo(incomplete_tags.row(im));
			Finfo(im, 0) = incomplete_loc[im].y;
			Finfo(im, 1) = unknown_num(incomplete_loc[im].y)/255;
		}

		incomplete.push_back(incomplete_tags);
		info.push_back(Finfo);
	}
}

void generator_HydraMarker::meshgrid(const Range xgv, const Range ygv, Mat1i& X, Mat1i& Y)
{
	Mat1i tx(1, xgv.end - xgv.start);
	Mat1i ty(ygv.end - ygv.start, 1);
	for (int t = 0, i = xgv.start; i < xgv.end; t++, i++) tx(t) = i;
	for (int t = 0, i = ygv.start; i < ygv.end; t++, i++) ty(t) = i;
		
	repeat(tx, ty.rows, 1, X);
	repeat(ty, 1, tx.cols, Y);
}

int generator_HydraMarker::calc_conflict(const Mat1b& field, Mat1b& conflict_region)
{
	int Size = field.rows;
	int order = this->tag_shape[0].rows;
	int conflict_count = 0;

	Mat1b field_rot;
	field.copyTo(field_rot);
	conflict_region = Mat1b::zeros(field.size());

	// read state
	int maxnum = 32766;	// the maximum side length of opencv remap function is 32767
	Mat1b state_tag;
	int block_num = this->tagMap[0].rows / maxnum + 1;
	for (int im = 0; im < block_num; im++)
	{
		Mat1b b_state;
		remap(field, b_state,
			this->tagMap[0].rowRange(im * maxnum, min((im + 1) * maxnum, this->tagMap[0].rows)),
			Mat(),
			INTER_NEAREST);
		state_tag.push_back(b_state);
	}

	// calc key of each tag
	Mat1f key = Mat1f::zeros(state_tag.rows, 1);
	for (size_t in = 0; in < state_tag.cols; in++)
		key += pow(2.0, in) * (Mat1f)state_tag.col(in);

	// sort key to check dumplicate tags
	Mat1i idx;
	cv::sortIdx(key, idx, SORT_EVERY_COLUMN | SORT_ASCENDING);
	for (size_t im = 0; im < idx.rows - 1; im++)
		if (key(idx(im)) == key(idx(im + 1)))
		{
			for (size_t in = 0; in < state_tag.cols; in++)
			{
				conflict_region(Point(this->tag2X[0](idx(im), in), this->tag2Y[0](idx(im), in))) = 1;
				conflict_region(Point(this->tag2X[0](idx(im + 1), in), this->tag2Y[0](idx(im + 1), in))) = 1;
			}
		}



	return countNonZero(conflict_region);
	//for (size_t rot = 0; rot < 4; rot++)
	//{
	//	rotate(field_rot, field_rot, ROTATE_90_CLOCKWISE);
	//	for (int ix = order - Size; ix < Size - order; ix++)
	//	{
	//		for (int iy = order - Size; iy < Size - order; iy++)
	//		{
	//			if (rot == 3 && ix == 0 && iy == 0)	continue;	// do not compare with self

	//			Mat trans_mat = (Mat_<double>(2, 3) << 1, 0, ix, 0, 1, iy);
	//			Mat1b field_shifted;
	//			warpAffine(field_rot, field_shifted, trans_mat, field.size(), INTER_LINEAR, BORDER_CONSTANT, 2);
	//			Mat1b hit = (field == field_shifted);
	//			Mat1b hit_block;
	//			erode(hit, hit_block, Mat1b::ones(order, order), Point(-1,-1), 1, 0, 0);
	//			int hit_num = countNonZero(hit_block);
	//			conflict_count += hit_num;

	//			// record conflict region
	//			conflict_region.setTo(1, hit_block > 0);
	//		}
	//	}
	//}

	//return conflict_count;
}
