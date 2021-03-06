//============================================================================
// Name        : dip2.cpp
// Author      : Ronny Haensch
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, apply noise reduction, save images
//============================================================================

#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// function headers of functions to be implemented
Mat spatialConvolution(const Mat&, const Mat&);
Mat averageFilter(const Mat& src, int kSize);
Mat medianFilter(const Mat& src, int kSize);
Mat adaptiveFilter(const Mat& src, int kSize, double threshold);
Mat bilateralFilter(const Mat& src, int kSize, double sigma);
// function headers of given functions
Mat noiseReduction(Mat&, string, int, double=0);
void generateNoisyImages(const Mat&);

void insertionSort(float window[]);
void cropCoordinate(int& coord, const int lowerBound, const int upperBound);

// usage: argv[1] == "generate" to generate noisy images, path to original image in argv[2]
// 	  argv[1] == "restorate" to load and restorate noisy images
int main(int argc, char** argv) {

	// check whether parameter were specified
	if (argc < 2) {
		cerr << "Usage:\n\tdip2 generate path_to_original\n\tdip2 restorate"  << endl;
		exit(-1);
	}

	// in a first step generate noisy images
	// path of original image is in argv[2]
	if (strcmp(argv[1], "generate") == 0) {
		// check whether original image was specified
		if (argc < 3) {
			cerr << "ERROR: original image not specified"  << endl;
			exit(-1);
		}

		// load image, path in argv[2], force gray-scale
		cout << "load original image" << endl;
		Mat img = imread(argv[2],0);
		if (!img.data) {
			cerr << "ERROR: file " << argv[2] << " not found" << endl;
			exit(-1);
		}
		// convert to floating point precision
		img.convertTo(img,CV_32FC1);
		cout << "done" << endl;

		// generate images with different types of noise
		cout << "generate noisy images" << endl;
		generateNoisyImages(img);
		cout << "done" << endl;
		cout << "Please run now: dip2 restorate" << endl;
	}

	// in a second step try to restorate noisy images
	if (strcmp(argv[1], "restorate") == 0) {
		// load images
		cout << "load images" << endl;
		Mat orig = imread("original.jpg",0);
		if (!orig.data) {
			cerr << "ERROR: original.jpg not found" << endl;
			exit(-1);
		}
		orig.convertTo(orig, CV_32FC1);
		Mat noise1 = imread("noiseType_1.jpg", 0);
		if (!noise1.data) {
			cerr << "noiseType_1.jpg not found" << endl;
			exit(-1);
		}
		noise1.convertTo(noise1, CV_32FC1);
		Mat noise2 = imread("noiseType_2.jpg",0);
		if (!noise2.data) {
			cerr << "noiseType_2.jpg not found" << endl;
			exit(-1);
		}
		noise2.convertTo(noise2, CV_32FC1);
		cout << "done" << endl;

		// apply noise reduction
		// TODO
		// ==> Choose appropriate noise reduction technique with appropriate parameters
		// ==> "average" or "median"? Why?
		// ==> try also "adaptive"
		Mat restorated1 = noiseReduction(noise1, "median", 3);
		Mat restorated2 = noiseReduction(noise2, "adaptive", 15, 30.0);

		// save images
		imwrite("restorated1.jpg", restorated1);
		imwrite("restorated2.jpg", restorated2);
	}

	return 0;
}

// the average filter
// src: input image
// kSize: window size used by local average
// return: filtered image
Mat averageFilter(const Mat& src, int kSize) {

	const Mat kernel = Mat::ones(kSize, kSize, src.type()) * (1.0 / (kSize*kSize));

	return spatialConvolution(src, kernel);
}

// the adaptive filter
// src: input image
// kSize: window size used by local average
// return: filtered image
Mat adaptiveFilter(const Mat& src, int kSize, double threshold) {

	const int w = src.rows;
	const int h = src.cols;

	Mat res = averageFilter(src, kSize);
	Mat origSmoothed3 = averageFilter(src, 3);
	for (int x = 0 ; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			const float diff = fabs((float)res.at<float>(x, y) - origSmoothed3.at<float>(x, y));
			if (diff > threshold) {
				// if the change of value exceeds the threshold,
				// we use the original value
				res.at<float>(x, y) = src.at<float>(x, y);
			}
		}
	}

	return res;
}

// the median filter
// src: input image
// kSize: window size used by median operation
// return: filtered image
Mat medianFilter(const Mat& src, int kSize) {

	const int w = src.rows;
	const int h = src.cols;
	const int kw = kSize;
	const int kh = kSize;
	const int kwHalf = (int) (kw / 2);
	const int khHalf = (int) (kh / 2);

	Mat dst(src.size(), src.type());
	float window[kSize * kSize];
	for (int x = 0; x < w; ++x) { // image.x
		for (int y = 0; y < h; ++y) { // image.y
			int wi = 0;
			for (int kx = 0; kx < kw; ++kx) { // kernel.x
				for (int ky = 0; ky < kh; ++ky) { // kernel.y
					int imgX = x - kwHalf + kx;
					cropCoordinate(imgX, 0, w);
					int imgY = y - khHalf + ky;
					cropCoordinate(imgY, 0, h);
					window[wi++] = src.at<float>(imgX, imgY);
				}
			}

			// sort the window to find median
			insertionSort(window);

			// assign the median to centered element of the matrix
			const float median = window[(kSize * kSize) / 2];
			dst.at<float>(x, y) = median;
		}
	}

	return dst;
}

