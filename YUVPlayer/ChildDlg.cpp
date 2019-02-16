#include "stdafx.h"
#include "ChildDlg.h"
#include "YUVPlayerDlg.h"
#include "multithread.h"
#include <stdlib.h>
#include "opencv.h"

CChildDlg::CChildDlg(UINT nID, CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{
	//++ 将需要标记的宏块的坐标初始化到图像区域外
    s32MBXIdx			= 0;		//++ 当前宏块所在列号（缩放前）
    s32MBYIdx			= 0;		//++ 当前宏块所在行号（缩放前）
    s32ViewMBx			= -100;		//++ 显示像素值的宏块的顶点坐标（缩放前）
    s32ViewMBy			= -100;		//++ 显示像素值的宏块的顶点坐标（缩放前）
    s32ViewBlkX			= -100;		//++ 显示像素值的宏块的顶点坐标（缩放后）
    s32ViewBlkY			= -100;		//++ 显示像素值的宏块的顶点坐标（缩放后）
	s32ViewBlkW			= 0;
	s32ViewBlkH			= 0;
    s32PrevBlkX			= -100;		//++ 前一个鼠标指向的宏块的顶点坐标（缩放后）
    s32PrevBlkY			= -100;		//++ 前一个鼠标指向的宏块的顶点坐标（缩放后）
	s32PrevBlkW			= 0;
	s32PrevBlkH			= 0;
    s32CurrMBx			= -100;		//++ 鼠标经过的宏块的顶点坐标（缩放前）
    s32CurrMBy			= -100;		//++ 鼠标经过的宏块的顶点坐标（缩放前）
    s32CurrBlkX			= -100;		//++ 鼠标经过的宏块的顶点坐标（缩放后）
    s32CurrBlkY			= -100;		//++ 鼠标经过的宏块的顶点坐标（缩放后）
	s32CurrBlkW			= 0;
	s32CurrBlkH			= 0;

#if CONGIF_VIEW_LCU
	s32MBXIdx_Lcu = 0;			//++ 当前宏块所在列号（缩放前）
	s32MBYIdx_Lcu = 0;			//++ 当前宏块所在行号（缩放前）
	s32ViewMBx_Lcu = -100;		//++ 显示像素值的宏块的顶点坐标（缩放前）
	s32ViewMBy_Lcu = -100;		//++ 显示像素值的宏块的顶点坐标（缩放前）
	s32ViewBlkX_Lcu = -100;		//++ 显示像素值的宏块的顶点坐标（缩放后）
	s32ViewBlkY_Lcu = -100;		//++ 显示像素值的宏块的顶点坐标（缩放后）
	s32ViewBlkW_Lcu = 0;
	s32ViewBlkH_Lcu = 0;
	s32PrevBlkX_Lcu = -100;		//++ 前一个鼠标指向的宏块的顶点坐标（缩放后）
	s32PrevBlkY_Lcu = -100;		//++ 前一个鼠标指向的宏块的顶点坐标（缩放后）
	s32PrevBlkW_Lcu = 0;
	s32PrevBlkH_Lcu = 0;
	s32CurrMBx_Lcu = -100;		//++ 鼠标经过的宏块的顶点坐标（缩放前）
	s32CurrMBy_Lcu = -100;		//++ 鼠标经过的宏块的顶点坐标（缩放前）
	s32CurrBlkX_Lcu = -100;		//++ 鼠标经过的宏块的顶点坐标（缩放后）
	s32CurrBlkY_Lcu = -100;		//++ 鼠标经过的宏块的顶点坐标（缩放后）
	s32CurrBlkW_Lcu = 0;
	s32CurrBlkH_Lcu = 0;
#endif
	bSizeChanged		= FALSE;
	pReadYUV[0]			= NULL;
	pReadYUV[1]			= NULL;
	pReadYUV[2]			= NULL;
	pOrigYUV[0]			= NULL;
	pOrigYUV[1]			= NULL;
	pOrigYUV[2]			= NULL;
	pMirrYUV[0]			= NULL;
	pMirrYUV[1]			= NULL;
	pMirrYUV[2]			= NULL;
	pRotaYUV[0]			= NULL;
	pRotaYUV[1]			= NULL;
	pRotaYUV[2]			= NULL;
	pDisplayLuma		= NULL;
	pDisplayChro		= NULL;
	pRGBBuff			= NULL;
	
    mouseMenu.LoadMenu(IDR_MOUSE_MENU);//装载自定义的右键菜单
    pSubMenu   = mouseMenu.GetSubMenu(0);//获取第一个弹出菜单，所以第一个菜单必须有子菜单
}

int32 CChildDlg::malloc_memory()
{
    uint32  u32LumaBuffSize;
    uint32  u32ChroBuffSize;
    uint32	u32MemorySize;
    
    //++ 为加快寻找同步帧的速度，原始图像内存空间按 4 字节整数倍开辟
    u32LumaBuffSize	  = ((u32LumaPicSize + 3) >> 2) << 2;
    u32ChroBuffSize	  = ((u32ChroPicSize + 3) >> 2) << 2;
    u32MemorySize	  = u32LumaBuffSize + (u32ChroBuffSize << 1);	//++ 读入的原始 YUV 图像
    u32MemorySize	 += u32LumaBuffSize + (u32ChroBuffSize << 1);	//++ 镜像缓冲
    u32MemorySize	 += u32LumaBuffSize + (u32ChroBuffSize << 1);	//++ 旋转缓冲
    u32MemorySize	 += u32LumaPicSize;	//++ 用于显示的 YUV 图像亮度分量
    u32MemorySize	 += u32ChroPicSize;	//++ 用于显示的 YUV 图像色度分量
    u32MemorySize	 += (((s32Width * 3 + 3) >> 2) << 2) * s32Height;	//++ RGB 内存空间
    
    pYUVBuff    = (LPBYTE)malloc(u32MemorySize);
    if (NULL == pYUVBuff)
    {
        AfxMessageBox("分配内存错误！\n", MB_ICONERROR);
        
        return FAILED_YUVPlayer;
    }
    pReadYUV[0]		= pYUVBuff;
    pReadYUV[1]		= pReadYUV[0] + u32LumaBuffSize;
    pReadYUV[2]		= pReadYUV[1] + u32ChroBuffSize;
    pMirrYUV[0]		= pReadYUV[2] + u32ChroBuffSize;
    pMirrYUV[1]		= pMirrYUV[0] + u32LumaBuffSize;
    pMirrYUV[2]		= pMirrYUV[1] + u32ChroBuffSize;
    pRotaYUV[0]		= pMirrYUV[2] + u32ChroBuffSize;
    pRotaYUV[1]		= pRotaYUV[0] + u32LumaBuffSize;
    pRotaYUV[2]		= pRotaYUV[1] + u32ChroBuffSize;
    pDisplayLuma	= pRotaYUV[2] + u32ChroBuffSize;
    pDisplayChro	= pDisplayLuma + u32LumaPicSize;
    pRGBBuff		= pDisplayChro + u32ChroPicSize;
    memset(pYUVBuff, 128, u32MemorySize);
    
    hloc = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256));
    if (NULL == hloc)
    {
        free(pYUVBuff);
        AfxMessageBox("分配内存错误！\n", MB_ICONERROR);
        
        return FAILED_YUVPlayer;
    }
    
    return SUCCEEDED_YUVPlayer;
}

