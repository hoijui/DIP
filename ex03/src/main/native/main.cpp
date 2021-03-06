//============================================================================
// Name        : dip3.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, apply unsharp masking
//============================================================================

#include "dip3.hpp"

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
	double thresh = 2.0;				// TODO threshold on difference necessary to perform operation
	double scale = 3.0;				// TODO scaling of edge enhancement

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
