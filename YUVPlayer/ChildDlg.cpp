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

#if LCU
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

void CChildDlg::color_space_convert(uint8 u8ImageMode)
{
    int32	i;
    int32	j;
    int32	s32ChroWidth;
	int32	s32ChroHeigth;
	//if (0)
	{
		bool reFlag = 0;
		Mat dstRgb(s32Height, s32Width, CV_8UC3);
		Mat srcYuv(s32Height * 3 / 2, s32Width, CV_8UC1);
		if (u8SampleFormat == NV12 || u8SampleFormat == NV21)
		{
			reFlag = 1;
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
				//pRGBBuff = dstRgb.data;
				break;
			}
		}
		if (reFlag)
			return;
	}
    
    switch (u8SampleFormat)
    {
    case YUV400:
    case YUV420:
        s32ChroWidth    = s32Width >> 1;
        s32ChroHeigth   = s32Height >> 1;
        break;
	case YUV422:
		s32ChroWidth = s32Width >> 1;
		s32ChroHeigth = s32Height;
		break;
#if ADDFORMAT
	case RGB8:
	case GBR8:
	case YUV444:
		s32ChroWidth = s32Width;
		s32ChroHeigth = s32Height;
		break;
#endif
    default:
        break;
    }
#if BITDEPTH //add
	if (u8BitDepth != BIT_DEPTH8)
	{
		//s32ChroWidth = s32ChroWidth << 1;
	}
#endif
    switch(u8ImageMode)
    {
    case IMAGE_YUV:
		YV12_to_RGB24(pOrigYUV[0], pOrigYUV[1], pOrigYUV[2], u8ImageMode);
        break;
        
    case IMAGE_Y:
		YV12_to_RGB24(pOrigYUV[0], pDisplayChro, pDisplayChro, u8ImageMode);
        break;
        
    case IMAGE_U:
#if ADDFORMAT
		switch (u8SampleFormat)
		{
			case YUV400:
			case YUV420:
#if BITDEPTH
				if (u8BitDepth != BIT_DEPTH8)
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width/2; i++)
						{
							pDisplayLuma[j * (s32Width*2) + (i*4)] = pOrigYUV[1][(j>>1) * (s32ChroWidth * 2) + (i<<1)];
							pDisplayLuma[j * (s32Width*2) + (i*4)+1] = pOrigYUV[1][(j>>1) * (s32ChroWidth * 2) + (i<<1) + 1];
							pDisplayLuma[j * (s32Width * 2) + (i*4)+2] = pOrigYUV[1][(j >> 1) * (s32ChroWidth * 2) + (i<<1)];
							pDisplayLuma[j * (s32Width * 2) + (i*4)+3] = pOrigYUV[1][(j >> 1) * (s32ChroWidth * 2) + (i<<1) + 1];
						}
					}
				}
				else
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * s32Width + i] = pOrigYUV[1][(j >> 1) * s32ChroWidth + (i >> 1)];
						}
					}
				}
				break;
#else
				for (j = 0; j < s32Height; j++)
				{
					for (i = 0; i < s32Width; i++)
					{
						pDisplayLuma[j * s32Width + i] = pOrigYUV[1][(j >> 1) * s32ChroWidth + (i >> 1)];
					}
				}
				break;
#endif
			case RGB8:
			case GBR8:
			case YUV444:
#if BITDEPTH
				if (u8BitDepth != BIT_DEPTH8)
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * (s32Width << 1) + (i<<1)] = pOrigYUV[1][j * (s32ChroWidth<<1) + (i<<1)];
							pDisplayLuma[j * (s32Width << 1) + (i<<1) + 1] = pOrigYUV[1][j * (s32ChroWidth<<1) + (i<<1) + 1];
						}
					}
				}
				else
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * s32Width + i] = pOrigYUV[1][j * s32ChroWidth + i];
						}
					}
				}
				break;
#else
				for (j = 0; j < s32Height; j++)
				{
					for (i = 0; i < s32Width; i++)
					{
						pDisplayLuma[j * s32Width + i] = pOrigYUV[1][j * s32ChroWidth + i];
					}
				}
				break;
#endif
			default:
				break;
		}
#else
		for (j = 0; j < s32Height; j++)
		{
			for (i = 0; i < s32Width; i++)
			{
				pDisplayLuma[j * s32Width + i] = pOrigYUV[1][(j >> 1) * s32ChroWidth + (i >> 1)];
			}
		}
