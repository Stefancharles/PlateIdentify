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


//����˳���ȡͼƬ
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
	//������ȡģ��ͼƬ
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
			result = Subtraction(in[i], templateImg[j]); //��ȡ��i�����ƺ���͵�j��ģ��ıȽϽ��
			cout << "result :" << result << endl;
			if (j == 0)
				lowest = result;
			if (lowest > Subtraction(in[i], templateImg[j + 1])) //��ȡ��i�����ƺ���͵�j+1��ģ��ıȽϽ�������j+1��С�����¼��ģ�����
			{
				lowest = Subtraction(in[i], templateImg[j + 1]);
				cout << "lowest :" << lowest << endl;
				index = j + 1;
			}
		}
		cout << "��" << i << "�����ƺ͵�" << index+1 << "��ģ���غ��ʸ�" << endl;
	}
	return true;
}


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
				Mat roiImg = binImg(Range(0, srcImg.rows), Range(startIndex-2, endIndex + 2));
				Mat resizeRoiImg = roiImg;
				//����ͼƬ�ߴ�Ϊ20*20
				resize(roiImg, resizeRoiImg, Size(20, 20));
				roiList.push_back(resizeRoiImg);
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

	PixelSubtraction(roiList);

	return;
}


void Pretreatment()
{
	cout << "Character_segmentation,Start..." << endl;
	
	Mat src = imread("plate_test.jpg");
	Mat srcImage = src;
	//������resizeΪ���136���߶�36�ľ���
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

	imshow("��ֵ��", src_threshold);

	VerticalProjection(src_threshold, srcImage);

	

	return;
}


#endif // Character_segmentation_H_
#pragma once
