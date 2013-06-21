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

	const float epsilon = 0.05f;

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

	// transform into frequency domain
	Mat S(s.size(), CV_32FC2); // complex
	Mat P(pBigShifted.size(), CV_32FC2); // complex
	dft(s, S, DFT_COMPLEX_OUTPUT);
	dft(pBigShifted, P, DFT_COMPLEX_OUTPUT);
//	dft(s, S);
//	dft(pBigShifted, P);


	const int w = P.rows;
	const int h = P.cols;
//	float PiAbsMax = 0.0f;
	for (int x = 0; x < w; ++x) { // image.x
		for (int y = 0; y < h; ++y) { // image.y
			float* Pi = P.at<float*>(x, y);
			float PiAbs = sqrt((Pi[0] * Pi[0]) + (Pi[1] * Pi[1]));
			Pabs.at<float>(x, y) = PiAbs;
		}
	}
	float PiAbsMax = max(PiAbsMax);



	// create the inverse filter
	// see DIP04_ST13_wiener.pdf page 12
	Mat PAbs = abs(P);
//	const float maxAbsP = max(PAbs);
	double maxAbsP;
//	int maxIdx[3];
	minMaxIdx(PAbs, 0, &maxAbsP, 0, 0);
	const float epsMaxAbsP = epsilon * maxAbsP;

	// TODO FIXME
	Mat Q(PAbs.size(), CV_32FC2);
	Mat Preciproce(P.size(), CV_32FC2);
	Mat PIsBig(PAbs.size(), CV_32FC1);
	Mat PIsSmall(PAbs.size(), CV_32FC1);
	Mat Pthresholded(PAbs.size(), CV_32FC1);
	Mat PthresholdedComp(P.size(), CV_32FC2);
	threshold(PAbs, PIsBig, epsMaxAbsP, 1.0, THRESH_BINARY);
	threshold(PAbs, PIsSmall, epsMaxAbsP, 1.0, THRESH_BINARY_INV);

//	vector<Mat> complexChannelsBig(2);
//	split(PIsBig, complexChannelsBig);
////	PIsBig = complexChannelsBig.at(0);
//	complexChannelsBig.at(1) = complexChannelsBig.at(0);
//	merge(complexChannelsBig, PIsBig);

//	vector<Mat> complexChannelsSmall(2);
//	split(PIsSmall, complexChannelsSmall);
////	PIsSmall = complexChannelsSmall.at(0);
//	complexChannelsSmall.at(1) = complexChannelsSmall.at(0);
//	merge(complexChannelsSmall, PIsSmall);

//	Q = ((1 / P) * PIsBig) + (epsMaxAbsP * PIsSmall);
//	Q = 1.0 / P;
//	Q *= PIsBig;
		divide(1.0, P, Preciproce);
		multiply(Preciproce, PIsBig, Q);
		multiply(epsMaxAbsP, PIsSmall, Pthresholded);
//		vector<Mat> toBeMerged;
//		toBeMerged.push_back(Pthresholded);
//		toBeMerged.push_back(Mat::zeros(P.size(), CV_32FC1));
//		merge(toBeMerged, PthresholdedComp);
		PthresholdedComp = Pthresholded;
//		cout << "PIsSmall: " << PIsSmall << endl;
		cout << "CV_32FC1: " << CV_32FC1 << endl;
		cout << "CV_32FC2: " << CV_32FC2 << endl;
		cout << "PIsSmall: " << PIsSmall.type() << endl;
		cout << "Pthresholded: " << Pthresholded.type() << endl;
		cout << "Mat::zeros(P.size(), CV_32FC1): " << Mat::zeros(P.size(), CV_32FC1).type() << endl;
		cout << "Q.size: " << Q.size[0] << "*" << Q.size[1] << " type:" << Q.type() << endl;
		cout << "PthresholdedComp.size: " << PthresholdedComp.size[0] << "*" << PthresholdedComp.size[1] << " type:" << PthresholdedComp.type() << endl;
		Q += PthresholdedComp;
	try {
	} catch (const exception& ex) {
		cout << "ERROR: " << ex.what() << endl;
	}

//	Q = ((1 / P) * PIsBig);
	Q += (epsMaxAbsP * PIsSmall);

	// multiply with the inverse model of the distortion
	// (== convolute in spacial domain)
//	Mat O_approx = S * Q;
	Mat O_approx(P.size(), P.type());
	multiply(S, Q, O_approx);

	// transform back into spatial domain
	Mat o_approx(O_approx.size(), CV_32FC1);
//	dft(O_approx, o_approx, DFT_INVERSE | DFT_SCALE);
	dft(O_approx, o_approx, DFT_INVERSE| DFT_SCALE | DFT_REAL_OUTPUT);
//	dft(O_approx, o_approx, DFT_INVERSE | DFT_SCALE | DFT_COMPLEX_OUTPUT);
	cout << "O_approx: " << O_approx.type() << endl;
	cout << "o_approx: " << o_approx.type() << endl;

	return o_approx;
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
