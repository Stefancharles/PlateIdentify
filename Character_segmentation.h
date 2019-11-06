#ifndef Character_segmentation_H_
#define Character_segmentation_H_

#include "plate_locate.h"


//垂直投影，分割字符
void VerticalProjection(Mat& binImg, Mat& srcImg)
{
	long width = binImg.cols;
	long height = binImg.rows;
	int perPixelValue;//每个像素的值
	int* projectValArry = new int[width];//创建一个用于储存每列白色像素个数的数组
	memset(projectValArry, 0, width * 4);//必须初始化数组

    //遍历每一列的图像灰度值，查找每一行255的值
	for (int col = 0; col < width; ++col)
	{
		for (int row = 0; row < height; ++row)
		{
			perPixelValue = binImg.at<uchar>(row, col);
			if (perPixelValue == 255)//如果是黑底白字
			{
				projectValArry[col]++;
			}
		}
	}

	/*新建一个Mat用于储存投影直方图并将背景置为白色*/
	Mat verticalProjectionMat(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			perPixelValue = 255;  //背景设置为白色。   
			verticalProjectionMat.at<uchar>(i, j) = perPixelValue;
		}
	}

	/*将直方图的曲线设为黑色*/
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < projectValArry[i]; j++)
		{
			perPixelValue = 0;  //直方图设置为黑色  
			verticalProjectionMat.at<uchar>(height - 1 - j, i) = perPixelValue;
		}
	}
	imshow("投影", verticalProjectionMat);
	//delete[] projectValArry;//删除数组空间

	vector<Mat> roiList;//用于储存分割出来的每个字符
	int startIndex = 0;//记录进入字符区的索引
	int endIndex = 0;//记录进入空白区域的索引
	bool inblock = false;//是否遍历到了字符区内
	bool isDot = false;//是否是点
	for (int i = 0; i < srcImg.cols; ++i)
	{
		if (!inblock && projectValArry[i] != 0) //进入有字符区域
		{
			inblock = true;
			startIndex = i;
			cout << "startIndex:" << startIndex << endl;
		}
		if (inblock && projectValArry[i] == 0) //进入空白区
		{
			endIndex = i;
			inblock = false;
			cout << "endIndex:" << endIndex << endl;
			cout << endl;
			if (endIndex - startIndex <= 3)
			{
				isDot = true;
			}
			else
			{
				isDot = false;
			}
			if (!isDot)
			{
				Mat roiImg = binImg(Range(0, srcImg.rows), Range(startIndex, endIndex + 1));
				roiList.push_back(roiImg);
			}
			
		}
	}
	//保存字符到文件
	for (int i = 0; i <roiList.size(); i++)
	{
		string filename = "";
		char num = i;
		filename += to_string(i);
		String jpg = "bin.jpg";
		filename = filename + jpg;
		imwrite(filename, roiList[i]);
		imshow(filename, roiList[i]);
	}
	
}


void Pretreatment()
{
	cout << "Character_segmentation,Start..." << endl;

	Mat srcImage = imread("plate_test.jpg");

	if (srcImage.data == NULL)
	{
		cout << "can't open image..." << endl;
	}
	else
	{
		cout << "input_image rows is " << srcImage.rows << endl;
		cout << "input_image cols is " << srcImage.cols << endl;
		//imshow("Source Image", srcImage);
	}
	Mat input_grey;
	cvtColor(srcImage, input_grey, CV_BGR2GRAY);
	//imshow("Gray plate area", input_grey);

	Mat src_threshold;
	threshold(input_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	imshow("二值化", src_threshold);

	VerticalProjection(src_threshold, srcImage);

	



}


#endif // Character_segmentation_H_
#pragma once
