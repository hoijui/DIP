//============================================================================
// Name        : dip3.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, apply unsharp masking
//============================================================================

#include "dip3.hpp"

/*
Generates a gaussian filter kernel of given size
kSize:		kernel size (used to calculate standard deviation)
return:		the generated filter kernel
*/
Mat createGaussianKernel(int kSize) {

	Mat kernel = Mat::zeros(kSize, kSize, CV_32FC1);

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
//	cout << "Kernel integral (should be ~= 1.0f): " << integral << endl;

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

	const bool spatial = (type == 0);
	Mat diff = mySmooth(in, size, spatial);
	diff -= in;
	diff *= scale;
	Mat res = in + diff;

	int ltt = 0;
	int htt = 0;
	const int w = in.rows;
	const int h = in.cols;
	for (int x = 0; x < w; ++x) { // image.x
		for (int y = 0; y < h; ++y) { // image.y
			// reset to input if the difference is below the threshold
			float newVal;
			if (diff.at<float>(x, y) < thresh) {
				newVal = in.at<float>(x, y);
				ltt++;
			} else {
				newVal = res.at<float>(x, y);
				htt++;
			}
			res.at<float>(x, y) = newVal;
		}
	}
//	cout << "higher then threshold: " << htt << "  /  lower: " << ltt << endl;

	return res;
}
