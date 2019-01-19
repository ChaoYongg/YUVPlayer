#include "opencv2\opencv.hpp"
using namespace cv;

#ifndef _WIN64

#ifdef NDEBUG
#pragma comment( lib, "comctl32.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/zlib.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_core310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_highgui310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_imgproc310.lib") 
#else
#pragma comment( lib, "comctl32.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/zlibd.lib")
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_core310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_highgui310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/x86/opencv_imgproc310d.lib") 
#endif

#endif