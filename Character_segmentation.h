#ifndef Character_segmentation_H_
#define Character_segmentation_H_

#include "plate_locate.h"

int Subtraction(Mat& a, Mat& b)
{
	char channelValue1, channelValue2;
	int sum = 0;
	for (int i = 0; i < a.rows; i++)
	{
		char* img1Ptr = a.ptr<char>(i);
		char* img2Ptr = b.ptr<char>(i);
		for (int j = 0; j < a.cols; j++)
		{
			channelValue1 = img1Ptr[j];
			channelValue2 = img2Ptr[j];
			sum += abs(channelValue1 - channelValue2);
		}
	}
	return sum;
}


//按照顺序读取图片
int ReadPic(Mat& in)
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
			cout << "can't open this image..." << endl;
			cout << filename << endl;
		}
		else
		{
			cout << "Success open this image..." << endl;
			cout << filename << endl;
			score = Subtraction(in, templatePic);
			return score;
		}
	}
}


int PixelSubtraction(vector<Mat>& in) 
{
	vector<Mat> templateImg;
	int result = 0, index = 0, lowest = 0;
	for (size_t i = 0; i < 12; i++)
	{
		string str = "";
		str += to_string(i);
		str += ".jpg";
		templateImg.push_back(imread(str));
	}

	for (int i = 0; i < in.size(); i++)
	{
		index = 0;
		for (int j = 0; j < templateImg.size() - 1; j++)
		{
			result = Subtraction(in[i], templateImg[j]); //获取第i个车牌号码和第j个模板的比较结果
			cout << "result :" << result << endl;
			if (j == 0)
				lowest = result;
			if (lowest > Subtraction(in[i], templateImg[j + 1])) //获取第i个车牌号码和第j+1个模板的比较结果，如果j+1更小，则记录下模板号码
			{
				lowest = Subtraction(in[i], templateImg[j + 1]);
				cout << "lowest :" << lowest << endl;
				index = j + 1;
			}
		}
		cout << "第" << i << "个车牌和第" << index+1 << "个模板重合率高" << endl;
	}
	return true;
}


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
				Mat roiImg = binImg(Range(0, srcImg.rows), Range(startIndex-2, endIndex + 2));
				Mat resizeRoiImg = roiImg;
				//调整图片尺寸为20*20
				resize(roiImg, resizeRoiImg, Size(20, 20));
				roiList.push_back(resizeRoiImg);
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

	PixelSubtraction(roiList);

	return;
}


void Pretreatment()
{
	cout << "Character_segmentation,Start..." << endl;
	
	Mat src = imread("plate_test.jpg");
	Mat srcImage = src;
	//将车牌resize为宽度136，高度36的矩形
	resize(src, srcImage, Size(136,36));

	if (srcImage.data == NULL)
	{
		cout << "can't open image..." << endl;
	}
	else
	{
		cout << "resize_image rows is " << srcImage.rows << endl;
		cout << "resize_image cols is " << srcImage.cols << endl;
		//imwrite("Resize Image.jpg", srcImage);
	}
	Mat input_grey;
	cvtColor(srcImage, input_grey, CV_BGR2GRAY);
	//imshow("Gray plate area", input_grey);

	Mat src_threshold;
	threshold(input_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	imshow("二值化", src_threshold);

	VerticalProjection(src_threshold, srcImage);

	

	return;
}


#endif // Character_segmentation_H_
#pragma once