#endif  
		YV12_to_RGB24(pDisplayLuma, pDisplayChro, pDisplayChro, u8ImageMode);

        break;
        
    case IMAGE_V:
#if ADDFORMAT
		switch (u8SampleFormat)
		{
			case YUV400:
			case YUV420:
#if BITDEPTH
				if (u8BitDepth != BIT_DEPTH8)
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width/2; i++)
						{
							pDisplayLuma[j * (s32Width * 2) + (i * 4)] = pOrigYUV[2][(j >> 1) * (s32ChroWidth * 2) + (i << 1)];
							pDisplayLuma[j * (s32Width * 2) + (i * 4) + 1] = pOrigYUV[2][(j >> 1) * (s32ChroWidth * 2) + (i << 1) + 1];
							pDisplayLuma[j * (s32Width * 2) + (i * 4) + 2] = pOrigYUV[2][(j >> 1) * (s32ChroWidth * 2) + (i << 1)];
							pDisplayLuma[j * (s32Width * 2) + (i * 4) + 3] = pOrigYUV[2][(j >> 1) * (s32ChroWidth * 2) + (i << 1) + 1];
						}
					}
					break;
				}
				else
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * s32Width + i] = pOrigYUV[2][(j >> 1) * s32ChroWidth + (i >> 1)];
						}
					}
					break;
				}

#else
				for (j = 0; j < s32Height; j++)
				{
					for (i = 0; i < s32Width; i++)
					{
						pDisplayLuma[j * s32Width + i] = pOrigYUV[2][(j >> 1) * s32ChroWidth + (i >> 1)];
					}
				}
				break;
#endif
			case RGB8:
			case GBR8:
			case YUV444:
#if BITDEPTH
				if (u8BitDepth != BIT_DEPTH8)
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * (s32Width << 1) + (i<<1)] = pOrigYUV[2][j * (s32ChroWidth<<1) + (i<<1)];
							pDisplayLuma[j * (s32Width << 1) + (i<<1) + 1] = pOrigYUV[2][j * (s32ChroWidth<<1) + (i<<1) + 1];
						}
					}
					break;
				}
				else
				{
					for (j = 0; j < s32Height; j++)
					{
						for (i = 0; i < s32Width; i++)
						{
							pDisplayLuma[j * s32Width + i] = pOrigYUV[2][j * s32ChroWidth + i];
						}
					}
					break;
				}

#else
				for (j = 0; j < s32Height; j++)
				{
					for (i = 0; i < s32Width; i++)
					{
						pDisplayLuma[j * s32Width + i] = pOrigYUV[2][j * s32ChroWidth + i];
					}
				}
				break;
#endif
			default:
				break;
		}
#else
		for (j = 0; j < s32Height; j++)
		{
			for (i = 0; i < s32Width; i++)
			{
				pDisplayLuma[j * s32Width + i] = pOrigYUV[2][(j >> 1) * s32ChroWidth + (i >> 1)];
			}
		}
#endif  
		YV12_to_RGB24(pDisplayLuma, pDisplayChro, pDisplayChro, u8ImageMode);

        break;
        
    default:

        break;
    }
}

