// Test code to track highway lane markings
// Warp matrices used for perspective warp are generated by “warp.cpp”.
// Result: 1. Generates .cvs file to record the first x coordinate of left lane and right lane from the left
//	   2. Display the detected lane markings and overlay them by yellow lines.
//
// Author: Siqi Wu(Stella)
// Email: siqiw@g.clemson.edu
// Date: 06/21/2016

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

using namespace cv;

int main(){
	
	//read file stream
	//The names of image files in the folder must be consecutive to be able to read by sequence. Thus the file names start from "mono_0000002050.png" were changed to start from "mono_0000000348.png" 
	VideoCapture sequence("images/mono_%010d.png");
	
	//check if file stream is opened successfully.
	if (! sequence.isOpened()){
		std::cout << "error: cannot open image sequence." << std::endl;
		return 0;
	}
	
	//create Matrix to store images
	Mat input;
	Mat output;
	Mat imgCanny;

	//open “intercepts.csv” to write
	std::ofstream csvfile;
	csvfile.open("intercepts.csv");

	int count = 0;
	int key = 0;
	//process each image until the end of the image sequence
	while(key != 27){	
		bool success = sequence.read(input);
		//if it is the end of the sequence, jump out of the loop
		if (input.empty() || !success){
			std::cout << "End of sequence" << std::endl;
			break;
		}
	
		//write the file name
		//Because the file names start from “mono_0000002050.png” have been changed, we have to change them back when writing to the file 
		if (count < 348){
			csvfile << "mono_" << std::setfill('0') << std::setw(10) <<  count << ".png,";
		}else{
			csvfile << "mono_" << std::setfill('0') << std::setw(10) <<  count+1602 << ".png,";
		}
		count ++;

		//get perspective warp matrix
		//the warp matrix is calculated from warp.cpp
		Mat warp_mat = Mat(3, 3, CV_32FC1);
		FileStorage fs("warp_mat.xml", FileStorage::READ);
		fs["warp_mat"] >> warp_mat;
		fs.release();

		//perspective warp
		warpPerspective(input, output, warp_mat, output.size());

		//create region of interest
		Rect roi = Rect(660, 630, 150, 570);
		Mat img_roi = output(roi);


		//GaussianBlur(img_roi, img_roi, Size(3, 3), 1.5);
		//equalizeHist(img_roi, img_roi);

		//canny edge detection
		Canny(img_roi, imgCanny, 40, 110);
		//check if is lane marking
		std::cout << "img_roi val " << img_roi.at<double>(0,0) << std::endl; 
		/*for(int i = 0; i < img_roi.rows; i++){
			for(int j = 0; j < img_roi.cols; j++){
				if (img_roi.at<int>(i, j) < 100)
					imgCanny.at<int>(i, j) = 0;
			}
		}*/

		//Hough Line
		cv::Scalar color = Scalar(0, 255, 255);  //yellow
		std::vector<Vec4i> lines;
		std::vector<Point2f> lane_coor_start;
		std::vector<Point2f> lane_coor_end;
		HoughLinesP(imgCanny, lines, 1, CV_PI/180, 55, 40, 30);
			
		//select lines using slope	
		for(size_t i = 0; i < lines.size(); i++){
			double k = 100;
			Vec4i l = lines[i];
			int xcor = l[0] - l[2]; //assert difference of x coordinates is not 0
			if (xcor != 0)
				k = (double)(l[1] - l[3])/(double)(l[0] - l[2]);
			if  (xcor == 0 || (k > 1.6)||(k < -1.6)){
				//use the coordinates in the original image when marking lane lines
				lane_coor_start.push_back(Point(l[0]+660, l[1]+630)); 						lane_coor_end.push_back(Point(l[2]+660, l[3]+630));
			}
		}
		//get the middle line in front of the car to help identify left lane markers and right lane markers
		lane_coor_start.push_back(Point(660+58, 630));
		lane_coor_end.push_back(Point(660+58, 1200));
		
		//warp back to the original image
		Mat inverse = Mat(3, 3, CV_32FC1);
		FileStorage fs1("warp_inverse.xml", FileStorage::READ);
		fs1["inverse"] >> inverse;
		fs1.release();
		
		std::vector<Point2f> coor_start, coor_end;
		//Use perspective transform to warp the lane markers back to the original image
		if(lane_coor_start.size() != 0){
			perspectiveTransform(lane_coor_start, coor_start, inverse);		
			perspectiveTransform(lane_coor_end, coor_end, inverse);		
		}
		//convert the original image to BGR image
		cvtColor(input, input, CV_GRAY2BGR);
		
		//detect the x coordinates of the left lane marker and right lane marker
		int last = coor_start.size()-1;
		//line(input, coor_start[last], coor_end[last], Scalar(0,0,255), 6, CV_AA);
		int sx = coor_start[last].x, ex = coor_end[last].x;
		int lane_l = 1600, lane_r = 1600;
		bool found_l = false, found_r = false;
		for(size_t i = 0; i < last; i++){
			Point s = coor_start[i], e = coor_end[i];
			//draw lane marker lines in yellow
			line(input, s, e, color, 4, CV_AA);
			if (s.x < sx && s.x < ex && s.x > 0 && s.x < lane_l){
				lane_l = s.x;
				found_l = true;
			}
			if (e.x < sx && e.x < ex && e.x > 0 && e.x < lane_l){
				lane_l = e.x;
				found_l = true;
			}
			if (s.x > sx && s.x > ex && s.x < lane_r){
				lane_r = s.x;
				found_r = true;
			}
			if (e.x > sx && e.x > ex && e.x < lane_r){
				lane_r = e.x;
				found_r = true;
			}
		}

		//write left lane and right lane x coordinates
		if (found_l)
			csvfile << lane_l << ",";
		else
			csvfile << "None,";
		if (found_r)
			csvfile << lane_r;
		else
			csvfile << "None";

		csvfile << "\n";

		//show images
		namedWindow("Canny", CV_WINDOW_NORMAL);
		namedWindow("Input", CV_WINDOW_NORMAL);
		imshow("Canny", imgCanny);
		imshow("Input", input);

		key = waitKey(2);
	}
	//close csv file
	csvfile.close();
	waitKey(0);
	return 0;
}
