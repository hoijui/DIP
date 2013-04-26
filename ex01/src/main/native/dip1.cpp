//============================================================================
// Name        : dip1.cpp
// Author      : Ronny Haensch
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, do something nice, save image
//============================================================================

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// function that performs some kind of (simple) image processing
Mat doSomethingThatMyTutorIsGonnaLike(Mat& img);

// usage: path to image in argv[1]
// main function, loads and saves image
int main(int argc, char** argv) {

	// check if image path was defined
	if (argc != 2) {
		cerr << "Usage: dip1 <path_to_image>" << endl;
		return -1;
	}

	// window names
	string win1 = string ("Original image");
	string win2 = string ("Result");

	// some images
	Mat inputImage, outputImage;

	// load image as gray-scale, path in argv[1]
	cout << "load image" << endl;
	inputImage = imread( argv[1] );
	cout << "done" << endl;

	// check if image can be loaded
	if (!inputImage.data) {
		cerr << "ERROR: Cannot read file " << argv[1] << endl;
		exit(-1);
	}

	// show input image
	namedWindow( win1.c_str() );
	imshow( win1.c_str(), inputImage );

	// do something (reasonable)
	outputImage = doSomethingThatMyTutorIsGonnaLike( inputImage );

	// show result
	namedWindow( win2.c_str() );
	imshow( win2.c_str(), outputImage );

	// save result
	imwrite("result.jpg", outputImage);

	// wait a bit
	waitKey(0);

	return 0;
}

// function that performs some kind of (simple) image processing
Mat doSomethingThatMyTutorIsGonnaLike(Mat& img) {

	const int div = 64;
	const int nl = img.rows;
	const int nc = img.cols * img.channels();

	for (int j = 0; j < nl; ++j) {
		uchar* data = img.ptr<uchar>(j);
		for (int i = 0; i < nc; ++i) {
			data[i] = (data[i] / div * div) + (div / 2);
		}
	}

	return img;
}

