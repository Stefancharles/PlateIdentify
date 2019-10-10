#include"plate_locate.h"


int SobelPlateLocate()
{
	cout << "SobelPlateLocate,Start..." << endl;

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
	/*
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

	imshow("轮廓图", mat_copy);*/

	Rect rec_adapt;//矩形区域
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
	imwrite("plate_test.jpg", mat_plate);
	imshow("车牌", mat_plate);
	waitKey(0);
}

/* 根据一幅图像与颜色模板获取对应的二值图
 * 输入RGB图像, 颜色模板（蓝色、黄色）
 * 输出灰度图（只有0和255两个值，255代表匹配，0代表不匹配）
 * 色调（H），饱和度（S），亮度（V）
 * 第一步，将图像的颜色空间从RGB转为HSV，在这里由于光照的影响，对于图像使用直方图均衡进行预处理；
 * 第二步，依次遍历图像的所有像素，当H值落在200-280之间并且S值与V值也落在0.35-1.0之间，标记为白色像素，否则为黑色像素；
 * 第三步，对仅有白黑两个颜色的二值图参照原先车牌定位中的方法，使用闭操作，
   取轮廓等方法将车牌的外接矩形截取出来做进一步的处理。*/
Mat colorMatch(const Mat& src, Mat& match, const Color r, const bool adaptive_minsv)
{
	// S和V的最小值由adaptive_minsv这个bool值判断
	// 如果为true，则最小值取决于H值，按比例衰减
	// 如果为false，则不再自适应，使用固定的最小值minabs_sv
	// 默认为false
	const float max_sv = 255;
	const float minref_sv = 64;

	const float minabs_sv = 95;

	//blue的H范围
	const int min_blue = 100;  
	const int max_blue = 140;  

	//yellow的H范围
	const int min_yellow = 15; 
	const int max_yellow = 40; 

	//white的H范围
	const int min_white = 0;   // 15
	const int max_white = 30;  // 40

	Mat src_hsv;
	// 转到HSV空间进行处理，颜色搜索主要使用的是H分量进行蓝色与黄色的匹配工作
	cvtColor(src, src_hsv, CV_BGR2HSV);

	vector<Mat> hsvSplit;
	split(src_hsv, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);//直方图均衡化，用于提高图像的质量
	merge(hsvSplit, src_hsv);//合并通道

	//匹配模板基色,切换以查找想要的基色
	int min_h = 0;
	int max_h = 0;
	switch (r) {
	case BLUE:
		min_h = min_blue;
		max_h = max_blue;
		break;
	case YELLOW:
		min_h = min_yellow;
		max_h = max_yellow;
		break;
	case WHITE:
		min_h = min_white;
		max_h = max_white;
		break;
	default:
		//Color::UNKNOWN;
		break;
	}

	float diff_h = float((max_h - min_h) / 2);
	int avg_h = min_h + diff_h;

	int channels = src_hsv.channels();
	int nRows = src_hsv.rows;
	//图像数据列需要考虑通道数的影响；
	int nCols = src_hsv.cols * channels;

	if (src_hsv.isContinuous())//连续存储的数据，按一行处理
	{
		nCols *= nRows;
		nRows = 1;
	}

	int i, j;
	uchar* p;
	float s_all = 0;
	float v_all = 0;
	float count = 0;
	for (i = 0; i < nRows; ++i)
	{
		p = src_hsv.ptr<uchar>(i);
		for (j = 0; j < nCols; j += 3)
		{
			int H = int(p[j]); //0-180
			int S = int(p[j + 1]);  //0-255
			int V = int(p[j + 2]);  //0-255

			s_all += S;
			v_all += V;
			count++;

			bool colorMatched = false;

			if (H > min_h&& H < max_h)
			{
				int Hdiff = 0;
				if (H > avg_h)
					Hdiff = H - avg_h;
				else
					Hdiff = avg_h - H;

				float Hdiff_p = float(Hdiff) / diff_h;

				// S和V的最小值由adaptive_minsv这个bool值判断
				// 如果为true，则最小值取决于H值，按比例衰减
				// 如果为false，则不再自适应，使用固定的最小值minabs_sv
				float min_sv = 0;
				if (true == adaptive_minsv)
					min_sv = minref_sv - minref_sv / 2 * (1 - Hdiff_p); // inref_sv - minref_sv / 2 * (1 - Hdiff_p)
				else
					min_sv = minabs_sv; // add

				if ((S > min_sv&& S < max_sv) && (V > min_sv&& V < max_sv))
					colorMatched = true;
			}

			if (colorMatched == true) {
				p[j] = 0; p[j + 1] = 0; p[j + 2] = 255;
			}
			else {
				p[j] = 0; p[j + 1] = 0; p[j + 2] = 0;
			}
		}
	}

	//cout << "avg_s:" << s_all / count << endl;
	//cout << "avg_v:" << v_all / count << endl;

	// 获取颜色匹配后的二值灰度图
	Mat src_grey;
	vector<Mat> hsvSplit_done;
	split(src_hsv, hsvSplit_done);
	src_grey = hsvSplit_done[2];

	match = src_grey;

	return src_grey;
}


int ColorPlateLocate()
{
	cout << "ColorPlateLocate,Start..." << endl;

	Mat srcImage = imread("colortest.jpg");
	Mat mat_copy;
	mat_copy = srcImage.clone();
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

	Mat match_grey;

	colorMatch(srcImage, match_grey, BLUE, false);
	imshow("mat_gray", match_grey);

	Mat src_threshold;
	threshold(match_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	Mat element = getStructuringElement(MORPH_RECT, Size(color_morphW, color_morphH));
	morphologyEx(src_threshold, src_threshold, MORPH_CLOSE, element);
	imshow("闭操作结果", src_threshold);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_threshold,
		contours,               // a vector of contours
		hierarchy,
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE);  // all pixels of each contours

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
	for (size_t i = 0; i < contours.size(); i++)
	{
		//----矩形区域非零像素占总的比例，防止有较大的空白区域干扰检测结果
		//----矩形的长宽限制，也可以再增加额外条件：长宽比例等
		//countNonZero 可以得到非零像素点的个数。
		//boundingRect 计算轮廓的最小外接矩形

		int true_pix_count = countNonZero(src_threshold(boundingRect(contours[i])));
		double true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(boundingRect(contours[i]).area());
		if (boundingRect(contours[i]).height > 10 && boundingRect(contours[i]).width > 80 && true_pix_rate > 0.7)
		{
			rec_adapt = boundingRect(contours[i]);
			drawContours(mat_copy, contours, static_cast<int>(i), Scalar(0, 0, 255), 1);
			drawContours(src_threshold, contours, static_cast<int>(i), Scalar(200, 200, 0), 2);
		}
	}

	Mat mat_plate;
	mat_plate = srcImage(rec_adapt);
	imshow("车牌", mat_plate);
	imwrite("plate_color_test.jpg", mat_plate);
	waitKey(0);
}



int main(int argc, char** argv)
{
	int choice;
	cout << "Input Your choice:" << endl;
	cout << "1.Run SobelPlateLocate." << endl;
	cout << "2.Run ColorPlateLocate." << endl;
	cin >> choice;
	switch (choice)
	{
	case 1:
		SobelPlateLocate();
		break;
	case 2:
		ColorPlateLocate();
		break;
	default:
		cout << "Input error." << endl;
		break;
	}
	
	
	return 0;
}