void CChildDlg::set_bmp_parameter()
{
    int32	i;
    HANDLE	hloc1;
    RGBQUAD	*argbq;
    
    
    BmpInfo	 = (LPBITMAPINFO) GlobalLock(hloc);
    hloc1	 = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE,(sizeof(RGBQUAD) * 256));
    argbq	 = (RGBQUAD *) LocalLock(hloc1);
    
    for (i = 0; i < 256; i ++)
    {
        argbq[i].rgbBlue		 = i;
        argbq[i].rgbGreen		 = i;
        argbq[i].rgbRed			 = i;
        argbq[i].rgbReserved	 = 0;
    }
    
    BmpInfo->bmiHeader.biSize			 = sizeof(BITMAPINFOHEADER);
    BmpInfo->bmiHeader.biPlanes			 = 1;
    BmpInfo->bmiHeader.biBitCount		 = 24;
    BmpInfo->bmiHeader.biCompression	 = BI_RGB;
    BmpInfo->bmiHeader.biWidth			 = s32Width;
    BmpInfo->bmiHeader.biHeight			 = s32Height;
    
    memcpy(BmpInfo->bmiColors, argbq, sizeof(RGBQUAD) * 256);
    
    LocalUnlock(hloc1);
    LocalFree(hloc1);
}

void CChildDlg::resize_window()
{
    //++ 将窗口尺寸映射为屏幕尺寸
    CRect	crRect(0, 0, s32ZoomWidth, s32ZoomHeight);
    CalcWindowRect(&crRect, CWnd::adjustOutside);
	SetWindowPos(NULL, 0, 0, crRect.Width(), crRect.Height(), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_HIDEWINDOW);
}

int32 CChildDlg::show_image(CDC *pDC)
{
    int32   s32Ret;
    
    BmpInfo->bmiHeader.biBitCount    = 24;
    pDC->SetStretchBltMode(COLORONCOLOR);
    s32Ret	 = StretchDIBits(pDC->m_hDC, 0, 0, s32ZoomWidth, s32ZoomHeight, 0, 0, s32Width, s32Height, pRGBBuff, BmpInfo, DIB_RGB_COLORS, SRCCOPY);
    
    if (s32Ret == GDI_ERROR)
    {
        return FAILED_YUVPlayer;
    }
    
    return SUCCEEDED_YUVPlayer;
}


void CChildDlg::copy_cb_to_y_plane(uint8 *pDisplayLuma, uint8* pCbPlane)
{
	int i, j;
	int32 s32ChroWidth;
	int32 s32ChroHeigth;

	switch (u8SampleFormat)
	{
	case YUV400:
	case YUV420:
		s32ChroWidth = s32Width >> u8Sample_x;
		s32ChroHeigth = s32Height >> u8Sample_y;
		break;
	case YUV422:
		s32ChroWidth = s32Width >> u8Sample_x;
		s32ChroHeigth = s32Height;
		break;
	case YUV444:
	case RGB24:
	case GBR24:
		s32ChroWidth = s32Width;
		s32ChroHeigth = s32Height;
		break;
	default:
		break;
	}

	switch (u8SampleFormat)
	{
	case YUV400:
	case YUV420:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			for (j = 0; j < s32Height; j++)
			{
				for (i = 0; i < (s32Width >> u8Sample_x); i++)
				{
					pDisplayLuma[j * (s32Width * 2) + (i * 4)] = pCbPlane[(j >> 1) * (s32ChroWidth * 2) + (i << 1)];
					pDisplayLuma[j * (s32Width * 2) + (i * 4) + 1] = pCbPlane[(j >> 1) * (s32ChroWidth * 2) + (i << 1) + 1];
					pDisplayLuma[j * (s32Width * 2) + (i * 4) + 2] = pCbPlane[(j >> 1) * (s32ChroWidth * 2) + (i << 1)];
					pDisplayLuma[j * (s32Width * 2) + (i * 4) + 3] = pCbPlane[(j >> 1) * (s32ChroWidth * 2) + (i << 1) + 1];
				}
			}
		}
		else
		{
			for (j = 0; j < s32Height; j++)
			{
				for (i = 0; i < s32Width; i++)
				{
					pDisplayLuma[j * s32Width + i] = pCbPlane[(j >> 1) * s32ChroWidth + (i >> 1)];
				}
			}
		}
		break;
	case RGB24:
	case GBR24:
	case YUV444:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			for (j = 0; j < s32Height; j++)
			{
				for (i = 0; i < s32Width; i++)
				{
					pDisplayLuma[j * (s32Width << 1) + (i << 1)] = pCbPlane[j * (s32ChroWidth << 1) + (i << 1)];
					pDisplayLuma[j * (s32Width << 1) + (i << 1) + 1] = pCbPlane[j * (s32ChroWidth << 1) + (i << 1) + 1];
				}
			}
		}
		else
		{
			for (j = 0; j < s32Height; j++)
			{
				for (i = 0; i < s32Width; i++)
				{
					pDisplayLuma[j * s32Width + i] = pCbPlane[j * s32ChroWidth + i];
				}
			}
		}
		break;
	default:
		break;
	}
}

void CChildDlg::color_space_convert(uint8 u8ImageMode)
{
    int32 i, j;

	if (u8SampleFormat == NV12 || u8SampleFormat == NV21)//其他格式后续可考虑都采用Opencv
	{
		Mat dstRgb(s32Height, s32Width, CV_8UC3);
		Mat srcYuv(s32Height * 3 / 2, s32Width, CV_8UC1);

		switch (u8ImageMode)
		{
		case IMAGE_Y:
		case IMAGE_U:
		case IMAGE_V:
		case IMAGE_YUV:
			srcYuv.data = pOrigYUV[0];
			cvtColor(srcYuv, dstRgb, u8SampleFormat == NV12 ? CV_YUV2BGR_NV12 : CV_YUV2BGR_NV21);
			for (int pih = 0; pih < s32Height; pih++)
				memcpy(pRGBBuff+pih*s32Width*3, dstRgb.data + (s32Height-pih -1) * s32Width * 3, s32Width * 3);
			break;
		}
		return;
	}
	else
	{
		switch (u8ImageMode)
		{
		case IMAGE_YUV:
			YV12_to_RGB24(pOrigYUV[0], pOrigYUV[1], pOrigYUV[2], u8ImageMode);
			break;

		case IMAGE_Y:
			YV12_to_RGB24(pOrigYUV[0], pDisplayChro, pDisplayChro, u8ImageMode);
			break;

		case IMAGE_U:
			copy_cb_to_y_plane(pDisplayLuma, pOrigYUV[1]);
			YV12_to_RGB24(pDisplayLuma, pDisplayChro, pDisplayChro, u8ImageMode);
			break;

		case IMAGE_V:
			copy_cb_to_y_plane(pDisplayLuma, pOrigYUV[2]);
			YV12_to_RGB24(pDisplayLuma, pDisplayChro, pDisplayChro, u8ImageMode);
			break;
		default:
			break;
		}
	}
}

