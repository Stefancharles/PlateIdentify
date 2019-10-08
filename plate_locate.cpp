#include "plate_locate.h"


CPlateLocate::CPlateLocate() {
	//m_GaussianBlurSize = DEFAULT_GAUSSIANBLUR_SIZE;
	//m_MorphSizeWidth = DEFAULT_MORPH_SIZE_WIDTH;
	//m_MorphSizeHeight = DEFAULT_MORPH_SIZE_HEIGHT;

	m_error = DEFAULT_ERROR;
	m_aspect = DEFAULT_ASPECT;
	m_verifyMin = DEFAULT_VERIFY_MIN;
	m_verifyMax = DEFAULT_VERIFY_MAX;

	m_angle = DEFAULT_ANGLE;

	m_debug = DEFAULT_DEBUG;
}

bool CPlateLocate::verifySizes(RotatedRect mr) {
	float error = m_error;
	// Spain car plate size: 52x11 aspect 4,7272
	// China car plate size: 440mm*140mm£¬aspect 3.142857

	// Real car plate size: 136 * 32, aspect 4
	float aspect = m_aspect;

	// Set a min and max area. All other patchs are discarded
	// int min= 1*aspect*1; // minimum area
	// int max= 2000*aspect*2000; // maximum area
	int min = 34 * 8 * m_verifyMin;  // minimum area
	int max = 34 * 8 * m_verifyMax;  // maximum area

	// Get only patchs that match to a respect ratio.
	float rmin = aspect - aspect * error;
	float rmax = aspect + aspect * error;

	float area = mr.size.height * mr.size.width;
	float r = (float)mr.size.width / (float)mr.size.height;
	if (r < 1) r = (float)mr.size.height / (float)mr.size.width;

	// cout << "area:" << area << endl;
	// cout << "r:" << r << endl;

	if ((area < min || area > max) || (r < rmin || r > rmax))
		return false;
	else
		return true;
}