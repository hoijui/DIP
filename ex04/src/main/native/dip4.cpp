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

static void circShiftXXX(Mat& in, Mat& out, int dx, int dy){

	int dstI, dstJ;

	for(int i=0; i<in.rows;i++)
	{
		dstI = i+dx;
		if(dstI<0)
		{
			dstI += out.rows;
		} else if(dstI>=out.rows){
			dstI -= out.rows;
		}

		for(int j=0; j<in.cols;j++)
		{
			dstJ = j+dy;
			if(dstJ<0)
			{
				dstJ += out.cols;
			} else if(dstJ>=out.cols){
				dstJ -= out.cols;
			}

			out.at<float>(dstI,dstJ) = in.at<float>(i,j);
		}
	}
}

static Mat inverseAndWiener(Mat& s, Mat& p, double snr, bool inverse) {

	const bool wiener = !inverse;

	// Pad input image to avoid ringing artifacts along image borders.
	int bH = p.cols;
	int bV = p.rows;
	Mat sBorder;
	copyMakeBorder(s, sBorder, bV, bV, bH, bH, BORDER_REPLICATE);

	// Allocate some memory like it is going out of style.
	Mat pBigShifted = Mat::zeros(sBorder.size(), CV_32F);
	Mat P = Mat::zeros(sBorder.size(), CV_32F);
	Mat S = Mat::zeros(sBorder.size(), CV_32F);
	Mat OApprox = Mat::zeros(sBorder.size(), CV_32F);
	Mat oApprox = Mat::zeros(sBorder.size(), CV_32F);

	// Shift kernel.
	const int pHalf = p.rows / 2;
	circShiftXXX(p, pBigShifted, -pHalf, -pHalf);

	// Transform shifted kernel and degrated input image into frequency domain.
	// Note: DFT_COMPLEX_OUTPUT means that we want the complex result to be stored
	//       in a two-channel matrix as opposed to the default compressed output.
	dft(pBigShifted, P, DFT_COMPLEX_OUTPUT);
	dft(sBorder, S, DFT_COMPLEX_OUTPUT);

	if (inverse) {
		const double epsilon = 0.05f;

		// Remove frequencies whose magnitude is below epsilon * max(freqKernel magnitude).
		double maxMagnitude;
		minMaxLoc(abs(P), 0, &maxMagnitude);

		const double threshold =  maxMagnitude * epsilon;
		for (int ri = 0; ri < P.rows; ri++) {
			for (int ci = 0; ci < P.cols; ci++) {
				if (norm(P.at<Vec2f>(ri, ci)) < threshold) {
					P.at<Vec2f>(ri, ci) = threshold;
				}
			}
		}
	}

	// OpenCV only provides a multiplication operation for complex matrices, so we need
	// to calculate the inverse (1/H) of our filter spectrum first. Since it is complex
	// we need to compute 1/H = H*/(HH*) = H*/(Re(H)^2+Im(H)^2), where H* -> complex conjugate of H.

	// Multiply spectrum of the degrated image with the complex conjugate of the frequency spectrum
	// of the filter.
	const bool conjFreqKernel = true;
	mulSpectrums(S, P, OApprox, DFT_COMPLEX_OUTPUT, conjFreqKernel); // I * H*

	// Split kernel spectrum into real and imaginary parts.
	Mat PChannels[] = {Mat::zeros(sBorder.size(), CV_32F), Mat::zeros(sBorder.size(), CV_32F)};
	split(P, PChannels); // 0:real, 1:imaginary

	// Calculate squared magnitude (Re(H)^2 + Im(H)^2) of filter spectrum.
	Mat freqKernelSqMagnitude = Mat::zeros(sBorder.rows, sBorder.cols, CV_32F);
	magnitude(PChannels[0], PChannels[1], freqKernelSqMagnitude); // freqKernelSqMagnitude = magnitude
	pow(PChannels[0], 2, freqKernelSqMagnitude); // freqKernelSqMagnitude = magnitude^2 = Re(H)^2 + Im(H)^2

	if (wiener) {
		// Add 1 / SNR^2 to the squared filter kernel magnitude.
		freqKernelSqMagnitude += 1 / pow(snr, 2.0);
	}

	// Split frequency spectrum of degradedPadded image into real and imaginary parts.
	Mat OApproxChannels[] = {Mat::zeros(sBorder.size(), CV_32FC1), Mat::zeros(sBorder.size(), CV_32F)};
	split(OApprox, OApproxChannels);

	// Divide each plane by the squared magnitude of the kernel frequency spectrum.
	// What we have done up to this point: (I * H*) / (Re(H)^2 + Im(H)^2) = I/H
	divide(OApproxChannels[0], freqKernelSqMagnitude, OApproxChannels[0]); // Re(I) / (Re(H)^2 + Im(H)^2)
	divide(OApproxChannels[1], freqKernelSqMagnitude, OApproxChannels[1]); // Im(I) / (Re(H)^2 + Im(H)^2)

	// Merge real and imaginary parts of the image frequency spectrum.
	merge(OApproxChannels, 2, OApprox);

	// Inverse DFT.
	// Note: DFT_REAL_OUTPUT means that we want the output to be a one-channel matrix again.
	dft(OApprox, oApprox, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);

	// Crop output image to original size.
	oApprox = oApprox(Rect(bH, bV, oApprox.cols - (bH * 2), oApprox.rows - (bV * 2)));

	return oApprox;
}

Mat inverseFilter(Mat& degraded, Mat& filter) {
	return inverseAndWiener(degraded, filter, -1.0, true);
}

Mat wienerFilter(Mat& degraded, Mat& filter, double snr) {
	return inverseAndWiener(degraded, filter, snr, false);
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
