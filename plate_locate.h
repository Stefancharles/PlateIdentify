#ifndef PLATELOCATE_H_
#define PLATELOCATE_H_

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "iostream"

using namespace cv;
using namespace std;


enum Color { BLUE, YELLOW, WHITE, UNKNOWN };

static const int SOBEL_SCALE = 1;
static const int SOBEL_DELTA = 0;
static const int SOBEL_DDEPTH = CV_16S;
static const int SOBEL_X_WEIGHT = 1;

static const int SOBEL_morphW = 17;
static const int SOBEL_morphH = 3;

static const int color_morphW = 17;
static const int color_morphH = 3;

static const int DEFAULT_VERIFY_MIN = 1;   // 3
static const int DEFAULT_VERIFY_MAX = 24;  // 20
static const int DEFAULT_ANGLE = 60;  // 30
static const int DEFAULT_DEBUG = 1;

const float DEFAULT_ERROR = 0.9f;    // 0.6
const float DEFAULT_ASPECT = 3.75f;  // 3.75

class CPlateLocate
{
public:
	CPlateLocate();
	bool verifySizes(RotatedRect mr);


protected:
	float m_error;
	float m_aspect;
	int m_verifyMin;
	int m_verifyMax;
	int m_angle;
	bool m_debug;



};



#endif // !PLATELOCATE_H_
#pragma once