void CChildDlg::calculate_pixels_pos(int32 x, int32 yStride, int32 cbStride, int *yPos, int *cbPos, uint8 u8ImageMode)
{
	//////计算位置
	switch (u8SampleFormat)
	{
	case YUV400:
		*yPos = yStride + x;
		if (u8BitDepth != BIT_DEPTH_8)
		{
			*yPos = (yStride << 1) + (x << 1);
		}
		break;
	case YUV420:
		*yPos = yStride + x;
		*cbPos = cbStride + (x >> 1);
		if (u8BitDepth != BIT_DEPTH_8)
		{
			*yPos = (yStride << 1) + (x << 1);
			*cbPos = (cbStride << 1) + ((x >> 1) << 1);
		}
		break;
	case YUV422:
		*yPos = yStride + x;
		*cbPos = cbStride + (x >> 1);
		if (u8BitDepth != BIT_DEPTH_8)
		{
			*yPos = (yStride << 1) + (x << 1);
			*cbPos = (cbStride << 1) + ((x >> 1) << 1);
		}
		break;
	case YUV444:
	case RGB24:
	case GBR24:
		*yPos = yStride + x;
		*cbPos = *yPos;
		if (u8BitDepth != BIT_DEPTH_8)
		{
			*yPos = (yStride << 1) + (x << 1);
			*cbPos = *yPos;
		}
		break;
	default:
		break;
	}
}

