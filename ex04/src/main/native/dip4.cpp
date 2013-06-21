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

	const float epsilon = 0.09f;

	const Mat& s = degraded;
	const Mat& p = filter;
	const int kw = p.cols;
	const int kh = p.rows;

	Mat pBig = Mat::zeros(s.size(), s.type());
	for (int x = 0; x < kw; ++x) {
		for (int y = 0; y < kh; ++y) {
			pBig.at<float>(x, y) = p.at<float>(x, y);
		}
	}
	Mat pBigShifted(s.size(), s.type());
	circShift(pBig, pBigShifted, -(kw/2), -(kh/2));

	//	TODO;
	//	add imaginary part/channel to the pBigShifted matrix,
	//			to get complex putput from the DFT
	Mat empty = Mat::zeros(pBigShifted.size(), pBigShifted.type());
	vector<Mat> pBigShiftedChannels;
	pBigShiftedChannels.push_back(pBigShifted);
	pBigShiftedChannels.push_back(empty);
	Mat pBigShiftedCompl(pBigShifted.size(), CV_32FC2);
	merge(pBigShiftedChannels, pBigShiftedCompl);

	// transform into frequency domain
	Mat S(s.size(), CV_32FC2); // complex
	Mat P(pBigShifted.size(), CV_32FC2); // complex
//	dft(s, S, DFT_COMPLEX_OUTPUT);
//	dft(pBigShifted, P, DFT_COMPLEX_OUTPUT);
	dft(s, S);
	dft(pBigShifted, P);

	const int w = P.rows;
	const int h = P.cols;
	float PiAbsMax = 0.0f;
	Mat Pabs(P.size(), CV_32FC1); // abs(P)
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			Vec2f Pi = P.at<Vec2f>(x, y);
			float PiAbs = sqrt((Pi[0] * Pi[0]) + (Pi[1] * Pi[1]));
			if (PiAbs > PiAbsMax) {
				PiAbsMax = PiAbs;
			}
			Pabs.at<float>(x, y) = PiAbs;
		}
	}

	// create the inverse filter
	// see DIP04_ST13_wiener.pdf page 12
	const float epsMaxAbsP = epsilon * PiAbsMax;

	Mat Preciproce(P.size(), CV_32FC2);
//	divide(1.0, P, Preciproce); // NOTE this would not do correct complex division
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			const Vec2f& Pi = P.at<Vec2f>(x, y);
			const float a2Nb2 = (Pi[0] * Pi[0]) + (Pi[1] * Pi[1]);
			Preciproce.at<Vec2f>(x, y)[0] =  Pi[0] / a2Nb2;
			Preciproce.at<Vec2f>(x, y)[1] = -Pi[1] / a2Nb2;
		}
	}

	Mat Q(Pabs.size(), CV_32FC2);
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			const float PiAbs = Pabs.at<float>(x, y);
			Vec2f Qi(0.0f, 0.0f);
			if (PiAbs >= epsMaxAbsP) {
				Qi = Preciproce.at<Vec2f>(x, y);
			} else {
				Qi[0] = 1.0f / epsMaxAbsP;
			}
			Q.at<Vec2f>(x, y) = Qi;
		}
	}

	// multiply with the inverse model of the distortion
	// (== convolute in spacial domain)
	Mat O_approx(P.size(), P.type());
	cout << "S: " << S.size[0] << "*" << S.size[1] << endl;
	cout << "Q: " << Q.size[0] << "*" << Q.size[1] << endl;
	multiply(S, Q, O_approx);

	// transform back into spatial domain
	Mat o_approx(O_approx.size(), CV_32FC1);
	dft(O_approx, o_approx, DFT_INVERSE| DFT_SCALE | DFT_REAL_OUTPUT);

	return o_approx;
}

Mat wienerFilter(Mat& degraded, Mat& filter, double snr) {

	// TODO
	return degraded;
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
