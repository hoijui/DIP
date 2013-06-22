// =============================================================================
// Name        : main.cpp
// Author      :
// Version     : 1.0
// Copyright   : -
// Description : Loads image, calculates structure tensor,
//               defines and plots interest points
// =============================================================================

#include <iostream>
#include <opencv2/opencv.hpp>

#include "dip5.hpp"

/**
 * Usage:
 *   path to image in argv[1],
 *   sigma of directional gradient in argv[2], default 0.5
 */
int main(int argc, char** argv) {

    // load image as grayscale, path in argv[1]
    Mat img = imread(argv[1], 0);
    if (!img.data){
	cerr << "ERROR: Cannot load image from " << argv[1] << endl;
    }
    // convert U8 to 32F
    img.convertTo(img, CV_32FC1);
    //showImage(img, "original", 0);

    // this list will contain interest points
    vector<KeyPoint> points;

    // define standard deviation of directional gradient
    double sigma;
    if (argc < 3)
      sigma = 0.5;
    else
      sigma = atof(argv[2]);
    // calculate interest points
    getInterestPoints(img, sigma, points);

    // plot result
    img = imread(argv[1]);
    drawKeypoints(img, points, img, Scalar(0,0,255), DrawMatchesFlags::DRAW_OVER_OUTIMG + DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    showImage(img, "keypoints", 0, true, true);
    return 0;

}


/*
*************************
***   GIVEN FUNCTIONS ***
*************************
*/

void showImage(Mat& img, const char* win, int wait, bool show, bool save){

    Mat aux = img.clone();

    // scale and convert
    if (img.channels() == 1)
	normalize(aux, aux, 0, 255, CV_MINMAX);
    aux.convertTo(aux, CV_8UC1);
    // show
    if (show){
      imshow( win, aux);
      waitKey(wait);
    }
    // save
    if (save)
      imwrite( (string("img/") + string(win)+string(".png")).c_str(), aux);
}

void nonMaxSuppression(Mat& img, Mat& out){

	Mat tmp = img.clone();

	for(int x=1; x<out.cols-1; x++){
		for(int y=1; y<out.rows-1; y++){
			if ( img.at<float>(y-1, x) >= img.at<float>(y, x) ){
				tmp.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x-1) >= img.at<float>(y, x) ){
				tmp.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x+1) >= img.at<float>(y, x) ){
				tmp.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>( y+1, x) >= img.at<float>(y, x) ){
				tmp.at<float>(y, x) = 0;
				continue;
			}
		}
	}
	tmp.copyTo(out);
}