void CChildDlg::convert_yuv_to_rgb(int32 x, int32 k, int32 yPos, int32 cbPos, uint8* pu8Y, uint8* pu8U, uint8* pu8V, uint8 u8ImageMode)
{
	//////像素转换
	int32 yTemp, uTemp, vTemp;//转换rgb\bgr格式所需变量
	int32 rTemp, gTemp, bTemp; //r,g,b临时变量
	int32 rgb[3];
	int32 rgbPos;
	int32 j;
	switch (u8SampleFormat)
	{
	case YUV400:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			rTemp = (pu8Y[yPos + 1] << 8) | pu8Y[yPos];

			rTemp = rTemp - (16 << (u8BitDepth - 8));

			rgb[2] = (rTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;
			rgb[1] = (rTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;
			rgb[0] = (rTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;
		}
		else
		{
			rgb[2] = (298 * pu8Y[yPos] + 128) >> 8;
			rgb[1] = (298 * pu8Y[yPos] + 128) >> 8;
			rgb[0] = (298 * pu8Y[yPos] + 128) >> 8;
		}
		break;
	case YUV420:
	case YUV422:
	case YUV444:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			rTemp = (pu8Y[yPos + 1] << 8) | pu8Y[yPos];
			gTemp = (pu8U[cbPos + 1] << 8) | pu8U[cbPos];
			bTemp = (pu8V[cbPos + 1] << 8) | pu8V[cbPos];

			rTemp = rTemp - (16 << (u8BitDepth - 8));
			gTemp = gTemp - (128 << (u8BitDepth - 8));
			bTemp = bTemp - (128 << (u8BitDepth - 8));

			if (u8ImageMode != IMAGE_YUV)
			{
				gTemp = 0;
				bTemp = 0;
			}

			rgb[2] = (298 * rTemp + 409 * bTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//r
			rgb[1] = (298 * rTemp - 100 * gTemp - 208 * bTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//g
			rgb[0] = (298 * rTemp + 516 * gTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//b		
		}
		else
		{
			rgb[2] = (298 * (pu8Y[yPos] - 16) + 409 * (pu8V[cbPos] - 128) + 128) >> 8;//r
			rgb[1] = (298 * (pu8Y[yPos] - 16) - 100 * (pu8U[cbPos] - 128) - 208 * (pu8V[cbPos] - 128) + 128) >> 8;//g
			rgb[0] = (298 * (pu8Y[yPos] - 16) + 516 * (pu8U[cbPos] - 128) + 128) >> 8;//b
		}
		break;
	case RGB24:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			rTemp = (pu8Y[yPos + 1] << 8) | pu8Y[yPos];
			gTemp = (pu8U[cbPos + 1] << 8) | pu8U[cbPos];
			bTemp = (pu8V[cbPos + 1] << 8) | pu8V[cbPos];

			yTemp = 0.212600*rTemp + 0.715200*gTemp + 0.072200*bTemp;
			uTemp = -0.1146*rTemp - 0.3854*gTemp + 0.5000*bTemp + (128 << (u8BitDepth - 8));
			vTemp = 0.5000*rTemp - 0.4542*gTemp - 0.0468*bTemp + (128 << (u8BitDepth - 8));

			yTemp = yTemp - (16 << (u8BitDepth - 8));
			uTemp = uTemp - (128 << (u8BitDepth - 8));
			vTemp = vTemp - (128 << (u8BitDepth - 8));

			rgb[2] = (298 * yTemp + 409 * vTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//r
			rgb[1] = (298 * yTemp - 100 * uTemp - 208 * vTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//g
			rgb[0] = (298 * yTemp + 516 * uTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//b
		}
		else
		{
			yTemp = 0.212600*pu8Y[yPos] + 0.715200*pu8U[cbPos] + 0.072200*pu8V[cbPos];
			uTemp = -0.1146*pu8Y[yPos] - 0.3854*pu8U[cbPos] + 0.5000*pu8V[cbPos] + 128;
			vTemp = 0.5000*pu8Y[yPos] - 0.4542*pu8U[cbPos] - 0.0468*pu8V[cbPos] + 128;

			rgb[2] = int32(1.164383 * (yTemp - 16) + 1.596027 * (vTemp - 128)); // r
			rgb[1] = int32(1.164383 * (yTemp - 16) - 0.812968 * (vTemp - 128) - 0.391762 * (uTemp - 128)); // g
			rgb[0] = int32(1.164383 * (yTemp - 16) + 2.017232 * (uTemp - 128)); // b	
		}
		break;
	case GBR24:
		if (u8BitDepth != BIT_DEPTH_8)
		{
			rTemp = (pu8Y[yPos + 1] << 8) | pu8Y[yPos];
			gTemp = (pu8U[cbPos + 1] << 8) | pu8U[cbPos];
			bTemp = (pu8V[cbPos + 1] << 8) | pu8V[cbPos];

			yTemp = 0.212600*bTemp + 0.715200*rTemp + 0.072200*gTemp;
			uTemp = -0.1146*bTemp - 0.3854*rTemp + 0.5000*gTemp + (128 << (u8BitDepth - 8));
			vTemp = 0.5000*bTemp - 0.4542*rTemp - 0.0468*gTemp + (128 << (u8BitDepth - 8));

			yTemp = yTemp - (16 << (u8BitDepth - 8));
			uTemp = uTemp - (128 << (u8BitDepth - 8));
			vTemp = vTemp - (128 << (u8BitDepth - 8));

			rgb[2] = (298 * yTemp + 409 * vTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//r
			rgb[1] = (298 * yTemp - 100 * uTemp - 208 * vTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//g
			rgb[0] = (298 * yTemp + 516 * uTemp + (128 << (u8BitDepth - 8))) >> u8BitDepth;//b
		}
		else
		{
			yTemp = 0.212600*pu8V[yPos] + 0.715200*pu8Y[cbPos] + 0.072200*pu8U[cbPos];
			uTemp = -0.1146*pu8V[yPos] - 0.3854*pu8Y[cbPos] + 0.5000*pu8U[cbPos] + 128;
			vTemp = 0.5000*pu8V[yPos] - 0.4542*pu8Y[cbPos] - 0.0468*pu8U[cbPos] + 128;

			rgb[2] = (298 * (yTemp - 16) + 409 * (vTemp - 128) + 128) >> 8;//r
			rgb[1] = (298 * (yTemp - 16) - 100 * (uTemp - 128) - 208 * (vTemp - 128) + 128) >> 8;//g
			rgb[0] = (298 * (yTemp - 16) + 516 * (uTemp - 128) + 128) >> 8;//b
		}
		break;
	default:
		break;
	}
	/////将转换好的rgb像素回填到RGB buffer中
	rgbPos = k + x * 3;
	for (j = 0; j < 3; j++)
	{
		if ((rgb[j] >= 0) && (rgb[j] <= 255))
		{
			pRGBBuff[rgbPos + j] = rgb[j];
		}
		else
		{
			pRGBBuff[rgbPos + j] = (rgb[j] < 0) ? 0 : 255;
		}
	}
}

void CChildDlg::YV12_to_RGB24(uint8* pu8Y, uint8* pu8U, uint8* pu8V, uint8 u8ImageMode)
{
	int32	x;
	int32	y;
	int32	k;      //bmp（rgb格式）空间的位置索引，bmp图像从下往上扫描
	int32	yStride	= 0;    //y横向跨度
	int32	cbStride = 0;   //uv横向跨度
	int32	s32RGBBuffStride	 = ((s32Width * 3 + 3) >> 2) << 2;
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;

	k	= s32Height * s32RGBBuffStride;
	for(y = 0; y < s32Height; y ++)
	{
		k	-= s32RGBBuffStride;
		for(x = 0; x < s32Width; x ++)
		{
			int32	yPos;//y分量索引
			int32	cbPos;//uv分量索引

			calculate_pixels_pos(x, yStride, cbStride, &yPos, &cbPos, u8ImageMode);
			convert_yuv_to_rgb(x, k, yPos, cbPos, pu8Y, pu8U, pu8V, u8ImageMode);
		}
		
		/////位置偏移到下一行
		yStride += s32Width;
		switch (u8SampleFormat)
		{
			case YUV400:
			case YUV420:
				cbStride += (y % 2) ? (s32Width >> u8Sample_x) : 0;
				break;
			case YUV422:
				cbStride += (s32Width >> u8Sample_x);
				break;
			case RGB24:
			case GBR24:
			case YUV444:
				cbStride += s32Width;
				break;
			default:
				break;
		}	
	}
}

void CChildDlg::show_macroblock_info()
{
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
#if CONGIF_VIEW_LCU
#endif

	if ((pMainDlg->bShowMBInfo == TRUE) && (pMainDlg->bEnMBInfo == TRUE))
	{
		//++ 获得像素值
		get_pixel_value();
		//++ 擦掉历史选定像素点标记
		MBInfoDlg.clean_mark();
		//++ 输出宏块像素信息
		MBInfoDlg.draw_pixel_table();
		//++ 输出宏块位置信息
		MBInfoDlg.m_sMBInfo.Format("%d\r\n%d\r\n( x , y ) = ( %d , %d )\r\n( x , y ) = ( %d , %d )\r\n( x , y ) = ( %d , %d )",
			s32MBYIdx_Lcu * (s32Width / 64 + (s32Width % 64 ? 1 : 0)) + s32MBXIdx_Lcu,
			s32MBYIdx * (s32Width >> 4) + s32MBXIdx, s32MBXIdx, s32MBYIdx,
			s32CurrMBx, s32CurrMBy, (s32CurrMBx >> 1), (s32CurrMBy >> 1));
		
		MBInfoDlg.UpdateData(FALSE);
		MBInfoDlg.ShowWindow(SW_SHOW);
		MBInfoDlg.SetWindowText("宏块信息 - " + fileName);
	}
}

void CChildDlg::view_macroblock()
{
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
    
    
    if ((s32CurrBlkX != s32ViewBlkX) || (s32CurrBlkY != s32ViewBlkY))
    {
        //++ 擦掉历史选定宏块的颜色标记
        CClientDC	currDC(this);
        CRect	currRect(s32ViewBlkX, s32ViewBlkY, s32ViewBlkX + s32ViewBlkW, s32ViewBlkY + s32ViewBlkH);
        InvalidateRect(currRect, TRUE);
        
        //++ 选定宏块采用蓝色标记
        CPen	bluePen;
        bluePen.CreatePen(PS_DOT, 1, RGB(0, 200, 255)); //++ 蓝色虚线笔
        currDC.SelectObject(&bluePen);
        currDC.MoveTo(s32CurrBlkX, s32CurrBlkY);
        currDC.LineTo(s32CurrBlkX + s32CurrBlkW - 1, s32CurrBlkY);
        currDC.LineTo(s32CurrBlkX + s32CurrBlkW - 1, s32CurrBlkY + s32CurrBlkH - 1);
        currDC.LineTo(s32CurrBlkX, s32CurrBlkY + s32CurrBlkH - 1);
        currDC.LineTo(s32CurrBlkX, s32CurrBlkY);
        bluePen.DeleteObject();
        
        s32ViewBlkX		= s32CurrBlkX;
        s32ViewBlkY		= s32CurrBlkY;
		s32ViewBlkW		= s32CurrBlkW;
		s32ViewBlkH		= s32CurrBlkH;
    }
#if CONGIF_VIEW_LCU
	if ((s32CurrBlkX_Lcu != s32ViewBlkX_Lcu) || (s32CurrBlkY_Lcu != s32ViewBlkY_Lcu))
	{
		//++ 擦掉历史选定宏块的颜色标记
		CClientDC	currDC_Lcu(this);
		CRect	currRect_Lcu(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu, s32ViewBlkX_Lcu + s32ViewBlkW_Lcu, s32ViewBlkY_Lcu + s32ViewBlkH_Lcu);
		InvalidateRect(currRect_Lcu, TRUE);

		//++ 选定宏块采用黄色标记
		CPen	bluePen_Lcu;
		bluePen_Lcu.CreatePen(PS_DOT, 1, RGB(255, 0, 0)); //++ 黄色虚线笔
		currDC_Lcu.SelectObject(&bluePen_Lcu);
		currDC_Lcu.MoveTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu);
		currDC_Lcu.LineTo(s32CurrBlkX_Lcu + s32CurrBlkW_Lcu - 1, s32CurrBlkY_Lcu);
		currDC_Lcu.LineTo(s32CurrBlkX_Lcu + s32CurrBlkW_Lcu - 1, s32CurrBlkY_Lcu + s32CurrBlkH_Lcu - 1);
		currDC_Lcu.LineTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu + s32CurrBlkH_Lcu - 1);
		currDC_Lcu.LineTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu);
		bluePen_Lcu.DeleteObject();

		s32ViewBlkX_Lcu = s32CurrBlkX_Lcu;
		s32ViewBlkY_Lcu = s32CurrBlkY_Lcu;
		s32ViewBlkW_Lcu = s32CurrBlkW_Lcu;
		s32ViewBlkH_Lcu = s32CurrBlkH_Lcu;
	}
#endif
	show_macroblock_info();
}

void CChildDlg::mark_macroblock()
{
    CRect	currRect;
    CClientDC	currDC(this);
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
    
	s32CurrMBx		= s32MBXIdx << 4;
	s32CurrMBy		= s32MBYIdx << 4;
	s32CurrBlkX		= s32CurrMBx * s32ZoomWidth / s32Width;
	s32CurrBlkY		= s32CurrMBy * s32ZoomHeight / s32Height;

    if ((s32CurrBlkX != s32PrevBlkX) || (s32CurrBlkY != s32PrevBlkY))
    {
        //++ 擦掉历史宏块的颜色标记
        if ((s32ViewBlkX != s32PrevBlkX) || (s32ViewBlkY != s32PrevBlkY))
        {
            currRect.top	 = s32PrevBlkY;
            currRect.left	 = s32PrevBlkX;
            currRect.right	 = s32PrevBlkX + s32PrevBlkW;
            currRect.bottom	 = s32PrevBlkY + s32PrevBlkH;
            InvalidateRect(currRect, TRUE);
        }
        
        //++ 鼠标经过选定宏块之后，将选定宏块恢复为蓝色标记
        if ((pMainDlg->bShowMBInfo == TRUE) && (s32PrevBlkX == s32ViewBlkX) && (s32PrevBlkY == s32ViewBlkY))
        {
            CPen	bluePen;
            bluePen.CreatePen(PS_DOT, 1, RGB(0, 200, 255)); //++ 蓝色虚线笔
            currDC.SelectObject(&bluePen);
            currDC.MoveTo(s32ViewBlkX, s32ViewBlkY);
            currDC.LineTo(s32ViewBlkX + s32ViewBlkW - 1, s32ViewBlkY);
            currDC.LineTo(s32ViewBlkX + s32ViewBlkW - 1, s32ViewBlkY + s32ViewBlkH - 1);
            currDC.LineTo(s32ViewBlkX, s32ViewBlkY + s32ViewBlkH - 1);
            currDC.LineTo(s32ViewBlkX, s32ViewBlkY);
            bluePen.DeleteObject();
        }
        
        //++ 当前宏块采用绿色标记
        CPen	greenPen;
        greenPen.CreatePen(PS_DOT, 1, RGB(0, 255, 0)); //++ 绿色虚线笔
        currDC.SelectObject(&greenPen);

		s32CurrBlkW		= (s32CurrMBx + 16) * s32ZoomWidth / s32Width - s32CurrBlkX;
		s32CurrBlkH		= (s32CurrMBy + 16) * s32ZoomHeight / s32Height - s32CurrBlkY;
        currDC.MoveTo(s32CurrBlkX, s32CurrBlkY);
        currDC.LineTo(s32CurrBlkX + s32CurrBlkW - 1, s32CurrBlkY);
        currDC.LineTo(s32CurrBlkX + s32CurrBlkW - 1, s32CurrBlkY + s32CurrBlkH - 1);
        currDC.LineTo(s32CurrBlkX, s32CurrBlkY + s32CurrBlkH - 1);
        currDC.LineTo(s32CurrBlkX, s32CurrBlkY);
        greenPen.DeleteObject();
        
        s32PrevBlkX		= s32CurrBlkX;
        s32PrevBlkY		= s32CurrBlkY;
		s32PrevBlkW		= s32CurrBlkW;
		s32PrevBlkH		= s32CurrBlkH;
    }

#if CONGIF_VIEW_LCU
	CRect	currRect_Lcu;
	CClientDC	currDC_Lcu(this);
	CYUVPlayerDlg	*pMainDlg_Lcu = (CYUVPlayerDlg *)this->pMainDlg;

	s32CurrMBx_Lcu = s32MBXIdx_Lcu << 6;//64*64
	s32CurrMBy_Lcu = s32MBYIdx_Lcu << 6;
	s32CurrBlkX_Lcu = s32CurrMBx_Lcu * s32ZoomWidth / s32Width;
	s32CurrBlkY_Lcu = s32CurrMBy_Lcu * s32ZoomHeight / s32Height;

	if ((s32CurrBlkX_Lcu != s32PrevBlkX_Lcu) || (s32CurrBlkY_Lcu != s32PrevBlkY_Lcu))
	{
		//++ 擦掉历史宏块的颜色标记
		if ((s32ViewBlkX_Lcu != s32PrevBlkX_Lcu) || (s32ViewBlkY_Lcu != s32PrevBlkY_Lcu))
		{
			currRect_Lcu.top = s32PrevBlkY_Lcu;
			currRect_Lcu.left = s32PrevBlkX_Lcu;
			currRect_Lcu.right = s32PrevBlkX_Lcu + s32PrevBlkW_Lcu;
			currRect_Lcu.bottom = s32PrevBlkY_Lcu + s32PrevBlkH_Lcu;
			InvalidateRect(currRect_Lcu, TRUE);
		}

		//++ 鼠标经过选定宏块之后，将选定宏块恢复为黄色标记
		if ((pMainDlg_Lcu->bShowMBInfo == TRUE) && (s32PrevBlkX_Lcu == s32ViewBlkX_Lcu) && (s32PrevBlkY_Lcu == s32ViewBlkY_Lcu))
		{
			CPen	bluePen_Lcu;
			bluePen_Lcu.CreatePen(PS_DOT, 1, RGB(255, 0, 0)); //++ 黄色虚线笔 RGB(251, 246, 54)
			currDC_Lcu.SelectObject(&bluePen_Lcu);
			currDC_Lcu.MoveTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu);
			currDC_Lcu.LineTo(s32ViewBlkX_Lcu + s32ViewBlkW_Lcu - 1, s32ViewBlkY_Lcu);
			currDC_Lcu.LineTo(s32ViewBlkX_Lcu + s32ViewBlkW_Lcu - 1, s32ViewBlkY_Lcu + s32ViewBlkH_Lcu - 1);
			currDC_Lcu.LineTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu + s32ViewBlkH_Lcu - 1);
			currDC_Lcu.LineTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu);
			bluePen_Lcu.DeleteObject();
		}
	}
#if CONGIF_VIEW_LCU
	//++ 当前宏块采用紫色标记
	CPen	greenPen_Lcu;
	greenPen_Lcu.CreatePen(PS_DOT, 1, RGB(102, 0, 204)); //++ 紫色虚线笔
	currDC_Lcu.SelectObject(&greenPen_Lcu);

	s32CurrBlkW_Lcu = (s32CurrMBx_Lcu + 64) * s32ZoomWidth / s32Width - s32CurrBlkX_Lcu;
	s32CurrBlkH_Lcu = (s32CurrMBy_Lcu + 64) * s32ZoomHeight / s32Height - s32CurrBlkY_Lcu;
	currDC_Lcu.MoveTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu);
	currDC_Lcu.LineTo(s32CurrBlkX_Lcu + s32CurrBlkW_Lcu - 1, s32CurrBlkY_Lcu);
	currDC_Lcu.LineTo(s32CurrBlkX_Lcu + s32CurrBlkW_Lcu - 1, s32CurrBlkY_Lcu + s32CurrBlkH_Lcu - 1);
	currDC_Lcu.LineTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu + s32CurrBlkH_Lcu - 1);
	currDC_Lcu.LineTo(s32CurrBlkX_Lcu, s32CurrBlkY_Lcu);
	greenPen_Lcu.DeleteObject();

	s32PrevBlkX_Lcu = s32CurrBlkX_Lcu;
	s32PrevBlkY_Lcu = s32CurrBlkY_Lcu;
	s32PrevBlkW_Lcu = s32CurrBlkW_Lcu;
	s32PrevBlkH_Lcu = s32CurrBlkH_Lcu;
#endif

#endif
}

