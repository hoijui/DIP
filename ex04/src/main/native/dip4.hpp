//============================================================================
// Name        : dip4.hpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : load image using opencv, degrade image,
//               and apply inverse as well as wiener filter
//============================================================================

#ifndef _DIP4_H_
#define _DIP4_H_

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// function headers of not yet implemented functions

/**
 * Function applies inverse filter to restore a degraded image.
 * @param degraded  degraded input image
 * @param filter  filter which caused degradation
 * @return  restored output image
 */
Mat inverseFilter(Mat& degraded, Mat& filter);

/**
 * Function applies wiener filter to restore a degraded image.
 * @param degraded  degraded input image
 * @param filter  filter which caused degradation
 * @param snr  signal to noise ratio of the input image
 * @return  restored output image
 */
Mat wienerFilter(Mat& degraded, Mat& filter, double snr);

/**
 * Creates a filter kernel matrix of a certain type and size.
 * @param kernel  where to store the generated kernel to
 * @param kSize  size of the kernel (kSize * kSize)
 * @param name  name of the type of kernel to generate; possible values:
 *   "gaussian", "uniform"
 */
void createKernel(Mat& kernel, int kSize, string name = "gaussian");

/**
 * Performs a circular shift in (dx,dy) direction.
 * @param in  input matrix
 * @param out  circular shifted matrix
 * @param dx  shift in x-direction
 * @param dy  shift in y-direction
 */
void circShift(Mat& in, Mat& out, int dx, int dy);

// function headers of given functions

/**
 * Function degrades a given image with gaussian blur and additive gaussian noise.
 * @param img  input image
 * @param degradedImg  degraded output image
 * @param filterDev  standard deviation of kernel for gaussian blur
 * @param snr  signal to noise ratio for additive gaussian noise
 * @return the used gaussian kernel
 */
Mat degradeImage(Mat& img, Mat& degradedImg, double filterDev, double snr);

/**
 * Function displays image (after proper normalization).
 * @param win  Window name
 * @param img  Image that shall be displayed
 * @param cut  whether to cut or scale values outside of [0,255] range
 */
void showImage(const char* win, Mat img, bool cut = true);

#endif // _DIP4_H_
