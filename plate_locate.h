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
static const double AFFINE = -0.15;
static const int DEFAULT_VERIFY_MIN = 1;   // 3
static const int DEFAULT_VERIFY_MAX = 24;  // 20
static const int DEFAULT_ANGLE = 60;  // 30
static const int DEFAULT_DEBUG = 1;

const float DEFAULT_ERROR = 0.9f;    // 0.6
const float DEFAULT_ASPECT = 3.75f;  // 3.75

//Rotation
static const int TYPE = CV_8UC3;
static const int WIDTH = 136;
static const int HEIGHT = 36;
//ÅÐ¶Ï×Ö·ûµãµÄÏñËØ³¤¶ÈãÐÖµ
static const int DOTLENGTH = 3;


static const char* kChars[] = {
  "0", "1", "2",
  "3", "4", "5",
  "6", "7", "8",
  "9",
  /*  10  */
  "A", "B", "C",
  "D", "E", "F",
  "G", "H", /* {"I", "I"} */
  "J", "K", "L",
  "M", "N", /* {"O", "O"} */
  "P", "Q", "R",
  "S", "T", "U",
  "V", "W", "X",
  "Y", "Z",
  /*  24  */
  "´¨" , "¶õ" , "¸Ó" ,
  "¸Ê" , "¹ó" , "¹ð" ,
  "ºÚ" , "»¦" , "¼½" ,
  "½ò" , "¾©" , "¼ª" ,
  "ÁÉ" , "Â³" , "ÃÉ" ,
  "Ãö" , "Äþ" , "Çà" ,
  "Çí" , "ÉÂ" , "ËÕ" ,
  "½ú" , "Íî" , "Ïæ" ,
  "ÐÂ" , "Ô¥" , "Óå" ,
  "ÔÁ" , "ÔÆ" , "²Ø" ,
  "Õã"
  /*  31  */
};

class CPlate {
public:
	CPlate() {
		m_score = -1;
		m_plateStr = "";
		m_plateColor = UNKNOWN;
	}

private:
	//! plate mat
	Mat m_plateMat;

	//! plate rect
	RotatedRect m_platePos;

	//! plate license
	String m_plateStr;

	//! plate locate type
	//LocateType m_locateType;

	//! plate color type
	Color m_plateColor;

	//! scale ratio
	float m_scale;

	//! plate likely
	double m_score;

	//! avg ostu level
	double m_ostuLevel;

	//! middle line
	Vec4f m_line;

	//! left point and right point;
	Point m_leftPoint;
	Point m_rightPoint;

	Rect m_mergeCharRect;
	Rect m_maxCharRect;

	int m_charCount;

	//! chinese mat;
	Mat m_chineseMat;

	//! chinese key;
	String m_chineseKey;

	//! distVec
	Vec2i m_distVec;
};

#endif // !PLATELOCATE_H_
#pragma once