void CChildDlg::remark_macroblock(CPaintDC *pDC) 
{
    //++ 当前宏块采用蓝色标记
    CPen	bluePen;
    bluePen.CreatePen(PS_DOT, 1, RGB(0, 200, 255)); //++ 蓝色笔
    pDC->SelectObject(&bluePen);
    pDC->MoveTo(s32ViewBlkX, s32ViewBlkY);
    pDC->LineTo(s32ViewBlkX + s32ViewBlkW - 1, s32ViewBlkY);
    pDC->LineTo(s32ViewBlkX + s32ViewBlkW - 1, s32ViewBlkY + s32ViewBlkH - 1);
    pDC->LineTo(s32ViewBlkX, s32ViewBlkY + s32ViewBlkH - 1);
    pDC->LineTo(s32ViewBlkX, s32ViewBlkY);
    bluePen.DeleteObject();
#if CONGIF_VIEW_LCU
	//++ 当前宏块采用黄色标记
	CPen	bluePen_Lcu;
	bluePen_Lcu.CreatePen(PS_DOT, 1, RGB(255, 0, 0)); //++ 黄色笔
	pDC->SelectObject(&bluePen_Lcu);
	pDC->MoveTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu);
	pDC->LineTo(s32ViewBlkX_Lcu + s32ViewBlkW_Lcu - 1, s32ViewBlkY_Lcu);
	pDC->LineTo(s32ViewBlkX_Lcu + s32ViewBlkW_Lcu - 1, s32ViewBlkY_Lcu + s32ViewBlkH_Lcu - 1);
	pDC->LineTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu + s32ViewBlkH_Lcu - 1);
	pDC->LineTo(s32ViewBlkX_Lcu, s32ViewBlkY_Lcu);
	bluePen_Lcu.DeleteObject();
