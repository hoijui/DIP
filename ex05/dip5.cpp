//============================================================================
// Name        : dip5.cpp
// Author      : 
// Version     : 1.0
// Copyright   : -
// Description : Loads image, calculates structure tensor, defines and plots interest points
//============================================================================

#include <iostream>
#include <list>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

// function headers of not yet implemented functions
void createFstDevKernel(Mat& kernel, double sigma);
void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points);
// function headers of given functions
void nonMaxSuppression(Mat& img, Mat& out);
void showImage(Mat& img, const char* win, int wait, bool show=true, bool save=false);

// usage: path to image in argv[1], sigma of directional gradient in argv[2], default 0.5
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

// uses structure tensor to define interest points (foerstner)
void getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points){

    // TO DO !!!

}

// creates a specific kernel
/*
kernel	the calculated kernel
kSize	size of the kernel
*/
void createFstDevKernel(Mat& kernel, double sigma){

    // TO DO !!!

}

/*
*************************
***   GIVEN FUNCTIONS ***
*************************
*/

// function to show and save one-channel images
// image will be saved to ./img/ folder (has to be created before)
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

// non-maxima suppression
// if any of the pixel at the 4-neighborhood is greater than current pixel, set it to zero
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

