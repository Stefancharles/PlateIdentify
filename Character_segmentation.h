#ifndef Character_segmentation_H_
#define Character_segmentation_H_

#include "plate_locate.h"


//��ֱͶӰ���ָ��ַ�
void VerticalProjection(Mat& binImg, Mat& srcImg)
{
	long width = binImg.cols;
	long height = binImg.rows;
	int perPixelValue;//ÿ�����ص�ֵ
	int* projectValArry = new int[width];//����һ�����ڴ���ÿ�а�ɫ���ظ���������
	memset(projectValArry, 0, width * 4);//�����ʼ������

    //����ÿһ�е�ͼ��Ҷ�ֵ������ÿһ��255��ֵ
	for (int col = 0; col < width; ++col)
	{
		for (int row = 0; row < height; ++row)
		{
			perPixelValue = binImg.at<uchar>(row, col);
			if (perPixelValue == 255)//����Ǻڵװ���
			{
				projectValArry[col]++;
			}
		}
	}

	/*�½�һ��Mat���ڴ���ͶӰֱ��ͼ����������Ϊ��ɫ*/
	Mat verticalProjectionMat(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			perPixelValue = 255;  //��������Ϊ��ɫ��   
			verticalProjectionMat.at<uchar>(i, j) = perPixelValue;
		}
	}

	/*��ֱ��ͼ��������Ϊ��ɫ*/
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < projectValArry[i]; j++)
		{
			perPixelValue = 0;  //ֱ��ͼ����Ϊ��ɫ  
			verticalProjectionMat.at<uchar>(height - 1 - j, i) = perPixelValue;
		}
	}
	imshow("ͶӰ", verticalProjectionMat);
	//delete[] projectValArry;//ɾ������ռ�

	vector<Mat> roiList;//���ڴ���ָ������ÿ���ַ�
	int startIndex = 0;//��¼�����ַ���������
	int endIndex = 0;//��¼����հ����������
	bool inblock = false;//�Ƿ���������ַ�����
	bool isDot = false;//�Ƿ��ǵ�
	for (int i = 0; i < srcImg.cols; ++i)
	{
		if (!inblock && projectValArry[i] != 0) //�������ַ�����
		{
			inblock = true;
			startIndex = i;
			cout << "startIndex:" << startIndex << endl;
		}
		if (inblock && projectValArry[i] == 0) //����հ���
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
	//�����ַ����ļ�
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

	imshow("��ֵ��", src_threshold);

	VerticalProjection(src_threshold, srcImage);

	



}


#endif // Character_segmentation_H_
#pragma once