#endif
}

void CChildDlg::show_mouse_menu()
{
    CString	csMBInfo;
    CPoint	cursorPos;//定义一个用于确定光标位置的位置

    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;

    
    GetCursorPos(&cursorPos);//获取当前光标的位置，以便使得菜单可以跟随光标
#if CONGIF_VIEW_LCU
	csMBInfo.Format("LCU地址：%d", s32MBYIdx_Lcu * (s32Width >> 6) + s32MBXIdx_Lcu);
	pSubMenu->ModifyMenu(ID_MENUITEM_MBINFO0, MF_BYCOMMAND, ID_MENUITEM_MBINFO0, csMBInfo);
#endif
    csMBInfo.Format("宏块地址：%d", s32MBYIdx * (s32Width >> 4) + s32MBXIdx);
    pSubMenu->ModifyMenu(ID_MENUITEM_MBINFO1, MF_BYCOMMAND, ID_MENUITEM_MBINFO1, csMBInfo);
    csMBInfo.Format("宏块坐标：(%d, %d)", s32MBXIdx, s32MBYIdx);
    pSubMenu->ModifyMenu(ID_MENUITEM_MBINFO2, MF_BYCOMMAND, ID_MENUITEM_MBINFO2, csMBInfo);
    csMBInfo.Format("亮度顶点：(%d, %d)", s32CurrMBx, s32CurrMBy);
    pSubMenu->ModifyMenu(ID_MENUITEM_MBINFO3, MF_BYCOMMAND, ID_MENUITEM_MBINFO3, csMBInfo);
    csMBInfo.Format("色度顶点：(%d, %d)", (s32CurrMBx >> 1), (s32CurrMBy >> 1));
    pSubMenu->ModifyMenu(ID_MENUITEM_MBINFO4, MF_BYCOMMAND, ID_MENUITEM_MBINFO4, csMBInfo);

    switch (pMainDlg->u8ImageMode)
    {
    case IMAGE_YUV:
        pSubMenu->CheckMenuItem(ID_MENUITEM_YUV, MF_BYCOMMAND | MF_CHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_Y, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_U, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_V, MF_BYCOMMAND | MF_UNCHECKED);
        
        break;
        
    case IMAGE_Y:
        pSubMenu->CheckMenuItem(ID_MENUITEM_YUV, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_Y, MF_BYCOMMAND | MF_CHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_U, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_V, MF_BYCOMMAND | MF_UNCHECKED);

        break;
        
    case IMAGE_U:
        pSubMenu->CheckMenuItem(ID_MENUITEM_YUV, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_Y, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_U, MF_BYCOMMAND | MF_CHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_V, MF_BYCOMMAND | MF_UNCHECKED);
        
        break;
        
    case IMAGE_V:
        pSubMenu->CheckMenuItem(ID_MENUITEM_YUV, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_Y, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_U, MF_BYCOMMAND | MF_UNCHECKED);
        pSubMenu->CheckMenuItem(ID_MENUITEM_V, MF_BYCOMMAND | MF_CHECKED);
        
        break;
        
    default:
        
        break;
    }

    switch (pMainDlg->bEnMBInfo)
    {
    case TRUE:
        pSubMenu->CheckMenuItem(ID_MENUITEM_SHOWMBINFO, MF_BYCOMMAND | MF_CHECKED);
        
        break;
        
    case FALSE:
        pSubMenu->CheckMenuItem(ID_MENUITEM_SHOWMBINFO, MF_BYCOMMAND | MF_UNCHECKED);
		
        break;
        
    default:
        
        break;
    }
	
    switch (pMainDlg->bAttachFlag)
    {
    case TRUE:
        pSubMenu->CheckMenuItem(ID_MENUITEM_ATTACH, MF_BYCOMMAND | MF_CHECKED);
        
        break;
        
    case FALSE:
        pSubMenu->CheckMenuItem(ID_MENUITEM_ATTACH, MF_BYCOMMAND | MF_UNCHECKED);
		
        break;
        
    default:
        
        break;
    }
	
	if ((pMainDlg->u8PlayMode == VIEW_MODE) || (pMainDlg->s8ImgNum != 2) || (s8DlgIdx == 2))
	{
        pSubMenu->EnableMenuItem(ID_MENUITEM_GOSAMEFRAME, MF_BYCOMMAND | MF_GRAYED);
	}
	else
	{
        pSubMenu->EnableMenuItem(ID_MENUITEM_GOSAMEFRAME, MF_BYCOMMAND | MF_ENABLED);
	}

	if (pMainDlg->u8PlayMode == VIEW_MODE)
	{
		pSubMenu->EnableMenuItem(ID_MENUITEM_NOTICE, MF_BYCOMMAND | MF_GRAYED);
		pSubMenu->CheckMenuItem(ID_MENUITEM_NOTICE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	else
	{
        pSubMenu->EnableMenuItem(ID_MENUITEM_NOTICE, MF_BYCOMMAND | MF_ENABLED);

		if (pMainDlg->bNoticeFlag == TRUE)
		{
			pSubMenu->CheckMenuItem(ID_MENUITEM_NOTICE, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			pSubMenu->CheckMenuItem(ID_MENUITEM_NOTICE, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}

    pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, cursorPos.x, cursorPos.y, this);   //在指定位置显示弹出菜单
}

void CChildDlg::set_image_mode(uint8 u8ImageMode) 
{
    // TODO: Add your command handler code here
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
    
    
    pMainDlg->u8ImageMode   = u8ImageMode;
    if (pMainDlg->get_play_status() != PLAY_STATUS)
    {
        pMainDlg->show_one_frame(FALSE, false);
    }
}

void CChildDlg::enable_mbinfo_dlg()
{
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
    
    
	pMainDlg->bShowMBInfo	= FALSE;	//++ bEnMBInfo 状态切换之后默认不显示宏块信息
    pMainDlg->bEnMBInfo	= !pMainDlg->bEnMBInfo;
	if (pMainDlg->bEnMBInfo == FALSE)
	{
		pMainDlg->hide_MBinfo_dlg();
	}
}

void CChildDlg::separate_pix_from_3plane()
{
	int32 i, j;
	int32 iBitShift = 0, iPixValue = 0;
	int32 s32LumaWidth = s32Width;
	int32 s32ChroWidth = s32Width >> u8Sample_x;
	uint8 *pLuma = pOrigYUV[0] + (s32ViewMBy * s32LumaWidth + s32ViewMBx);
	uint8 *pCb = pOrigYUV[1] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx));
	uint8 *pCr = pOrigYUV[2] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx));
	//第一平面
	for (j = 1; j < 1 + MBInfoDlg.u8LumaPointNumY; j++)
	{
		for (i = 1; i < 1 + MBInfoDlg.u8LumaPointNumX; i++)
		{
			iPixValue = pLuma[(j - 1) * s32LumaWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
		}
	}
	//第二平面
	for (j = 17; j < 17 + MBInfoDlg.u8ChroPointNumY; j++)
	{
		for (i = 1; i < 1 + MBInfoDlg.u8ChroPointNumX; i++)
		{
			iPixValue = pCb[(j - 17) * s32ChroWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pCb[(j - 17) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pCb[(j - 17) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
		}
	}
	//第三平面
	for (j = 33; j < 33 + MBInfoDlg.u8ChroPointNumY; j++)
	{
		for (i = 1; i < 1 + MBInfoDlg.u8ChroPointNumX; i++)
		{
			iPixValue = pCr[(j - 33) * s32ChroWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pCr[(j - 33) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pCr[(j - 33) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
		}
	}
}

void CChildDlg::separate_pix_from_2plane()
{
	int32 i, j, k;
	int32 iBitShift = 0, iPixValue = 0;
	int32 s32LumaWidth = s32Width;
	int32 s32ChroWidth = s32Width >> u8Sample_x;
	int32 s32ChroHeigh = s32Height >> u8Sample_y;
	uint8 *pLuma = pOrigYUV[0] + (s32ViewMBy * s32LumaWidth + s32ViewMBx);
	uint8 *pCb;// = pOrigYUV[1] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> u8Sample_x);
	uint8 *pCr;// = pOrigYUV[2] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> u8Sample_x);
	//第一平面
	for (j = 1; j < 1 + MBInfoDlg.u8LumaPointNumY; j++)
	{
		for (i = 1; i < 1 + MBInfoDlg.u8LumaPointNumX; i++)
		{
			iPixValue = pLuma[(j - 1) * s32LumaWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
		}
	}
	//第二平面的像素和第三平面的像素交错存放(如NV21/NV12)
	int32 s32ViewMByPos = (s32ViewMBy >> u8Sample_y);
	int32 s32ChroWidthPackt = s32ChroWidth << 1;

	for (j = 17; j < 17 + MBInfoDlg.u8ChroPointNumY; j++)
	{
		if (s32ViewMByPos < s32ChroHeigh){
			pCb = pOrigYUV[1] + (s32ViewMBy >> u8Sample_x) * s32ChroWidthPackt + ((s32ViewMBx >> u8Sample_x) << 1);// ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> u8Sample_x);
		}
		else{
			pCb = pOrigYUV[2] + ((s32ViewMBy >> u8Sample_x) - s32ChroHeigh) * s32ChroWidthPackt + ((s32ViewMBx >> u8Sample_x) << 1);
		}

		for (i = 1, k = 1; i < 1 + (MBInfoDlg.u8ChroPointNumX << 1); i += 2)
		{
			iPixValue = pCb[(j - 17) * s32ChroWidthPackt + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pCb[(j - 17) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pCb[(j - 17) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][k++] = iPixValue;
		}
	}

	for (j = 33; j < 33 + MBInfoDlg.u8ChroPointNumY; j++)
	{
		if (s32ViewMByPos < s32ChroHeigh){
			pCb = pOrigYUV[1] + (s32ViewMBy >> u8Sample_x) * s32ChroWidthPackt + ((s32ViewMBx >> u8Sample_x) << 1);// ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> u8Sample_x);
		}
		else{
			pCb = pOrigYUV[2] + ((s32ViewMBy >> u8Sample_x) - s32ChroHeigh) * s32ChroWidthPackt + ((s32ViewMBx >> u8Sample_x) << 1);
		}

		for (i = 2, k = 1; i < 2 + (MBInfoDlg.u8ChroPointNumX << 1); i += 2)
		{
			iPixValue = pCb[(j - 33) * s32ChroWidthPackt + (i - 1)];
			if (u8BitDepth != BIT_DEPTH_8)
			{
				iBitShift = 1;
				iPixValue = pCb[(j - 33) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pCb[(j - 33) * (s32ChroWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][k++] = iPixValue;
		}
	}
}

void CChildDlg::get_pixel_value()
{
    uint8 u8LumaPointNumX;		//++ 亮度水平方向需要显示的点数
    uint8 u8LumaPointNumY;		//++ 亮度垂直方向需要显示的点数
    uint8 u8ChroPointNumX;		//++ 色度水平方向需要显示的点数
    uint8 u8ChroPointNumY;		//++ 色度垂直方向需要显示的点数
	uint8 u8PlaneNum;			//++ 参见下面两行的解释
  
    //++ 启用临界区保护
    CCriticalSection	CriticalSection(pCriticalSection);
    u8LumaPointNumX		 = min(s32Width - s32ViewMBx, 16);
    u8LumaPointNumY		 = min(s32Height - s32ViewMBy, 16);
    u8ChroPointNumX		 = (u8LumaPointNumX >> u8Sample_x);
    u8ChroPointNumY		 = (u8LumaPointNumY >> u8Sample_y);
    MBInfoDlg.u8LumaPointNumX	 = u8LumaPointNumX;
    MBInfoDlg.u8LumaPointNumY	 = u8LumaPointNumY;
    MBInfoDlg.u8ChroPointNumX	 = u8ChroPointNumX;
    MBInfoDlg.u8ChroPointNumY	 = u8ChroPointNumY;

	switch (u8SampleFormat)
	{
	case YUV400:
	case YUV420:
	case YUV422:
	case YUV444:
	case RGB24:
	case GBR24:
		u8PlaneNum = 3;
		break;
	case NV12:
	case NV21:
		u8PlaneNum = 2;
	default:
		break;
	}
	//NV12和NV21属于YUV420格式，是一种two - plane模式，即Y和UV分为两个Plane，但是UV（CbCr）为交错存储，而不是分为三个plane
	//YV12是一种three - plane模式，即Y、U和V三个Plane，但是UV（CbCr）为连续存储，即UUUU… VVVV…。
	if (u8PlaneNum == 3){
		separate_pix_from_3plane();
	}
	if (u8PlaneNum == 2){
		separate_pix_from_2plane();
	}
}

void CChildDlg::draw_dash_frame(CRect &cRect)
{
    CWnd	*pDesktop		= GetDesktopWindow();
    CDC		*pdcDesktop		= pDesktop->GetWindowDC();
	
	
	pdcDesktop->DrawFocusRect(&cRect);
    pDesktop->ReleaseDC(pdcDesktop);
}

void CChildDlg::change_size(LPRECT pRect)
{
	int32	s32NewWidth;
	int32	s32NewHeight;
	int32	s32WndWidth;
	int32	s32WndHeight;
	RECT	*pRc	= (RECT*)pRect;


	newSizeRect		= CRect(pRc);
	s32NewWidth		= newSizeRect.Width();
	s32NewHeight	= newSizeRect.Height();
	s32WndWidth		= wndRect.Width();
	s32WndHeight	= wndRect.Height();

	if (s32NewHeight != s32WndHeight)
	{
		newSizeRect.right	= wndRect.left + s32WndWidth * s32NewHeight / s32WndHeight;
	}
	else
	{
		newSizeRect.bottom	= wndRect.top + s32WndHeight * s32NewWidth / s32WndWidth;
	}

	draw_dash_frame(oldSizeRect);
	draw_dash_frame(newSizeRect);
	pRc->left		= wndRect.left;
	pRc->right		= wndRect.right;
	pRc->top		= wndRect.top;
	pRc->bottom		= wndRect.bottom;
	oldSizeRect		= newSizeRect;
}

void CChildDlg::change_location(LPRECT pRect)
{
	RECT	*pRc	= (RECT*)pRect;


	newSizeRect		= CRect(pRc);
	oldSizeRect		= newSizeRect;
}

void CChildDlg::update_image()
{
	int32	s32ScreenWidth		= GetSystemMetrics(SM_CXSCREEN);
	int32	s32ScreenHeight		= GetSystemMetrics(SM_CYSCREEN) - 64;
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;


    CRect	crRect(0, 0, s32Width, s32Height);
    CalcWindowRect(&crRect, CWnd::adjustOutside);
    s32ZoomWidth	 = s32Width * newSizeRect.Width() / crRect.Width();
    s32ZoomHeight	 = s32Height * newSizeRect.Width() / crRect.Width();

	if (s32ZoomWidth >= s32ScreenWidth)
	{
		s32ZoomHeight	= s32Height * s32ScreenWidth / s32Width;
		s32ZoomWidth	= s32ScreenWidth;
	}
	if (s32ZoomHeight >= s32ScreenHeight)
	{
		s32ZoomWidth	= s32Width * s32ScreenHeight / s32Height;
		s32ZoomHeight	= s32ScreenHeight;
	}

//	SetWindowPos(NULL, newSizeRect.left, newSizeRect.top, newSizeRect.Width(), newSizeRect.Height(), SWP_NOSENDCHANGING);	//prevent receiving more moving message
	draw_dash_frame(oldSizeRect);
	resize_window();
	
	s32ViewBlkX		= s32ViewMBx * s32ZoomWidth / s32Width;
	s32ViewBlkY		= s32ViewMBy * s32ZoomHeight / s32Height;
	s32ViewBlkW		= (s32ViewMBx + 16) * s32ZoomWidth / s32Width - s32ViewBlkX;
	s32ViewBlkH		= (s32ViewMBy + 16) * s32ZoomHeight / s32Height - s32ViewBlkY;
#if CONGIF_VIEW_LCU
	s32ViewBlkX_Lcu = s32ViewMBx_Lcu * s32ZoomWidth / s32Width;
	s32ViewBlkY_Lcu = s32ViewMBy_Lcu * s32ZoomHeight / s32Height;
	s32ViewBlkW_Lcu = (s32ViewMBx_Lcu + 64) * s32ZoomWidth / s32Width - s32ViewBlkX_Lcu;
	s32ViewBlkH_Lcu = (s32ViewMBy_Lcu + 64) * s32ZoomHeight / s32Height - s32ViewBlkY_Lcu;
#endif

	ShowWindow(SW_SHOW);
    GetWindowRect(&wndRect);
    oldSizeRect	= wndRect;

	pMainDlg->layout_windows();
}
BEGIN_MESSAGE_MAP(CChildDlg, CDialog)
END_MESSAGE_MAP()


LPBYTE CChildDlg::getRGBBuff()
{
	return pRGBBuff;
}
