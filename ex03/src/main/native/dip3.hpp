//============================================================================
// Name        : dip3.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, apply unsharp masking
//============================================================================

#ifndef _DIP3_H_
#define _DIP3_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// function headers of not yet implemented functions
Mat createGaussianKernel(int kSize);
Mat circShift(Mat& in, int dx, int dy);
Mat frequencyConvolution(Mat& in, Mat& kernel);
Mat spatialConvolution(Mat& in, Mat& kernel);
Mat usm(Mat& in, int smoothType, int size, double thresh, double scale);
// function headers of given functions
Mat mySmooth(Mat& in, int size, bool spatial);
void cropCoordinate(int& coord, const int lowerBound, const int upperBound);

#endif // _DIP3_H_
