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

/*
Generates a gaussian filter kernel of given size
kSize:		kernel size (used to calculate standard deviation)
return:		the generated filter kernel
*/
static void createGaussianKernel(Mat& kernel, int kSize) {

//	kernel = Mat::zeros(kSize, kSize, CV_32FC1);

	const int ksh = kSize / 2;
	const int muX = ksh;
	const int muY = ksh;
	const float sigma = kSize / 5.0f;
	const float sigmaSqr = sigma * sigma;
	const float gaussMul = 1.0f / (2 * M_PI * sigma * sigma);

	float integral = 0.0f;
	for (int x = 0; x < kSize; ++x) {
		for (int y = 0; y < kSize; ++y) {
			// calculate gaussian coordinates
			const int gX = x - muX;
			const int gY = y - muY;
			// calculate gaussian value
			const float curVal = gaussMul * exp(-0.5f * ((gX*gX + gY*gY) / sigmaSqr));
			// store it
			kernel.at<float>(x, y) = curVal;
			integral += curVal;
		}
	}
	cout << "Kernel integral (should be ~= 1.0f): " << integral << endl;
}

void createKernel(Mat& kernel, int kSize, string name) {

	if (name == "gaussian") {
		createGaussianKernel(kernel, kSize);
	} else {
		// TODO
	}
}

void circShift(Mat& in, Mat& out, int dx, int dy) {

	const int h = in.rows;
	const int w = in.cols;

//	out = Mat::zeros(h, w, in.type());

	for (int y = 0; y < h; ++y) {
		int yNew = y + dy;
		if (yNew < 0) {
			yNew = yNew + h;
		} else if (yNew >= h) {
			yNew = yNew - h;
		}

		for (int x = 0; x < w; ++x) {
			int xNew = x + dx;
			if (xNew < 0) {
				xNew = xNew + w;
			} else if (xNew >= w) {
				xNew = xNew - w;
			}

			out.at<float>(yNew, xNew) = in.at<float>(y, x);
		}
	}
}
