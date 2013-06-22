// =============================================================================
// Name        : dip5.hpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : Loads image, calculates structure tensor,
//               defines and plots interest points
// =============================================================================

#ifndef _DIP5_H_
#define _DIP5_H_

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

// function headers of not yet implemented functions

/**
 * Uses structure tensor to define interest points (foerstner).
 */
void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points);

/**
 * Creates a specific kernel.
 * @param kernel  the calculated kernel
 * @param kSize  size of the kernel
 */
void createFstDevKernel(Mat& kernel, double sigma);



// function headers of given functions

/**
 * Show and save one-channel images.
 * Image will be saved to ./img/ folder (has to be created before).
 */
void showImage(Mat& img, const char* win, int wait, bool show, bool save);

/**
 * Non-maxima suppression.
 * If any of the pixel at the 4-neighborhood is greater than current pixel,
 * set it to zero.
 */
void nonMaxSuppression(Mat& img, Mat& out);

#endif // _DIP5_H_
