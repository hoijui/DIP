//============================================================================
// Name        : dip3.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, apply unsharp masking
//============================================================================

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

void cropCoordinate(int& looseCoord, const int lowerBound, const int upperBound);

// usage: path to image in argv[1]
int main(int argc, char** argv) {

	// file to save time measurements
	fstream fileSpatial("convolutionSpatialDomain.txt", ios::out);
	fstream fileFrequency("convolutionFrequencyDomain.txt", ios::out);

  	// some windows for displaying images
	const char* win_1 = "Degraded Image";
	const char* win_2 = "Enhanced Image";
	const char* win_3 = "Differences";
	namedWindow( win_1, CV_WINDOW_AUTOSIZE );
	namedWindow( win_2, CV_WINDOW_AUTOSIZE );
	namedWindow( win_3, CV_WINDOW_AUTOSIZE );

	// for time measurements
	clock_t time;

	// parameter of USM
	int numberOfKernelSizes = 10;			// number of differently sized smoothing kernels
	double thresh = 0;				// TODO threshold on difference necessary to perform operation
	double scale = 1;				// TODO scaling of edge enhancement

	// load image, path in argv[1]
	cout << "load image" << endl;
	Mat imgIn = imread(argv[1]);
	// distort image with gaussian blur
	int size = 3;
	GaussianBlur(imgIn, imgIn, Size(floor(size/2)*2+1,floor(size/2)*2+1), size/5., size/5.);
	imwrite("dedgraded.png", imgIn);
	cout << "done" << endl;

	// show degraded
	imshow( win_1, imgIn);

	// create output image
	Mat result = Mat(imgIn.rows, imgIn.cols, CV_32FC3);

	// convert and split input image
	// convert BGR to HSV
	cvtColor(imgIn, imgIn, CV_BGR2HSV);
	// convert U8 to 32F
	imgIn.convertTo(imgIn, CV_32FC3);
	// split into planes
	vector<Mat> planes;
	split(imgIn, planes);
	// only work on value-channel
	Mat value = planes.at(2).clone();

	// unsharp masking
	// try different kernel sizes
	for (int s=1; s<=numberOfKernelSizes; s++) {

		// use this size for smoothing
		int size = 4*s+1;

		// either working in spatial or frequency domain
		for (int type=0; type<2; type++) {
			// speak to me
			switch(type) {
				case 0: cout << "> USM (" << size << "x" << size << ", using spatial domain):\t" << endl;break;
				case 1: cout << "> USM (" << size << "x" << size << ", using frequency domain):\t" << endl;break;
			}

			// measure starting time
			time = clock();
			// perform unsharp masking
			Mat tmp = usm(value, type, size, thresh, scale);
			// measure stopping time
			time = (clock() - time);
			// print the ellapsed time
			switch(type) {
				case 0:{
					cout << ((double)time)/CLOCKS_PER_SEC << "sec\n" << endl;
					fileSpatial << ((double)time)/CLOCKS_PER_SEC << endl;
				} break;
				case 1:{
					cout << ((double)time)/CLOCKS_PER_SEC << "sec\n" << endl;
					fileFrequency << ((double)time)/CLOCKS_PER_SEC << endl;
				} break;
			}

			// produce output image
			planes.at(2) = tmp;
			// merge planes to color image
			merge(planes, result);
			// convert 32F to U8
			result.convertTo(result, CV_8UC3);
			// convert HSV to BGR
			cvtColor(result, result, CV_HSV2BGR);

			// show and save output images
			// create filename
			ostringstream fname;
			fname << string(argv[1]).substr(0,string(argv[1]).rfind(".")) << "_USM_" << size << "x" << size << "_";
			switch(type) {
				case 0: fname << "spatialDomain";break;
				case 1: fname << "frequencyDomain";break;
			}
			imshow( win_2, result);
			imwrite((fname.str() + "_enhanced.png").c_str(), result);

			// produce difference image
			planes.at(2) = tmp - value;
			normalize(planes.at(2), planes.at(2), 0, 255, CV_MINMAX);
			// merge planes to color image
			merge(planes, result);
			// convert 32F to U8
			result.convertTo(result, CV_8UC3);
			// convert HSV to BGR
			cvtColor(result, result, CV_HSV2BGR);
			imshow( win_3, result);
			imwrite((fname.str() + "_diff2original.png").c_str(), result);

			// images will be displayed for 3 seconds
			//cvWaitKey(3000);
			// reset to original
			planes.at(2) = value;
		}
	}

	// be tidy
	fileSpatial.close();
	fileFrequency.close();

	return 0;
}

