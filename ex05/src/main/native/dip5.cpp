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


static void cropCoordinate(int& coord, const int lowerBound, const int upperBound) {

	if (coord < lowerBound) {
		coord = lowerBound + (lowerBound - coord);
	} else if (coord >= upperBound) {
		coord = upperBound - (coord - upperBound + 1);
	}
}

static Mat spatialConvolution(const  Mat& in, const  Mat& kernel) {

	Mat res = Mat(in.size(), in.type());

	const int w = in.rows;
	const int h = in.cols;
	const int kw = kernel.rows;
	const int kh = kernel.cols;
	const int kwHalf = (int) (kw / 2);
	const int khHalf = (int) (kh / 2);
	for (int x = 0; x < w; ++x) { // image.x
		for (int y = 0; y < h; ++y) { // image.y
			float newVal = 0.0f;
			for (int kx = 0; kx < kw; ++kx) { // kernel.x
				for (int ky = 0; ky < kh; ++ky) { // kernel.y
					int imgX = x - kwHalf + kx;
					cropCoordinate(imgX, 0, w);
					int imgY = y - khHalf + ky;
					cropCoordinate(imgY, 0, h);
					newVal += in.at<float>(imgX, imgY) * kernel.at<float>(kx, ky);
				}
			}
			res.at<float>(x, y) = newVal;
		}
	}

	return res;
}

static void makeProbability(Mat& matrix) {

	const float absSum = cv::sum(cv::sum(abs(matrix)))[0];
	matrix = matrix / absSum;
}

static int gaussSigmaToKernelSize(const double sigma) {

	const int kSize = (((int)ceil(sigma * 3.0)) * 2) + 1;
	return kSize;
}

static Mat createGaussianKernel(Mat& kernel, const double sigma) {

	const int kSizeW = kernel.rows;
	const int kSizeH = kernel.cols;
	const int muX = kSizeW / 2;
	const int muY = kSizeH / 2;
	const float sigmaSqr = sigma * sigma;
	const float gaussMul = 1.0f / (2.0f * M_PI * sigmaSqr);
	float normalizer = -1.0f / (2.0f * sigmaSqr);

	float integral = 0.0f;
	for (int x = 0; x < kSizeW; ++x) {
		for (int y = 0; y < kSizeH; ++y) {
			// calculate gaussian coordinates
			const int gX = x - muX;
			const int gY = y - muY;
			// calculate the gaussian value
			const float curVal = gaussMul * exp((gX*gX + gY*gY) * normalizer);
			// store it
			kernel.at<float>(x, y) = curVal;
			integral += curVal;
		}
	}
	cout << "Kernel integral (should be ~= 1.0f): " << integral << endl;

	// ... just in case it is not, make it so!
	makeProbability(kernel);

	return kernel;
}

static Mat createGaussianKernel(const int kSize) {

	const float sigma = kSize / 5.0f;
	Mat kernel(1, kSize, CV_32FC1);
	createGaussianKernel(kernel, sigma);
	return kernel;
}

static Mat createGaussianKernel(const double sigma) {

	const int kSize = gaussSigmaToKernelSize(sigma);
	Mat kernel(1, kSize, CV_32FC1);
	createGaussianKernel(kernel, sigma);
	return kernel;
}



void createFstDevKernel(Mat& kernel, const double sigma) {

	const int w = kernel.rows;
	const int h = kernel.cols;
	const float sigmaSqr = sigma * sigma;
	const float sigma22 = 2 * sigmaSqr;
	const float sigma4Pi = 2 * M_PI * sigmaSqr * sigmaSqr;
	const float hw = (w / 2);
	const float hh = (h / 2);
	for (int x = 0; x < w; ++x) {
		const float xv = x - hw;
		for (int y = 0; y < h; ++y) {
			const float yv = y - hh;
			const float commonFac = exp(-(xv*xv + yv*yv) / sigma22) / sigma4Pi;

			// see DIP05_ST13_interest.pdf page 10
			if (kernel.type() == CV_32FC2) {
				kernel.at<Vec2f>(x, y)[0] = -xv * commonFac; // partial derivative towards x
				kernel.at<Vec2f>(x, y)[1] = -yv * commonFac; // partial derivative towards y
			} else {
				kernel.at<float>(x, y) = -yv * commonFac; // partial derivative towards y
			}
		}
	}

	makeProbability(kernel);
}

static Mat createFstDevKernel(const double sigma) {

	const int kSize = gaussSigmaToKernelSize(sigma);
	Mat kernel(1, kSize, CV_32FC1);

	createFstDevKernel(kernel, sigma);

	return kernel;
}


void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points) {

	cout << "Interest Points calculation" << endl;

	cout << "initialize stuff" << endl;
	Mat gauss = createGaussianKernel(2*sigma);
	Mat devXK = createFstDevKernel(sigma);
	Mat devYK;
	transpose(devXK, devYK);


	cout << "calculate the x- and y-gradients" << endl;
	Mat gradX = spatialConvolution(img, devXK);
	Mat gradY = spatialConvolution(img, devYK);

	Mat gXX;
	Mat gYY;
	Mat gXY;
	multiply(gradX, gradY, gXY);
	multiply(gradX, gradX, gXX);
	multiply(gradY, gradY, gYY);

	Mat gXXs = spatialConvolution(gXX, gauss);
	Mat gYYs = spatialConvolution(gYY, gauss);
	Mat gXYs = spatialConvolution(gXY, gauss);


	cout << "calculate the trace" << endl;
	// see DIP05_ST13_interest.pdf page 22
	Mat trace;
	add(gXXs, gYY, trace);


	cout << "calculate the determinant" << endl;
	// see DIP05_ST13_interest.pdf page 22
	Mat det;
	Mat temp;
	multiply(gXXs, gYYs, det);
	multiply(gXYs, gXYs, temp);
	subtract(det, temp, det);


	cout << "calculate the weight" << endl;
	// Strength of gradients in the neighborhood
	// see DIP05_ST13_interest.pdf page 22
	Mat w;
	divide(det, trace, w);
	Mat wNoMax;
	nonMaxSuppression(w, wNoMax);


	cout << "calculate the isotropy" << endl;
	// Measures the uniformity of gradient directions in the neighbourhood
	// see DIP05_ST13_interest.pdf page 22
	multiply(trace, trace, temp);
	Mat q;
	divide(4*det, temp, q);
	Mat qNoMax;
	nonMaxSuppression(q, qNoMax);


	cout << "do the thresholding" << endl;
	// XXX why do we have to choose wMin so high?
	const float wMin = 70.0; // 0.5 ... 1.5
	const float qMin = 0.5; // 0.5 ... 0.75
	const float wMinActual = wMin * mean(wNoMax)[0];
	for (int x = 0; x < img.rows; ++x) {
		for (int y = 0; y < img.cols; ++y) {
			if ((qNoMax.at<float>(x, y) >= qMin)
					&& (wNoMax.at<float>(x, y) >= wMinActual))
			{
				KeyPoint kp;
				kp.pt.x = y;
				kp.pt.y = x;
				kp.angle = 0;
				kp.size = 3;
				kp.response = wNoMax.at<float>(x, y);
				points.push_back(kp);
			}
		}
	}
}