// sortieren
void insertionSort(float window[]) {

	int j = 0;
	for (int i = 0; i < 9; i++) {
		float temp = window[i];
		for (j = i-1; j >= 0 && temp < window[j]; j--) {
			window[j+1] = window[j];
		}
		window[j+1] = temp;
	}
}

// the bilateral filter
// src: input image
// kSize: window size of kernel --> used to compute std-dev of spatial kernel
// sigma: standard-deviation of radiometric kernel
// return: filtered image
Mat bilateralFilter(const Mat& src, int kSize, double sigma) {

	// TODO
	return src.clone();
}

/**
 * Makes sure that the given coordinate coimes to lay within the bounds.
 * This may be achieved by one of multiple border-handling techniques.
 * We use edge mirroring here.
 */
void cropCoordinate(int& coord, const int lowerBound, const int upperBound) {

	if (coord < lowerBound) {
		coord = lowerBound + (lowerBound - coord);
	} else if (coord >= upperBound) {
		coord = upperBound - (coord - upperBound + 1);
	}
}

// convolution in spatial domain
// src: input image
// kernel: filter kernel
// return: convolution result
Mat spatialConvolution(const Mat& src, const Mat& kernel) {

	Mat res = Mat(src.size(), src.type());

	const bool space = true;
	if (space) {
		const int w = src.rows;
		const int h = src.cols;
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
						newVal += src.at<float>(imgX, imgY) * kernel.at<float>(kx, ky);
					}
				}
				res.at<float>(x, y) = newVal;
			}
		}
	} else { // frequency
		Mat freqSrc = Mat(src.size(), src.type());
		Mat freqKernel = Mat(kernel.size(), kernel.type());
		dft(src, freqSrc);
		dft(kernel, freqKernel);
		Mat freqRes = freqSrc * freqKernel; // FIXME This will obviously crash, because the kernel is much smaller then the image -> dimensions do not match
		dft(freqRes, res, DFT_INVERSE);
	}

	return res;
}

// ************************
// ***  Given Functions ***
// ************************

// noise reduction
// src: input image
// method: name of method to be performed
//	    "average" ==> moving average
//	    "median" ==> median filter
//	    "adaptive" ==> edge preserving average filter
//	    "bilateral" ==> bilateral filter
// kSize: window size used by median operation
// param: if method == "adaptive" : threshold ; if method == "bilateral" standard-deviation of radiometric kernel
// return: output image
Mat noiseReduction(Mat& src, string method, int kSize, double param) {

	// apply moving average filter
	if (method.compare("average") == 0) {
		return averageFilter(src, kSize);
	}
	// apply median filter
	if (method.compare("median") == 0) {
		return medianFilter(src, kSize);
	}
	// apply adaptive average filter
	if (method.compare("adaptive") == 0) {
		return adaptiveFilter(src, kSize, param);
	}
	// apply bilateral filter
	if (method.compare("bilateral") == 0) {
		return bilateralFilter(src, kSize, param);
	}

	cerr << "WARNING: Unknown filtering method! Return original" << endl;
	return src;
}

// generates and saves different noisy versions of input image
// orig: input image
void generateNoisyImages(const Mat& orig) {

	// save original
	imwrite("original.jpg", orig);

	// some temporary images
	Mat tmp1(orig.rows, orig.cols, CV_32FC1);
	Mat tmp2(orig.rows, orig.cols, CV_32FC1);
	// first noise operation
	float noiseLevel = 0.15;
	randu(tmp1, 0, 1);
	threshold(tmp1, tmp2, noiseLevel, 1, CV_THRESH_BINARY);
	multiply(tmp2,orig,tmp2);
	threshold(tmp1, tmp1, 1-noiseLevel, 1, CV_THRESH_BINARY);
	tmp1 *= 255;
	tmp1 = tmp2 + tmp1;
	threshold(tmp1, tmp1, 255, 255, CV_THRESH_TRUNC);
	// save image
	imwrite("noiseType_1.jpg", tmp1);

	// second noise operation
	noiseLevel = 50;
	randn(tmp1, 0, noiseLevel);
	tmp1 = orig + tmp1;
	threshold(tmp1,tmp1,255,255,CV_THRESH_TRUNC);
	threshold(tmp1,tmp1,0,0,CV_THRESH_TOZERO);
	// save image
	imwrite("noiseType_2.jpg", tmp1);
}
