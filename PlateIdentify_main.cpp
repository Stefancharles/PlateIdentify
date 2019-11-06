#include"plate_locate.h"
#include "Character_segmentation.h"

bool CacuSlope(const Mat& in, const double angle, double& slope)
{
	int nRows = in.rows;
	int nCols = in.cols;

	assert(in.channels() == 1);

	int comp_index[3];
	int len[3];

	comp_index[0] = nRows / 4;
	comp_index[1] = nRows / 4 * 2;
	comp_index[2] = nRows / 4 * 3;

	const uchar* p;

	for (int i = 0; i < 3; i++) {
		int index = comp_index[i];
		p = in.ptr<uchar>(index);

		int j = 0;
		int value = 0;
		while (0 == value && j < nCols) value = int(p[j++]);

		len[i] = j;
	}

	 cout << "len[0]:" << len[0] << endl;
	 cout << "len[1]:" << len[1] << endl;
	 cout << "len[2]:" << len[2] << endl;

	// len[0]/len[1]/len[2] are used to calc the slope

	double maxlen = max(len[2], len[0]);
	double minlen = min(len[2], len[0]);
	double difflen = abs(len[2] - len[0]);

	double PI = 3.14159265;

	double g = tan(angle * PI / 180.0);

	if (maxlen - len[1] > nCols / 32 || len[1] - minlen > nCols / 32) {

		double slope_can_1 =
			double(len[2] - len[0]) / double(comp_index[1]);
		double slope_can_2 = double(len[1] - len[0]) / double(comp_index[0]);
		double slope_can_3 = double(len[2] - len[1]) / double(comp_index[0]);
		 cout<<"angle:"<<angle<<endl;
		 cout<<"g:"<<g<<endl;
		 cout << "slope_can_1:" << slope_can_1 << endl;
		 cout << "slope_can_2:" << slope_can_2 << endl;
		 cout << "slope_can_3:" << slope_can_3 << endl;
		 if(g>=0)
		slope = abs(slope_can_1 - g) <= abs(slope_can_2 - g) ? slope_can_1
			: slope_can_2;
		 cout << "slope:" << slope << endl;
		return true;
	}
	else {
		slope = 0;
	}

	return false;
}


void Affine(const Mat& in, Mat& out, const double slope)
{

	Point2f dstTri[3];
	Point2f plTri[3];

	float height = (float)in.rows;
	float width = (float)in.cols;
	float xiff = (float)abs(slope) * height;

	if (slope > 0) {

		// right, new position is xiff/2

		plTri[0] = Point2f(0, 0);
		plTri[1] = Point2f(width - xiff - 1, 0);
		plTri[2] = Point2f(0 + xiff, height - 1);

		dstTri[0] = Point2f(xiff / 2, 0);
		dstTri[1] = Point2f(width - 1 - xiff / 2, 0);
		dstTri[2] = Point2f(xiff / 2, height - 1);
	}
	else {

		// left, new position is -xiff/2

		plTri[0] = Point2f(0 + xiff, 0);
		plTri[1] = Point2f(width - 1, 0);
		plTri[2] = Point2f(0, height - 1);

		dstTri[0] = Point2f(xiff / 2, 0);
		dstTri[1] = Point2f(width - 1 - xiff + xiff / 2, 0);
		dstTri[2] = Point2f(xiff / 2, height - 1);
	}

	Mat warp_mat = getAffineTransform(plTri, dstTri);

	Mat affine_mat;
	affine_mat.create((int)height, (int)width, TYPE);

	if (in.rows > HEIGHT || in.cols > WIDTH)

		warpAffine(in, affine_mat, warp_mat, affine_mat.size(),
			CV_INTER_AREA);
	else
		warpAffine(in, affine_mat, warp_mat, affine_mat.size(), CV_INTER_CUBIC);

	out = affine_mat;
	//imshow("Affine Pic", affine_mat);
	
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

	cout << "avg_s:" << s_all / count << endl;
	cout << "avg_v:" << v_all / count << endl;

	// 获取颜色匹配后的二值灰度图
	Mat src_grey;
	vector<Mat> hsvSplit_done;
	split(src_hsv, hsvSplit_done);
	src_grey = hsvSplit_done[2];

	match = src_grey;

	return src_grey;
}


