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

	//TODO:GaussianBlur:��ͼ��ȥ�룬Ϊ��Ե����㷨��׼����

	//NOTE:��˹ģ�����ٽ��лҶȻ�������ɫ�ʵĸ�˹ģ�����̱ȻҶȺ�ĸ�˹ģ�����̸����׼�⵽��Ե�㡣
	Mat mat_blur, mat_copy;
	mat_blur = srcImage.clone();
	mat_copy = srcImage.clone();
	GaussianBlur(srcImage, mat_blur, Size(5, 5), 0, 0, BORDER_DEFAULT);
	imshow("GaussianBlur", mat_blur);


	//TODO:Gray:Ϊ��Ե����㷨׼���ҶȻ�������

	//NOTE:�ܶ�ͼ�����㷨����ֻ�����ڻҶ�ͼ������Sobel���ӡ�
	Mat mat_gray;
	if (mat_blur.channels() == 3)
		cvtColor(mat_blur, mat_gray, CV_RGB2GRAY);
	else
		mat_gray = mat_blur;
	imshow("Gray", mat_gray);


	//TODO:Sobel����:���ͼ���еĴ�ֱ��Ե���������ֳ��ơ�

	//NOTE:ͨ��Sobel���ӣ��������е��ַ��복�ı����������ֿ�����Ϊ����Ķ�ֵ����ղ������»���.

	int scale = SOBEL_SCALE;
	int delta = SOBEL_DELTA;
	int ddepth = SOBEL_DDEPTH;

	//����grad_x��grad_y����
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	//��X�����ݶ�
	Sobel(mat_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	imshow("X�����ݶ�", abs_grad_x);
	//����Y�����ݶ�
	//Sobel(mat_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	//convertScaleAbs(grad_y, abs_grad_y);
	//imshow("Y�����ݶ�", abs_grad_y);


	//NOTE:����ˮƽ�����󵼣���������ֱ�����󵼡��������������ǣ�����������˴�ֱ�����󵼣�������ܶ�ˮƽ��Ե��
	Mat grad;
	addWeighted(abs_grad_x, SOBEL_X_WEIGHT, 0, 0, 0, grad);
	imshow("���巽��Sobel", grad);

	//TODO:��ֵ����Ϊ��������̬ѧ����Morph��׼����ֵ����ͼ��
	/*
	* CV_THRESH_OTSU:����Ӧ��ֵ
	* CV_THRESH_BINARY:����ֵ��:���ص�ֵԽ�ӽ�0��Խ���ܱ���ֵΪ0����֮��Ϊ1
	* CV_THRESH_BINARY_INV:����ֵ��
	* ����ֵ���������ƣ�����ֵ���������
	* �����ַ�ǳ����������������ַ������ǳ
	* ��ͼ���ÿ��������һ����ֵ����
	* ��Ҷ�ͼ����ϸ�����£���ֵ��ͼ���еİ�ɫ��û����ɫǿ�밵������
	*/
	Mat mat_threshold;
	double otsu_thresh_val =
		threshold(grad, mat_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
	imshow("��ֵ�����", mat_threshold);


	//TODO���ղ�������������ĸ���ӳ�Ϊһ����ͨ�򣬱���ȡ��������
	Mat element = getStructuringElement(MORPH_RECT, Size(SOBEL_morphW, SOBEL_morphH));
	morphologyEx(mat_threshold, mat_threshold, MORPH_CLOSE, element);
	imshow("�ղ������", mat_threshold);

	//TODO��ȡ����������ͨ�����Χ���������������γ���Ӿ��Ρ���
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(mat_threshold,
		contours,               // a vector of contours
		hierarchy,
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE);  // all pixels of each contours

	//TODO:����ȡ������Ľ��

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

	imshow("����ͼ", mat_copy);
	Rect rec_adapt;//��������
	cout << "Total contours is :" << contours.size() << endl;
	for (size_t i = 0; i < contours.size(); i++)
	{
		//----���������������ռ�ܵı�������ֹ�нϴ�Ŀհ�������ż����
		//----���εĳ������ƣ�Ҳ���������Ӷ������������������
		//countNonZero ���Եõ��������ص�ĸ�����
		//boundingRect ������������С��Ӿ���

		int true_pix_count = countNonZero(mat_threshold(boundingRect(contours[i])));
		double true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(boundingRect(contours[i]).area());
		if (boundingRect(contours[i]).height > 10 && boundingRect(contours[i]).width > 80 && true_pix_rate > 0.7)
		{
			rec_adapt = boundingRect(contours[i]);
			drawContours(mat_copy, contours, static_cast<int>(i), Scalar(0, 0, 255), 1);
			drawContours(mat_threshold, contours, static_cast<int>(i), Scalar(200, 200, 0), 2);
		}

	}
	imshow("����ͼ", mat_copy);

	//TODO����ԭͼ�аѳ���������ʾ����,���泵��ͼƬ�ļ�
	Mat mat_plate;
	mat_plate = srcImage(rec_adapt);


	//imwrite("plate_test_rotation.jpg", mat_plate);
	//imshow("����", mat_plate);
	//cout << "mat_plate rows is " << mat_plate.rows << endl;
	//cout << "mat_plate cols is " << mat_plate.cols << endl;
	waitKey(0);
}






#endif // !SOBLE_H_
#pragma once