void CChildDlg::YV12_to_RGB24(uint8* pu8Y, uint8* pu8U, uint8* pu8V, uint8 u8ImageMode)
{
	int32	x;
	int32	y;
	int32	k;      //bmp（rgb格式）空间的位置索引，bmp图像从下往上扫描
	int32	yStride	= 0;    //y横向跨度
	int32	cbStride = 0;   //uv横向跨度
	int32   subSampleY = 0; //y方向是否下采样
	int32	rgb[3];
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
#if ADDFORMAT
			//////计算位置
			switch (u8SampleFormat)
			{
				case YUV400:
					subSampleY = 0;
					yPos = yStride + x;
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
					{
						yPos = (yStride << 1) + (x << 1);
					}
					break;
				case YUV420:
					subSampleY = 1;
					yPos = yStride + x;
					cbPos = cbStride + (x >> 1);
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
					{
						yPos = (yStride << 1) + (x << 1);
						cbPos = (cbStride << 1) + ((x >> 1) << 1);
					}
					break;
				case YUV422:
					subSampleY = 0;
					yPos = yStride + x;
					cbPos = cbStride + (x >> 1);
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
					{
						yPos = (yStride << 1) + (x << 1);
						cbPos = (cbStride << 1) + ((x >> 1) << 1);
					}
					break;
				case YUV444:
				case RGB8:
				case GBR8:		
					subSampleY = 0;
					yPos = yStride + x;
					cbPos = yPos;
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
					{
						yPos = (yStride << 1) + (x << 1);
						cbPos = yPos;
					}
					break;
				default:
					break;
			}
#else
			yPos = yStride + x;
			cbPos = cbStride + (x >> 1);
#endif
#if ADDFORMAT
			//////像素转换
			int32 yTemp, uTemp, vTemp;//转换rgb\bgr格式所需变量
			int  rTemp, gTemp, bTemp; //r,g,b临时变量
			switch (u8SampleFormat)
			{
				case YUV400:
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
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
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
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
				case RGB8:
					if (u8BitDepth != BIT_DEPTH8/* && u8ImageMode == IMAGE_YUV*/)
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
				case GBR8:
					if (u8BitDepth != BIT_DEPTH8 /*&& u8ImageMode == IMAGE_YUV*/)
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
#else
			rgb[2]	= int32(1.164383 * (pu8Y[yPos] - 16) + 1.596027 * (pu8V[cbPos] - 128)); // r
			rgb[1] = int32(1.164383 * (pu8Y[yPos] - 16) - 0.812968 * (pu8V[cbPos] - 128) - 0.391762 * (pu8U[cbPos] - 128)); // g
			rgb[0] = int32(1.164383 * (pu8Y[yPos] - 16) + 2.017232 * (pu8U[cbPos] - 128)); // b			
#endif
			/////将转换好的rgb像素回填到RGB buffer中
			int j;
			yPos = k + x * 3;
			for(j = 0; j < 3; j ++)
			{
				if((rgb[j] >= 0) && (rgb[j] <= 255))
				{
					pRGBBuff[yPos + j] = rgb[j];
				}
				else
				{
					pRGBBuff[yPos + j] = (rgb[j] < 0) ? 0 : 255;
				}
			}
		}
		
		/////位置偏移到下一行
		yStride += s32Width;

		//if(y % (1<<subSampleY) )
		{
#if ADDFORMAT
			switch (u8SampleFormat)
			{
				case YUV400:
				case YUV420:
					cbStride += (y % 2) ? (s32Width >> 1) : 0;
					break;
				case YUV422:
					cbStride += (s32Width >> 1);
					break;
				case RGB8:
				case GBR8:
				case YUV444:
					cbStride += s32Width;
					break;
				default:
					break;
			}
#else
			cbStride += (s32Width >> 1);
#endif
		}
	}
}

void CChildDlg::YUY2_to_RGB24(uint8 *pu8RGBData, uint8 *pu8YUVData)
{
	int32  R, G, B;
	int32  x, y;
	int32  Y0, U, Y1, V;


	for (y = 0; y < s32Height; y ++)
	{
		for (x = 0; x < s32Width; x += 2)
		{
			Y0 = *pu8YUVData++;
			U  = *pu8YUVData++;
			Y1 = *pu8YUVData++;
			V  = *pu8YUVData++;

			R  = int32(1.164383 * (Y0 - 16) + 1.596027 * (V - 128));
			G  = int32(1.164383 * (Y0 - 16) - 0.812968 * (V - 128) - 0.391762 * (U - 128));
			B  = int32(1.164383 * (Y0 - 16) + 2.017232 * (U - 128));
			if ( R < 0 )
				R = 0;
			if ( R > 255)
				R = 255;
			if ( G < 0 )
				G = 0;
			if ( G > 255)
				G = 255;
			if ( B < 0 )
				B = 0;
			if ( B > 255 )
				B = 255;
			*pu8RGBData++ = (uint8)B;
			*pu8RGBData++ = (uint8)G;
			*pu8RGBData++ = (uint8)R;

			R  = int32(1.164383 * (Y1 - 16) + 1.596027 * (V - 128));
			G  = int32(1.164383 * (Y1 - 16) - 0.812968 * (V - 128) - 0.391762 * (U - 128));
			B  = int32(1.164383 * (Y1 - 16) + 2.017232 * (U - 128));
			if ( R < 0 )
				R = 0;
			if ( R > 255)
				R = 255;
			if ( G < 0 )
				G = 0;
			if ( G > 255)
				G = 255;
			if ( B < 0 )
				B = 0;
			if ( B > 255 )
				B = 255;
			*pu8RGBData++ = (uint8)B;
			*pu8RGBData++ = (uint8)G;
			*pu8RGBData++ = (uint8)R;
		}
	}
}

