#ifndef SOBLE_H_
#define SOBLE_H_

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "iostream"
#include "plate_locate.h"

using namespace cv;
using namespace std;

int SobelPlateLocate(String pic_name)
{
	cout << "SobelPlateLocate,Start..." << endl;

	Mat srcImage = imread(pic_name);

	if (srcImage.data == NULL)
	{
		return -1;
		cout << "can't open image..." << endl;
	}
	else
	{
		cout << "input_image rows is " << srcImage.rows << endl;
		cout << "input_image cols is " << srcImage.cols << endl;
		imshow("Source Image", srcImage);
	}

	//TODO:GaussianBlur:对图像去噪，为边缘检测算法做准备。

	//NOTE:高斯模糊，再进行灰度化。基于色彩的高斯模糊过程比灰度后的高斯模糊过程更容易检测到边缘点。
	Mat mat_blur, mat_copy;
	mat_blur = srcImage.clone();
	mat_copy = srcImage.clone();
	GaussianBlur(srcImage, mat_blur, Size(5, 5), 0, 0, BORDER_DEFAULT);
	imshow("GaussianBlur", mat_blur);


	//TODO:Gray:为边缘检测算法准备灰度化环境。

	//NOTE:很多图像处理算法仅仅只适用于灰度图像，例如Sobel算子。
	Mat mat_gray;
	if (mat_blur.channels() == 3)
		cvtColor(mat_blur, mat_gray, CV_RGB2GRAY);
	else
		mat_gray = mat_blur;
	imshow("Gray", mat_gray);


	//TODO:Sobel算子:检测图像中的垂直边缘，便于区分车牌。

	//NOTE:通过Sobel算子，将车牌中的字符与车的背景明显区分开来，为后面的二值化与闭操作打下基础.

	int scale = SOBEL_SCALE;
	int delta = SOBEL_DELTA;
	int ddepth = SOBEL_DDEPTH;

	//创建grad_x和grad_y矩阵
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	//求X方向梯度
	Sobel(mat_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("X方向梯度", abs_grad_x);
	//测试Y方向梯度
	//Sobel(mat_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	//convertScaleAbs(grad_y, abs_grad_y);
	//imshow("Y方向梯度", abs_grad_y);


	//NOTE:仅做水平方向求导，而不做垂直方向求导。这样做的意义是，如果我们做了垂直方向求导，会检测出很多水平边缘。
	Mat grad;
	addWeighted(abs_grad_x, SOBEL_X_WEIGHT, 0, 0, 0, grad);
	imshow("整体方向Sobel", grad);

	//TODO:二值化：为后续的形态学算子Morph等准备二值化的图像。
	/*
	* CV_THRESH_OTSU:自适应阈值
	* CV_THRESH_BINARY:正二值化:像素的值越接近0，越可能被赋值为0，反之则为1
	* CV_THRESH_BINARY_INV:反二值化
	* 正二值化处理蓝牌，反二值化处理黄牌
	* 蓝牌字符浅，背景深，黄牌则是字符深，背景浅
	* 对图像的每个像素做一个阈值处理
	* 与灰度图像仔细区分下，二值化图像中的白色是没有颜色强与暗的区别
	*/
	Mat mat_threshold;
	double otsu_thresh_val =
		threshold(grad, mat_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
	imshow("二值化结果", mat_threshold);


	//TODO：闭操作：将车牌字母连接成为一个连通域，便于取轮廓。　
	Mat element = getStructuringElement(MORPH_RECT, Size(SOBEL_morphW, SOBEL_morphH));
	morphologyEx(mat_threshold, mat_threshold, MORPH_CLOSE, element);
	imshow("闭操作结果", mat_threshold);

	//TODO：取轮廓：将连通域的外围勾画出来，便于形成外接矩形。　
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(mat_threshold,
		contours,               // a vector of contours
		hierarchy,
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE);  // all pixels of each contours

	//TODO:绘制取轮廓后的结果

	int index = 0;
	for (; index >= 0; index = hierarchy[index][0])
	{
		Scalar color(rand() & 255, rand() & 255, rand() & 255);
		drawContours(mat_copy,
			contours,
			index,
			color,
			2,
			8,
			hierarchy);
	}

	imshow("轮廓图", mat_copy);
	Rect rec_adapt;//矩形区域
	cout << "Total contours is :" << contours.size() << endl;
	for (size_t i = 0; i < contours.size(); i++)
	{
		//----矩形区域非零像素占总的比例，防止有较大的空白区域干扰检测结果
		//----矩形的长宽限制，也可以再增加额外条件：长宽比例等
		//countNonZero 可以得到非零像素点的个数。
		//boundingRect 计算轮廓的最小外接矩形

		int true_pix_count = countNonZero(mat_threshold(boundingRect(contours[i])));
		double true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(boundingRect(contours[i]).area());
		if (boundingRect(contours[i]).height > 10 && boundingRect(contours[i]).width > 80 && true_pix_rate > 0.7)
		{
			rec_adapt = boundingRect(contours[i]);
			drawContours(mat_copy, contours, static_cast<int>(i), Scalar(0, 0, 255), 1);
			drawContours(mat_threshold, contours, static_cast<int>(i), Scalar(200, 200, 0), 2);
		}

	}
	imshow("轮廓图", mat_copy);

	//TODO：在原图中把车牌区域显示出来,保存车牌图片文件
	Mat mat_plate;
	mat_plate = srcImage(rec_adapt);


	//imwrite("plate_test_rotation.jpg", mat_plate);
	//imshow("车牌", mat_plate);
	//cout << "mat_plate rows is " << mat_plate.rows << endl;
	//cout << "mat_plate cols is " << mat_plate.cols << endl;
	waitKey(0);
}






#endif // !SOBLE_H_
#pragma once
