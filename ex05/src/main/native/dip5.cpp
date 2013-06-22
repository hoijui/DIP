// =============================================================================
// Name        : dip5.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : Loads image, calculates structure tensor,
//               defines and plots interest points
// =============================================================================

#define _USE_MATH_DEFINES

#include <iostream>
#include <opencv2/opencv.hpp>

#include "dip5.hpp"

void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points) {

	// TODO
}

void createFstDevKernel(Mat& kernel, double sigma) {

	const int w = kernel.rows;
	const int h = kernel.cols;
	const float sigma22 = 2 * sigma * sigma;
	const float sigma4Pi = 2 * M_PI * sigma * sigma * sigma * sigma;
	const float hw = (w / 2) + 0.5f;
	const float hh = (h / 2) + 0.5f;
	for (int x = 0; x < w; ++x) {
		const float xv = x - hw;
		for (int y = 0; y < h; ++y) {
			const float yv = y - hh;
			const float commonFac = exp((xv*xv + yv*yv) / sigma22) / sigma4Pi;
			// see DIP05_ST13_interest.pdf page 10
			kernel.at<Vec2f>(x, y)[0] = -xv * commonFac; // partial derivative towards x
			kernel.at<Vec2f>(x, y)[1] = -yv * commonFac; // partial derivative towards y
		}
	}
}
