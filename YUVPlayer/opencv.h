#include "opencv2\opencv.hpp"
using namespace cv;

#ifdef NDEBUG
#pragma comment( lib, "comctl32.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/zlib.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/ippicvmt.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_core310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_highgui310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_video310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_ml310.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_imgproc310.lib") 
#else
#pragma comment( lib, "comctl32.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/zlibd.lib")
#pragma comment( lib, "../3rdparty/opencv310/lib/ippicvmt.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_core310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_highgui310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_video310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_ml310d.lib") 
#pragma comment( lib, "../3rdparty/opencv310/lib/opencv_imgproc310d.lib") 
#endif