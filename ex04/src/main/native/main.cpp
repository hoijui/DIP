//============================================================================
// Name        : main.cpp
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

/**
 * usage: path to image in argv[1]
 */
int main(int argc, char** argv) {

	// some windows for displaying images
	const char* win_1 = "Original Image";
	const char* win_2 = "Degraded Image";
	const char* win_3 = "Restored Image: Inverse filter";
	const char* win_4 = "Restored Image: Wiener filter";
	namedWindow(win_1);
	namedWindow(win_2);
	namedWindow(win_3);
	namedWindow(win_4);

	// load image, path in argv[1]
	cout << "load image" << endl;
	Mat img = imread(argv[1], 0);
	// convert U8 to 32F
	img.convertTo(img, CV_32FC1);
	cout << " > done" << endl;

	// show and safe gray-scale version of original image
	showImage(win_1, img);
	imwrite("original.png", img);

	// degrade image
	cout << "degrade image" << endl;
	double filterDev = 9;
	double snr = 10; //10000;
	Mat degradedImg;
	Mat gaussKernel = degradeImage(img, degradedImg, filterDev, snr);
	cout << " > done" << endl;

	// show and safe degraded image
	showImage(win_2, degradedImg);
	imwrite("degraded.png", degradedImg);

	// inverse filter
	cout << "inverse filter" << endl;
	Mat restoredImgInverseFilter = inverseFilter(degradedImg, gaussKernel);
	cout << " > done" << endl;

	// show and safe restored image
	showImage(win_3, restoredImgInverseFilter);
	imwrite("restored_inverse.png", restoredImgInverseFilter);

	// wiener filter
	cout << "wiener filter" << endl;
	Mat restoredImgWienerFilter = wienerFilter(degradedImg, gaussKernel, snr);
	cout << " > done" << endl;

	// show and safe restored image
	showImage(win_4, restoredImgWienerFilter, false);
	imwrite("restored_wiener.png", restoredImgWienerFilter);

	// wait
	waitKey(0);

	return 0;
}

/*
 *************************
 ***   GIVEN FUNCTIONS ***
 *************************
 */

Mat degradeImage(Mat& img, Mat& degradedImg, double filterDev, double snr) {

	int kSize = round(filterDev * 3)*2 - 1;

	Mat gaussKernel = getGaussianKernel(kSize, filterDev, CV_32FC1);
	gaussKernel = gaussKernel * gaussKernel.t();
	filter2D(img, degradedImg, -1, gaussKernel);

	Mat mean, stddev;
	meanStdDev(img, mean, stddev);

	Mat noise = Mat::zeros(img.rows, img.cols, CV_32FC1);
	randn(noise, 0, stddev.at<double>(0) / snr);
	degradedImg = degradedImg + noise;
	threshold(degradedImg, degradedImg, 255, 255, CV_THRESH_TRUNC);
	threshold(degradedImg, degradedImg, 0, 0, CV_THRESH_TOZERO);

	return gaussKernel;
}

void showImage(const char* win, Mat img, bool cut) {

	Mat tmp = img.clone();

	if (tmp.channels() == 1) {
		if (cut) {
			threshold(tmp, tmp, 255, 255, CV_THRESH_TRUNC);
			threshold(tmp, tmp, 0, 0, CV_THRESH_TOZERO);
		} else {
			normalize(tmp, tmp, 0, 255, CV_MINMAX);
		}

		tmp.convertTo(tmp, CV_8UC1);
	} else {
		tmp.convertTo(tmp, CV_8UC3);
	}
	imshow(win, tmp);
}
