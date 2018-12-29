/*
Copyright 2011. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libelas.
Authors: Andreas Geiger

libelas is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or any later version.

libelas is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libelas; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA 
*/

// Demo program showing how libelas can be used, try "./elas -h" for help

#include <iostream>
#include <algorithm>
#include <time.h>
#include "elas.h"
#include "image.h"

#include<opencv.hpp>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world400d.lib")
#else
#pragma comment(lib, "opencv_world400.lib")
#endif

using namespace std;

// compute disparities of image input pair file_1, file_2
int process (const std::string& file_1, const std::string& file_2) 
{
	//load image
	cv::Mat imgL = cv::imread(file_1, cv::IMREAD_GRAYSCALE);
	cv::Mat imgR = cv::imread(file_2, cv::IMREAD_GRAYSCALE);

	
	if (imgL.empty() && imgR.empty())
	{
		std::cout << "The image pairs is not exist!" << std::endl;
		return 0;
	}

	// check for correct size
	if (imgL.size() != imgR.size())
	{
		std::cout << "The size of left image is not equal to the right" << std::endl;
		return 0;
	}


	// get image width and height
	int32_t width = imgL.cols;
	int32_t height = imgL.rows;


	// allocate memory for disparity images
	const int32_t dims[3] = { width, height, width }; // bytes per line = width
	float* D1_data = (float*)malloc(width*height*sizeof(float));
	float* D2_data = (float*)malloc(width*height*sizeof(float));

	uint8_t* I1_Data = (uint8_t*)malloc(width*height*sizeof(uint8_t));
	uint8_t* I2_Data = (uint8_t*)malloc(width*height*sizeof(uint8_t));

	memcpy(I1_Data, imgL.data, width*height*sizeof(uint8_t));
	memcpy(I2_Data, imgR.data, width*height*sizeof(uint8_t));


	// process
	Elas::parameters param;
	param.postprocess_only_left = false;
	Elas elas(param);
	elas.process(I1_Data, I2_Data, D1_data, D2_data, dims);

	// find maximum disparity for scaling output disparity images to [0..255]
	float disp_max = 0;
	for (int32_t i = 0; i < width*height; i++) 
	{
		if (D1_data[i] > disp_max) 
			disp_max = D1_data[i];
		if (D2_data[i] > disp_max) 
			disp_max = D2_data[i];
	}

	// copy float to uchar
	image<uchar> *D1 = new image<uchar>(width, height);
	image<uchar> *D2 = new image<uchar>(width, height);
	for (int32_t i = 0; i < width*height; i++) 
	{
		D1->data[i] = (uint8_t)std::max(255.0*D1_data[i] / disp_max, 0.0);
		D2->data[i] = (uint8_t)std::max(255.0*D2_data[i] / disp_max, 0.0);
	}

	// save disparity images
	char output_1[1024];
	char output_2[1024];
	strncpy(output_1, file_1.c_str(), strlen(file_1.c_str()) - 4);
	strncpy(output_2, file_2.c_str(), strlen(file_2.c_str()) - 4);
	output_1[strlen(file_1.c_str()) - 4] = '\0';
	output_2[strlen(file_2.c_str()) - 4] = '\0';
	strcat(output_1, "_disp.png");
	strcat(output_2, "_disp.png");

	cv::Mat leftDisp(height, width, CV_8UC1, D1->data);
	cv::Mat righDisp(height, width, CV_8UC1, D2->data);
 	cv::namedWindow("leftDisp");
 	cv::namedWindow("rightDisp");
 	cv::imshow("leftDisp", leftDisp);
 	cv::imshow("rightDisp", righDisp);
 	cv::waitKey(10);


	//save image
	cv::imwrite(output_1, leftDisp);
	cv::imwrite(output_2, righDisp);
	

	// free memory
	delete I1_Data;
	delete I2_Data;
	delete D1;
	delete D2;
	free(D1_data);
	free(D2_data);
}

int main ()
{
	clock_t starttime = clock();
	
	std::string dir = "E:/";
	int flag = 0;
// 	for (int frameno = 0;; frameno++)
// 	{
// 		char base_name[256];
// 		sprintf(base_name, "%06d.png", frameno);
// 		std::string bufl = dir + "1/testing/0006/" + base_name;
// 		std::string bufr = dir + "2/testing/0006/" + base_name;
// 		std::cout << " " << frameno << std::endl;
// 		flag = process(bufl, bufr);
// 		if (flag == 0)
// 		{
// 			frameno = 0;
// 		}
// 	}
	std::string buf_left = "L0.png";
	std::string buf_right = "R0.png";
	flag = process(buf_left, buf_right);

	clock_t endtime = clock();
	cout << "time is " << double(endtime - starttime) / CLOCKS_PER_SEC << endl;
	cout << "... done!" << endl;
	system("pause");
	return 0;
}


