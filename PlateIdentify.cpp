#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "iostream"

using namespace cv;
using namespace std;

static const int SOBEL_SCALE = 1;
static const int SOBEL_DELTA = 0;
static const int SOBEL_DDEPTH = CV_16S;
static const int SOBEL_X_WEIGHT = 1;

int main(int argc, char** argv)
{
	cout << "test_plate_locate" << endl;

	Mat srcImage = imread("plate_judge.jpg");

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
	Mat mat_blur;
	mat_blur = srcImage.clone();
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

	//TODO:为后续的形态学算子Morph等准备二值化的图像。
	/*
	CV_THRESH_OTSU:自适应阈值
	CV_THRESH_BINARY:正二值化:像素的值越接近0，越可能被赋值为0，反之则为1
	CV_THRESH_BINARY_INV:反二值化
	正二值化处理蓝牌，反二值化处理黄牌
	蓝牌字符浅，背景深，黄牌则是字符深，背景浅
	*/
	Mat mat_threshold;
	double otsu_thresh_val =
		threshold(grad, mat_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
	imshow("二值化结果", mat_threshold);




	waitKey(0);
	return 0;
}