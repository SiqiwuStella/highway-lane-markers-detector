// Helper code generate matrices used for perspective warp in “track.cpp” to track highway lane markings 
// Result: Generates 2 matrices used for perspective warp
//
// Author: Siqi Wu(Stella)
// Email: siqiw@g.clemson.edu
// Date: 06/21/2016

#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace cv;

int main(){
	Mat img;
	Mat input;
	Mat output;

	img = imread("images/mono_0000002196.png");
	if (img.empty()){
		std::cout << "error: cannot read image." << std::endl;
		return 0;
	}
	
	cvtColor(img, input, CV_BGR2GRAY);

	//create perspective warp matrix
	Mat warp_mat = Mat(3, 3, CV_32FC1);
	//create input and output quad points
	Point2f inputQuad[4], outputQuad[4];
	
	//image width and height
	int w = img.cols;
	int h = img.rows;
	std::cout << "w = " << w << " h = " << h << std::endl;

	//get a set of 4 points to calculate the perspective warp matrix
	inputQuad[0] = Point2f(683, 630);
	inputQuad[1] = Point2f(67, 1200);
	inputQuad[2] = Point2f(763, 630);
	inputQuad[3] = Point2f(1540, 1200);
	
	outputQuad[0] = Point2f(683, 630);
	outputQuad[1] = Point2f(683, 1200);
	outputQuad[2] = Point2f(763, 630);
	outputQuad[3] = Point2f(763, 1200);

	
	warp_mat = getPerspectiveTransform(inputQuad, outputQuad);
	
	//write the warp matrix
	FileStorage file("warp_mat.xml", FileStorage::WRITE);
	file << "warp_mat" << warp_mat;
	file.release();
	
	//display the selected points
	int a = 683, b = 630;
	rectangle(img, Point(a, b), Point(a+3, b+3), Scalar(0, 0, 255), 8, 1);
	int a1 = 67, b1 = 1200;
	rectangle(img, Point(a1, b1), Point(a1+3, b1+3), Scalar(0, 0, 255), 8, 1);
	int a2 = 763, b2 = 630;
	rectangle(img, Point(a2, b2), Point(a2+3, b2+3), Scalar(0, 0, 255), 8, 1);
	int a3 = 1540, b3 = 1200;
	rectangle(img, Point(a3, b3), Point(a3+3, b3+3), Scalar(0, 0, 255), 8, 1);

	//warp the image to get a plan view of the road
	warpPerspective(input, output, warp_mat, output.size());
	
	//get the inverse matrix
	Mat inverse = Mat(3, 3, CV_32FC1);
	invert(warp_mat, inverse, CV_SVD);

	//write the inverse matrix to file
	FileStorage file2("warp_inverse.xml", FileStorage::WRITE);
	file2 << "inverse" <<  inverse;
	file2.release();

	//warp back to the original image
	//warpPerspective(output, output, inverse, output.size());	

	namedWindow("Output", CV_WINDOW_NORMAL);
	imshow("Output", output);
	namedWindow("Input", CV_WINDOW_NORMAL);
	imshow("Input", img);
	

	waitKey(0);
	return 0;
}