/*
Generates a gaussian filter kernel of given size
kSize:		kernel size (used to calculate standard deviation)
return:		the generated filter kernel
*/
Mat createGaussianKernel(int kSize) {

	Mat kernel = Mat::zeros(kSize, kSize, CV_32FC1);

	const int kh = kSize / 2;
	const float sigma = kSize / 5.0f;
	const float sigmaSqr = sigma * sigma;
	const float gaussMul = 1.0f / (2 * M_PI * sigma * sigma);

	for (int x = 0; x < kh + 1; ++x) {
		for (int y = 0; y < kh + 1; ++y) {
			// calculate gaussian value
			const float curVal = gaussMul * exp(0.5f * ((x*x + y*y) / sigmaSqr));
			// ... and copy it to each quarter
			kernel.at<float>(kh + x, kh + y) = curVal;
			kernel.at<float>(kh - x, kh + y) = curVal;
			kernel.at<float>(kh + x, kh - y) = curVal;
			kernel.at<float>(kh - x, kh - y) = curVal;
		}
	}

	return kernel;
}

/*
Performes a circular shift in (dx,dy) direction
in	input matrix
dx	shift in x-direction
dy	shift in y-direction
return	circular shifted matrix
*/
Mat circShift(Mat& in, int dx, int dy) {

	const int h = in.rows;
	const int w = in.cols;

	Mat res = Mat::zeros(h, w, in.type());

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

			res.at<float>(yNew, xNew) = in.at<float>(y, x);
		}
	}

	return res;
}

/*
Performes a convolution by multiplication in frequency domain
in		input image
kernel		filter kernel
return		output image
*/
Mat frequencyConvolution(Mat& in, Mat& kernel) {

	const int kw = kernel.cols;
	const int kh = kernel.rows;

	Mat kernelBig = Mat::zeros(in.size(), in.type());
	for (int x = 0; x < kw + 1; ++x) {
		for (int y = 0; y < kh + 1; ++y) {
			kernelBig.at<float>(x, y) = kernel.at<float>(x, y);
		}
	}
	circShift(kernelBig, kw/2, kh/2);

	Mat freqIn = Mat(in.size(), in.type());
	Mat freqKernel = Mat(kernelBig.size(), kernelBig.type());
	Mat res = Mat(in.size(), in.type());
	dft(in, freqIn);
	dft(kernel, freqKernel);
	Mat freqRes = freqIn * freqKernel;
	dft(freqRes, res, DFT_INVERSE | DFT_SCALE);

	return res;
}

/**
 * Makes sure that the given coordinate coimes to lay within the bounds.
 * This may be achieved by one of multiple border-handling techniques.
 * We use edge mirroring here.
 */
void cropCoordinate(int& looseCoord, const int lowerBound, const int upperBound) {

	if (looseCoord < lowerBound) {
		looseCoord = lowerBound + (lowerBound - looseCoord);
	} else if (looseCoord >= upperBound) {
		looseCoord = upperBound - (upperBound + 1 - looseCoord);
	}
}

/*
Performes a convolution by multiplication in spatial domain
in		input image
kernel		filter kernel
return		output image
*/
Mat spatialConvolution(Mat& in, Mat& kernel) {

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



/*
Performs UnSharp Masking to enhance fine image structures
in		input image
type		integer defining how convolution for smoothing operation is done
		0 <==> spatial domain; 1 <==> frequency domain
size		size of used smoothing kernel
thresh		minimal intensity difference to perform operation
scale		scaling of edge enhancement
return		enhanced image
*/
Mat usm(Mat& in, int type, int size, double thresh, double scale) {
	// TODO
}

/*
*************************
***   GIVEN FUNCTIONS ***
*************************
*/

/*
Performes smoothing operation by convolution
in		input image
size		size of filter kernel
spatial		true if convolution shall be performed in spatial domain, false otherwise
return		smoothed image
*/
Mat mySmooth(Mat& in, int size, bool spatial) {

	// create filter kernel
	Mat kernel = createGaussianKernel(size);

	// perform convoltion
	if (spatial) {
		return spatialConvolution(in, kernel);
	} else {
		return frequencyConvolution(in, kernel);
	}
}
