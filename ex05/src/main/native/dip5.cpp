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


static Mat circShift(Mat& in, int dx, int dy) {

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

static Mat frequencyConvolution(Mat& in, Mat& kernel) {

	const int kw = kernel.cols;
	const int kh = kernel.rows;

	Mat kernelBig = Mat::zeros(in.size(), in.type());
	for (int x = 0; x < kw; ++x) {
		for (int y = 0; y < kh; ++y) {
			kernelBig.at<float>(x, y) = kernel.at<float>(x, y);
		}
	}
	Mat kernelBigShifted = circShift(kernelBig, -(kw/2), -(kh/2));

	// transform into frequency domain
	Mat freqIn = Mat(in.size(), CV_32FC2); // complex
	Mat freqKernel = Mat(kernelBigShifted.size(), CV_32FC2); // complex
	dft(in, freqIn);
	dft(kernelBigShifted, freqKernel);

	// multiply in frequency domain (-> convolute in spatial domain)
	Mat freqRes = Mat(kernelBig.size(), CV_32FC2); // complex
	mulSpectrums(freqIn, freqKernel, freqRes, 0);

	// transform back into spatial domain
	Mat res(in.size(), in.type());
	dft(freqRes, res, DFT_INVERSE | DFT_SCALE);

	return res;
}

void applyKernelAt(Mat& in, Mat& out, Mat& kernel, int row, int col){

	float total = 0;
	int rowCoord, colCoord;
	int kerCenterRowShift = kernel.rows / 2;
	int kerCenterColShift = kernel.cols / 2;

	for(int i = 0; i < kernel.rows; i++){
		for(int j = 0; j < kernel.cols; j++){

			rowCoord = row - kerCenterRowShift + i;
			colCoord = col - kerCenterColShift + j;

			// correct coordinates at borders (move them inside the image again)
			rowCoord = rowCoord < 0 ? abs(rowCoord) : rowCoord;
			rowCoord = rowCoord >= in.rows ? (2*in.rows-1) - rowCoord : rowCoord;
			colCoord = colCoord < 0 ? abs(colCoord) : colCoord;
			colCoord = colCoord >= in.cols? (2*in.cols-1) - colCoord : colCoord;

			//now sum up stuff according to kernel and superimposed pixel value
			total += kernel.at<float>(i,j) * in.at<float>(rowCoord,colCoord);
		}
	}

	//done summing up over a region in the input image, writing to output.
	out.at<float>(row,col) = total;
}

// spatial convolution
/*
in		input image
out		output image
kernel		the convolution kernel
phi		orientation
*/
void spatialConvolution(Mat& in, Mat& out, Mat& kernel){
	for(int row = 0; row < in.rows; row++)
	{
		for(int col = 0; col < in.cols; col++)
		{
			applyKernelAt(in,out,kernel,row,col);
		}
	}
}

void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points) {

	const int kernelSize = 9;
	cout << "I-Points!" << endl;
	Mat devXK(kernelSize, kernelSize, img.type());
	Mat devYK(kernelSize, kernelSize, img.type());
	Mat gauss(kernelSize, kernelSize, img.type());
	createFstDevKernel(gauss, 2*sigma);
	createFstDevKernel(devXK, sigma);
	cv::transpose(devXK, devYK);

	cout << "Gradients!" << endl;
	Mat gradX = Mat::zeros(img.size(), CV_32FC1);
	Mat gradY = Mat::zeros(img.size(), CV_32FC1);

	// then calculate the gradients
	cout << "Gradients 1!" << endl;
//	spatialConvolution(img,gradX,devXK);
	gradX = frequencyConvolution(img, devYK);
	//showImage(gradX, "gX", 0, true, devXK);
//	spatialConvolution(img,gradY,devYK);
	gradY = frequencyConvolution(img, devYK);
	//showImage(gradY, "gY", 0, true, true);

	cout << "Gradients 2!" << endl;
	Mat gXX, gYY,gXY,avgGXY, trace, det, temp, w, wnms, q, qnms;
	Mat gYYs = Mat::zeros(img.size(), CV_32FC1);
	Mat gXXs = Mat::zeros(img.size(), CV_32FC1);
	Mat gXYs = Mat::zeros(img.size(), CV_32FC1);

	multiply(gradX, gradY, gXY);
	multiply(gradX, gradX, gXX);
	multiply(gradY, gradY, gYY);

	cout << "Gradients 3!" << endl;
//	spatialConvolution(gXX,gXXs,gauss);
//	spatialConvolution(gYY,gYYs,gauss);
//	spatialConvolution(gXY,gXYs,gauss);
	gXXs = frequencyConvolution(gXX, gauss);
	gYYs = frequencyConvolution(gYY, gauss);
	gXYs = frequencyConvolution(gXY, gauss);

	cout << "Trace!" << endl;
	// trace
	add(gXXs, gYY, trace);
	
	//showImage(trace, "trace", 0, true, true);

	// determinant
	cout << "Determinant!" << endl;
	multiply(gXXs, gYYs, det);
	//showImage(det, "xxyy", 0, true, true);
	multiply(gXYs, gXYs, temp);
//	showImage(temp, "xyxy", 0, true, true);
	subtract(det, temp, det);
	//showImage(det, "det", 0, true, true);

	// weight
	cout << "Weight!" << endl;
	divide(det, trace, w);
	nonMaxSuppression(w, wnms);
	showImage(wnms, "wmns", 0, true, true);

	// isodings
	cout << "Isodings!" << endl;
	multiply(trace,trace,temp);
	divide(4*det,temp,q);
	nonMaxSuppression(q,qnms);
	showImage(qnms, "qmns", 0, true, true);

	cout << "Thresholding!" << endl;
	float threshW = 100;
	float threshQ = 0.8;
	for(int x=0; x<img.rows; x++){
		for(int y=0; y<img.cols; y++){
			if (qnms.at<float>(x,y) >= threshQ && wnms.at<float>(x,y) >= threshW)
			{
				KeyPoint kp;
				kp.pt.x = y;
				kp.pt.y = x;
				kp.angle = 0;
				kp.size = 3;
				kp.response = wnms.at<float>(x,y);
				points.push_back(kp);
			}
		}
	}
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
			if (kernel.type() == CV_32FC2) {
				kernel.at<Vec2f>(x, y)[0] = -xv * commonFac; // partial derivative towards x
				kernel.at<Vec2f>(x, y)[1] = -yv * commonFac; // partial derivative towards y
			} else {
				kernel.at<float>(x, y) = -xv * commonFac; // partial derivative towards x
			}
		}
	}
}
