#ifndef Character_segmentation_H_
#define Character_segmentation_H_

#include "plate_locate.h"

int pixelSubtraction(Mat& a, Mat& b)
{
	int cols, rows;
	if (a.cols >= b.cols)
		cols = b.cols;
	else
		cols = a.cols;
	if (a.rows >= b.rows)
		rows = b.rows;
	else
		rows = a.rows;
	char channel_value_a, channel_value_b;
	int sum = 0;
	for (int i = 0; i < rows; i++)
	{
		char* a_row_ptr = a.ptr<char>(i);
		char* b_row_ptr = b.ptr<char>(i);
		for (int j = 0; j < cols; j++)
		{
			channel_value_a = a_row_ptr[j];
			channel_value_b = b_row_ptr[j];
			sum += abs(channel_value_a - channel_value_b);
		}
	}
	return sum;
}


//按照顺序读取图片
int readPic(Mat& in)
{
	int score = 0;
	string pattern_jpg = "D:/OpenCV_Code/PlateIdentify/template/*.jpg";
	vector<String> image_files;
	glob(pattern_jpg, image_files);
	if (image_files.size() == 0) 
	{
		cout << "No image files[jpg]" << endl;
	}
	//遍历读取模板图片
	for (int i = 0; i < image_files.size(); i++)
	{
		string filename = "";
		int num = i;
		filename += to_string(i);
		String jpg = ".jpg";
		filename = filename + jpg;
		Mat templatePic = imread(filename);
		if (templatePic.data == NULL)
		{
			//cout << "can't open this image..." << endl;
			//cout << filename << endl;
		}
		else
		{
			//cout << "Success open this image..." << endl;
			//cout << filename << endl;
			score = pixelSubtraction(in, templatePic);
			return score;
		}
	}
}


int charSubtraction(vector<Mat>& in)
{
	vector<vector<Mat>> template_pics;
	for (int i = 0; i < 65; i++)
	{
		vector<Mat> a;
		template_pics.push_back(a);
	}
	String base_pic_path = "D:\\OpenCV_Code\\EasyPR\\resources\\train\\ann\\";
	String pic_path;
	String tail = "\\*.jpg";
	vector<String> pic_list;
	for (int i = 0; i < 65; i++)
	{
		pic_path = base_pic_path + kChars[i] + tail;
		glob(pic_path, pic_list);
		if (pic_list.size() == 0)
		{
			cout << "No template image files[jpg]!" << endl;
			return -1;
		}
		for (size_t j = 0; j < pic_list.size(); j++)
		{
			template_pics[i].push_back(imread(pic_list[j]));
			cvtColor(template_pics[i][j], template_pics[i][j], COLOR_BGR2GRAY);
			threshold(template_pics[i][j], template_pics[i][j], 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
		}
	}
	double result = 0, lowest = 0;
	int index;
	cout << "----------------- " << endl;
	cout << "The recognized license plate is： " ;
	for (int i = 0; i < in.size(); i++)
	{
		index = 0;
		for (int k = 0; k < template_pics.size(); k++)
		{
			for (int j = 0; j < template_pics[k].size(); j++)
			{
				result += pixelSubtraction(in[i], template_pics[k][j]);
			}
			result = result / template_pics[k].size(); 
			if (k == 0)
				lowest = result;
			if (lowest > result) 
			{
				lowest = result;
				index = k;
			}
			result = 0;
		}
		cout << kChars[index];
		
	}
	cout << endl;
	cout << "----------------- " << endl;
	return true;
}


//垂直投影，分割字符
void verticalProjection(Mat& binImg, Mat& srcImg)
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
			cout << "Distance:" << endIndex - startIndex << endl;
			cout << endl;
			//判断“.”
			if (endIndex - startIndex <= DOTLENGTH)
			{
				isDot = true;
			}
			else
			{
				isDot = false;
			}
			if (!isDot)
			{
				Mat roiImg = binImg(Range(0, srcImg.rows), Range(startIndex-2, endIndex+2));
				Mat resizeRoiImg = roiImg;
				//调整图片尺寸为20*20
				resize(roiImg, resizeRoiImg, Size(20, 20));
				roiList.push_back(resizeRoiImg);
				//roiList.push_back(roiImg);
			}
			
		}
	}

	//保存字符到文件
	for (int i = 0; i <roiList.size(); i++)
	{
		string filename = "";
		char num = i;
		filename += to_string(i+1);
		String jpg = ".jpg";
		filename = filename + jpg;
		imwrite(filename, roiList[i]);
		imshow(filename, roiList[i]);
	}

	charSubtraction(roiList);


	return;
}


void preTreatment()
{
	cout << "Character_segmentation,Start..." << endl;
	
	Mat srcImage = imread("su.jpg");
	//Mat srcImage = src;
	//将车牌resize为宽度136，高度36的矩形
	//resize(src, srcImage, Size(136,36));

	if (srcImage.data == NULL)
	{
		cout << "can't open image..." << endl;
	}
	else
	{
		cout << "resize_image rows is " << srcImage.rows << endl;
		cout << "resize_image cols is " << srcImage.cols << endl;
		cout << endl;
		//imwrite("Resize Image.jpg", srcImage);
	}
	Mat input_grey;
	cvtColor(srcImage, input_grey, CV_BGR2GRAY);
	//imshow("Gray plate area", input_grey);

	Mat src_threshold;
	threshold(input_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	imshow("二值化", src_threshold);

	verticalProjection(src_threshold, srcImage);

	

	return;
}


#endif // Character_segmentation_H_
#pragma once