void ProcessGreyPic(Mat& match_grey,Mat& mat_copy,double roi_angle)
{
	Mat src_threshold;
	threshold(match_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
	morphologyEx(src_threshold, src_threshold, MORPH_CLOSE, element);
	//imshow("小图闭操作结果", src_threshold);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_threshold,
		contours,               // a vector of contours
		hierarchy,
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE);  // all pixels of each contours

	RotatedRect rotated_rec;
	Rect rec_adapt;//矩形区域
	for (size_t i = 0; i < contours.size(); i++)
	{
		int true_pix_count = countNonZero(src_threshold(boundingRect(contours[i])));
		double true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(boundingRect(contours[i]).area());
		if (boundingRect(contours[i]).height > 10 && boundingRect(contours[i]).width > 80 && true_pix_rate > 0.5)
		{
			rec_adapt = boundingRect(contours[i]);
			rotated_rec = minAreaRect(contours[i]);
			//drawContours(mat_copy, contours, static_cast<int>(i), Scalar(0, 0, 255), 1);
			//drawContours(src_threshold, contours, static_cast<int>(i), Scalar(200, 200, 0), 2);
		}
	}

	Mat mat_plate, dstImage;
	mat_plate = mat_copy(rec_adapt);
	//imshow("车牌", mat_plate);

	double roi_angle_1 = rotated_rec.angle;
	cout << "Second ROI angle is : " << roi_angle_1 << endl;

	Mat mat_affine;
	
	Affine(mat_plate, mat_affine, AFFINE);
	//矫正后的车牌区域
	imwrite("charpic.jpg", mat_affine);
	//字符分割
	//vector<Mat> DividedChar;
	Pretreatment();
	//charDivision(mat_affine, DividedChar);
}


int ColorPlateLocate()
{
	cout << "ColorPlateLocate,Start..." << endl;

	Mat srcImage = imread("car1.jpg");
	//srcImage.resize(960);

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
		//imshow("Source Image", srcImage);
	}

	Mat match_grey;

	colorMatch(srcImage, match_grey, BLUE, false);
	//imshow("mat_gray", match_grey);

	Mat src_threshold;
	threshold(match_grey, src_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);

	Mat element = getStructuringElement(MORPH_RECT, Size(color_morphW, color_morphH));
	morphologyEx(src_threshold, src_threshold, MORPH_CLOSE, element);
	//imshow("闭操作结果", src_threshold);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_threshold,
		contours,               // a vector of contours
		hierarchy,
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE);  // all pixels of each contours
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
	}*/

	//imshow("轮廓图", mat_copy);

	RotatedRect rotated_rec;
	Rect rec_adapt;//矩形区域
	for (size_t i = 0; i < contours.size(); i++)
	{
		//----矩形区域非零像素占总的比例，防止有较大的空白区域干扰检测结果
		//----矩形的长宽限制，也可以再增加额外条件：长宽比例等
		//countNonZero 可以得到非零像素点的个数。
		//boundingRect 计算轮廓的最小外接矩形

		int true_pix_count = countNonZero(src_threshold(boundingRect(contours[i])));
		double true_pix_rate = static_cast<double>(true_pix_count) / static_cast<double>(boundingRect(contours[i]).area());
		if (boundingRect(contours[i]).height > 10 && boundingRect(contours[i]).width > 80 && true_pix_rate > 0.5)
		{
			rec_adapt = boundingRect(contours[i]);
			rotated_rec = minAreaRect(contours[i]);
			drawContours(mat_copy, contours, static_cast<int>(i), Scalar(0, 0, 255), 1);
			//drawContours(src_threshold, contours, static_cast<int>(i), Scalar(200, 200, 0), 2);
		}
	}

	Mat mat_plate, dstImage;
	mat_plate = srcImage(rec_adapt);
	//imshow("车牌大致区域", mat_plate);
	//imwrite("car1_plate.jpg", mat_plate);

	double angle_rec = rotated_rec.angle;
	cout << "First ROI angle is : " << angle_rec << endl;

	//旋转
	Point2f center(mat_plate.cols / 2, mat_plate.rows / 2);

	Mat rot_mat = getRotationMatrix2D(center, angle_rec, 1.0);

	warpAffine(mat_plate, dstImage, rot_mat, Size(mat_plate.cols, mat_plate.rows), CV_INTER_CUBIC);

	//imshow("dstImage_warp", dstImage);

	imwrite("car_rotation.jpg", dstImage);
	
	/*Rect_<float> safeBoundRect;
	fillBlank(rotated_rec, dstImage, safeBoundRect);
	Mat safeImage = dstImage(safeBoundRect);
	imshow("safeImage", safeImage);*/

	Mat small_mat_grey;

	colorMatch(dstImage, small_mat_grey, BLUE, false);

	//imshow("samll_mat_grey", small_mat_grey);

	ProcessGreyPic(small_mat_grey, dstImage, angle_rec);



	waitKey(0);
}


int main(int argc, char** argv)
{
	ColorPlateLocate();
	
	return 0;
}