void CChildDlg::show_macroblock_info()
{
    CYUVPlayerDlg	*pMainDlg	 = (CYUVPlayerDlg *)this->pMainDlg;
#if LCU
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
#if LCU
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

#if LCU
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
#if LCU
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
#if LCU
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
#if LCU
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

void CChildDlg::get_pixel_value()
{
    uint8	u8LumaPointNumX;		//++ 亮度水平方向需要显示的点数
    uint8	u8LumaPointNumY;		//++ 亮度垂直方向需要显示的点数
    uint8	u8ChroPointNumX;		//++ 色度水平方向需要显示的点数
    uint8	u8ChroPointNumY;		//++ 色度垂直方向需要显示的点数
    int32	i;
    int32	j;
    int32	s32LumaWidth	 = s32Width;
    int32	s32ChroWidth	 = s32Width >> 1;
    uint8	*pLuma		 = pOrigYUV[0] + (s32ViewMBy * s32LumaWidth + s32ViewMBx);
    uint8	*pCb		 = pOrigYUV[1] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> 1);
    uint8	*pCr		 = pOrigYUV[2] + ((s32ViewMBy * s32ChroWidth + s32ViewMBx) >> 1);
    
    
    //++ 启用临界区保护
    CCriticalSection	CriticalSection(pCriticalSection);
    u8LumaPointNumX		 = min(s32Width - s32ViewMBx, 16);
    u8LumaPointNumY		 = min(s32Height - s32ViewMBy, 16);
    u8ChroPointNumX		 = (u8LumaPointNumX >> 1);
    u8ChroPointNumY		 = (u8LumaPointNumY >> 1);
    MBInfoDlg.u8LumaPointNumX	 = u8LumaPointNumX;
    MBInfoDlg.u8LumaPointNumY	 = u8LumaPointNumY;
    MBInfoDlg.u8ChroPointNumX	 = u8ChroPointNumX;
    MBInfoDlg.u8ChroPointNumY	 = u8ChroPointNumY;
#if PIXMEND
	int iBitShift=0, iPixValue = 0;
#endif
    for (j = 1; j < 1 + u8LumaPointNumY; j ++)
    {
        for (i = 1; i < 1 + u8LumaPointNumX; i ++)
        {
#if PIXMEND
			iPixValue = pLuma[(j - 1) * s32LumaWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH8)
			{
				iBitShift = 1;
				iPixValue = pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift) + 1] << 8 | pLuma[(j - 1) * (s32LumaWidth << iBitShift) + ((i - 1) << iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i]	 = iPixValue;
#else
			MBInfoDlg.pixelValue[j][i]	 = pLuma[(j - 1) * s32LumaWidth + (i - 1)];
#endif
        }
    }
    for (j = 18; j < 18 + u8ChroPointNumY; j ++)
    {
        for (i = 1; i < 1 + u8ChroPointNumX; i ++)
        {
#if PIXMEND
			iPixValue = pCb[(j - 18) * s32ChroWidth + (i - 1)];
			if (u8BitDepth != BIT_DEPTH8)
			{
				iBitShift = 1;
				iPixValue = pCb[(j - 18) * (s32ChroWidth << iBitShift) + ((i - 1)<<iBitShift) + 1] << 8 | pCb[(j - 18) * (s32ChroWidth <<iBitShift) + ((i - 1)<<iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
#else
			MBInfoDlg.pixelValue[j][i]	 = pCb[(j - 18) * s32ChroWidth + (i - 1)];
#endif
        }
        
        for (i = 9; i < 9 + u8ChroPointNumX; i ++)
        {
#if PIXMEND
			iPixValue = pCr[(j - 18) * s32ChroWidth + (i - 9)];
			if (u8BitDepth != BIT_DEPTH8)
			{
				iBitShift = 1;
				iPixValue = pCr[(j - 18) * (s32ChroWidth <<iBitShift) + ((i - 9) <<iBitShift) + 1] << 8 | pCr[(j - 18) * (s32ChroWidth <<iBitShift) + ((i - 9)<<iBitShift)];
			}
			MBInfoDlg.pixelValue[j][i] = iPixValue;
#else
			MBInfoDlg.pixelValue[j][i]	 = pCr[(j - 18) * s32ChroWidth + (i - 9)];
#endif
        }
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
#if LCU
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
