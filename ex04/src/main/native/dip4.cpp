//============================================================================
// Name        : dip4.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, degrade image,
//               and apply inverse as well as wiener filter
//============================================================================

#include <iostream>
#include <opencv2/opencv.hpp>

// function headers of not yet implemented functions
#include "dip4.hpp"


Mat inverseFilter(Mat& degraded, Mat& filter) {

	// TODO
}

Mat wienerFilter(Mat& degraded, Mat& filter, double snr) {

	// TODO
}

void createKernel(Mat& kernel, int kSize, string name) {

	// TODO
}

void circShift(Mat& in, Mat& out, int dx, int dy) {

	// TODO copy from ex03
}
