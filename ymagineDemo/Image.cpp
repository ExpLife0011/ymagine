


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ԥ����
#include "Image.h"
#include <OleCtl.h>

#define _USE_MATH_DEFINES
#include <Math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ȡͼ�����ݡ����� TRUE ��ʾ�ɹ���������ͼ�����ݣ����� FALSE ��ʾʧ�ܣ�ԭ��ͼ�����ݲ��ᱻ����
BOOL CImage::Load(PCTSTR ptzFileName)
{
	HDC hDC;
	BOOL bResult;
	HBITMAP hBitmap;
	BITMAP bmpBitmap;
	IPicture *pPicture;
	BITMAPINFOHEADER bihInfo;
	WCHAR wzFileName[MAX_PATH];

	bResult = FALSE;
	hBitmap = NULL;
	pPicture = NULL;

	// ��ȡ�ļ���
	_StrToWStr(wzFileName, ptzFileName);

	// ʹ�� IPicture �ӿڴ�ͼ���ļ�
	if (OleLoadPicturePath(wzFileName, NULL, 0, 0, IID_IPicture, (PVOID *) &pPicture) == S_OK)
	{
		// ����ȡλͼ���
		pPicture->get_Handle((OLE_HANDLE *) &hBitmap);
	}

	// ����� IPicture �ӿڴ�ʧ�ܣ������� LoadImage() ��ͼ��
	if (hBitmap == NULL)
	{
		hBitmap = (HBITMAP) LoadImage(NULL, ptzFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	}

	if (hBitmap)
	{
		// ��ȡͼ������
		if (GetObject(hBitmap, sizeof(BITMAP), &bmpBitmap))
		{
			if (Create(bmpBitmap.bmWidth, bmpBitmap.bmHeight))
			{
				m_dScale = 1;
				bResult = TRUE;
				GetBih(&bihInfo);
				hDC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
				GetDIBits(hDC, hBitmap, 0, m_uHeight, m_pbBits, (PBITMAPINFO) &bihInfo, DIB_RGB_COLORS);
				DeleteDC(hDC);
			}
		}
	}

	// ����� pPicture �ӿڴ򿪵�ͼ�����ͷţ����Զ����٣�
	if (pPicture)
	{
		pPicture->Release();
	}
	// ������� LoadImage() �򿪵�ͼ��������
	else if (hBitmap)
	{
		DeleteObject(hBitmap);
	}

	return bResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����ͼ������
BOOL CImage::Save(PCTSTR ptzFileName, LPCRECT prtRect)
{
	UINT i;
	UINT uMin;
	UINT uMax;
	UINT uLeft;
	UINT uRight;
	BOOL bResult;
	HANDLE hFile;
	DWORD dwSize;
	DWORD dwPitch;
	BITMAPFILEHEADER bihFile;
	BITMAPINFOHEADER bihInfo;

	// ���� BITMAPINFOHEADER �ṹ
	GetBih(&bihInfo);
	if (prtRect)
	{
		bihInfo.biWidth = _RectWidth(*prtRect);
		bihInfo.biHeight = _RectHeight(*prtRect);
		dwPitch = _DibPitch24(bihInfo.biWidth);
		bihInfo.biSizeImage = dwPitch * bihInfo.biHeight;

		uMin = GetHeight() - max(prtRect->top, prtRect->bottom) - 1;
		uMax = GetHeight() - min(prtRect->top, prtRect->bottom) - 1;
		uLeft = min(prtRect->left, prtRect->right);
		uRight = max(prtRect->left, prtRect->right);
		if ((uMax >= GetHeight()) || (uRight > GetWidth()))
		{
			return FALSE;
		}
	}

	// �����ļ�
	bResult = FALSE;
	hFile = _CreateFileForWrite(ptzFileName);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ���� BITMAPFILEHEADER �ṹ
		bihFile.bfType = 0x4D42;
		bihFile.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bihInfo.biSizeImage;
		bihFile.bfReserved1 = 0;
		bihFile.bfReserved2 = 0;
		bihFile.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		// д�� BITMAPFILEHEADER  ��Ϣ
		bResult = WriteFile(hFile, &bihFile, sizeof(BITMAPFILEHEADER), &dwSize, 0);
		if (bResult)
		{
			// д�� BITMAPINFOHEADER ��Ϣ
			WriteFile(hFile, &bihInfo, sizeof(BITMAPINFOHEADER), &dwSize, 0);

			// д�� DIB ����
			if (prtRect)
			{
				for (i = uMin; i <= uMax; i++)
				{
					WriteFile(hFile, GetBits(uLeft, i), dwPitch, &dwSize, NULL);
				}
			}
			else
			{
				bResult = WriteFile(hFile, GetBits(), bihInfo.biSizeImage, &dwSize, 0);
			}
		}

		// �ر��ļ����
		CloseHandle(hFile);
	}

	return bResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����ͼ��
VOID CImage::Draw(HDC hDC, INT iLeft, INT iTop)
{
	BITMAPINFOHEADER bihInfo;

	if (m_pbBits)
	{
		GetBih(&bihInfo);
		SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(hDC,
			iLeft, iTop, GetScaleWidth(), GetScaleHeight(),
			0, 0, m_uWidth, m_uHeight,
			m_pbBits, (PBITMAPINFO) &bihInfo, DIB_RGB_COLORS, SRCCOPY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����ͼ��
VOID CImage::Stretch(UINT uWidth, UINT uHeight)
{
	HDC hDC;
	HDC hSrcDC;
	HDC hDestDC;
	PBYTE pbSrc;
	PBYTE pbDest;
	HBITMAP hSrcBmp;
	HBITMAP hDestBmp;
	HBITMAP hOldSrcBmp;
	HBITMAP hOldDestBmp;
	BITMAPINFOHEADER bihInfo;

	if (m_pbBits)
	{
		Backup();

		// �����豸����
		hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
		hSrcDC = CreateCompatibleDC(hDC);
		hDestDC = CreateCompatibleDC(hDC);

		// ����ԭͼ��� DIBSection ����
		GetBih(&bihInfo);
		hSrcBmp = CreateDIBSection(hDC, (PBITMAPINFO) &bihInfo, DIB_RGB_COLORS, (VOID **) &pbSrc, NULL, 0 );
		CopyMemory(pbSrc, m_pbBits, GetSize());

		// ����Ŀ��ͼ��� DIBSection ����
		bihInfo.biWidth = uWidth;
		bihInfo.biHeight = uHeight;
		bihInfo.biSizeImage = _DibSize24(uWidth, uHeight);
		hDestBmp = CreateDIBSection(hDC, (PBITMAPINFO) &bihInfo, DIB_RGB_COLORS, (PVOID *) &pbDest, NULL, 0);

		// ѡ��ͼ��
		hOldSrcBmp = (HBITMAP) SelectObject(hSrcDC, hSrcBmp);
		hOldDestBmp = (HBITMAP) SelectObject(hDestDC, hDestBmp);

		// ���Ż���
		SetStretchBltMode(hDestDC, COLORONCOLOR);
		StretchBlt(hDestDC, 0, 0, uWidth, uHeight,
			hSrcDC, 0, 0, m_uWidth, m_uHeight, SRCCOPY);

		// ж��ͼ��
		SelectObject(hSrcDC, hOldSrcBmp);
		SelectObject(hDestDC, hOldDestBmp);

		// ɾ���豸����
		DeleteDC(hSrcDC);
		DeleteDC(hDestDC);
		DeleteDC(hDC);

		// ��������ȡ�����ŵ�ͼ������
		Create(uWidth, uHeight, pbDest);

		// ɾ�������� DIBSection ����
		DeleteObject(hSrcBmp);
		DeleteObject(hDestBmp);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ˮƽ��תͼ��
VOID CImage::FlipHorizon()
{
	UINT y;
	PBYTE pbBits;
	PBYTE pbStop;
	RGBTRIPLE rgbTemp;

	if (m_pbBits)
	{
		Backup();

		for (y = 0; y < m_uHeight; y++)
		{
			pbBits = GetBits(y);
			pbStop = GetBits(m_uWidth - 1, y);
			while (pbBits < pbStop)
			{
				rgbTemp = *((RGBTRIPLE *) pbBits);
				*((RGBTRIPLE *) pbBits) = *((RGBTRIPLE *) pbStop);
				*((RGBTRIPLE *) pbStop) = rgbTemp;
				pbBits += 3;
				pbStop -= 3;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ֱ��תͼ��
VOID CImage::FlipVertical()
{
	UINT uPitch;
	PBYTE pbFirst;
	PBYTE pbLast;
	PBYTE pbTemp;

	if (m_pbBits)
	{
		Backup();

		uPitch = GetPitch();
		pbTemp = (PBYTE) _HeapAlloc(uPitch);
		pbFirst = m_pbBits;
		pbLast = GetBits(m_uHeight - 1);

		while (pbFirst < pbLast)
		{
			CopyMemory(pbTemp, pbFirst, uPitch);
			CopyMemory(pbFirst, pbLast, uPitch);
			CopyMemory(pbLast, pbTemp, uPitch);
			pbFirst += uPitch;
			pbLast -= uPitch;
		}
		_HeapFree(pbTemp);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��תͼ��
VOID CImage::Rotate(UINT uAngle, COLORREF crColor)
{
	INT x;
	INT y;
	INT x1;
	INT y1;
	INT x2;
	INT y2;
	INT x3;
	INT y3;
	INT xMin;
	INT yMin;
	INT xMax;
	INT yMax;
	INT iWidth;
	INT iHeight;
	INT xSource;
	INT ySource;
	DOUBLE dSin;
	DOUBLE dCos;
	PBYTE pbBits;
	DWORD dwPixel;

	// ����Ƿ���Ҫ��ת
	uAngle %= 360;
	if ((uAngle == 0) || (m_pbBits == NULL))
	{
		return;
	}

	// ����
	Backup();

	switch (uAngle)
	{
	case 90:
		// ��ת 90 ��
		iWidth = m_uHeight;
		iHeight = m_uWidth;
		pbBits = (PBYTE) _HeapAlloc(_DibSize24(iWidth, iHeight));

		for (y = 0; y < iWidth; y++)
		{
			for (x = 0; x < iHeight; x++)
			{
				*((RGBTRIPLE *) _DibBits24(pbBits, iWidth, (iWidth - y - 1), x)) = *((RGBTRIPLE *) GetBits(x, y));
			}
		}
		break;

	case 180:
		// ��ת 180 ��
		iWidth = m_uWidth;
		iHeight = m_uHeight;
		pbBits = (PBYTE) _HeapAlloc(_DibSize24(iWidth, iHeight));

		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{
				*((RGBTRIPLE *) _DibBits24(pbBits, iWidth, (iWidth - x - 1), (iHeight - y - 1))) =
					*((RGBTRIPLE *) GetBits(x, y));
			}
		}
		break;

	case 270:
		// ��ת 270 ��
		iWidth = m_uHeight;
		iHeight = m_uWidth;
		pbBits = (PBYTE) _HeapAlloc(_DibSize24(iWidth, iHeight));

		for (y = 0; y < iWidth; y++)
		{
			for (x = 0; x < iHeight; x++)
			{
				*((RGBTRIPLE *) _DibBits24(pbBits, iWidth, y, (iHeight - x - 1))) = *((RGBTRIPLE *) GetBits(x, y));
			}
		}
		break;

	default:
		// ��ת������
		dSin = sin((DOUBLE) ((uAngle / 90.0) * (M_PI / 2)));
		dCos = cos((DOUBLE) ((uAngle / 90.0) * (M_PI / 2)));

		x1 = (INT) (m_uHeight * dSin);
		x1 = - x1;
		y1 = (INT) (m_uHeight * dCos);
		x2 = (INT) (m_uWidth * dCos - m_uHeight * dSin);
		y2 = (INT) (m_uHeight * dCos + m_uWidth * dSin);
		x3 = (INT) (m_uWidth * dCos);
		y3 = (INT) (m_uWidth * dSin);

		xMin = min(0, min(x1, min(x2, x3)));
		yMin = min(0, min(y1, min(y2, y3)));
		xMax = max(x1, max(x2, x3));
		yMax = max(y1, max(y2, y3));

		if ((xMin < 0) && (xMax < 0))
		{
			iWidth = -xMin;
		}
		else
		{
			iWidth = xMax - xMin;
		}

		if ((yMin < 0) && (yMax < 0))
		{
			iHeight = -yMin;
		}
		else
		{
			iHeight = yMax - yMin;
		}

		pbBits = (PBYTE) _HeapAlloc(_DibSize24(iWidth, iHeight));

		for(y = 0; y < iHeight; y++)
		{
			for(x = 0; x < iWidth; x++)
			{
				xSource = (INT) ((x + xMin) * dCos + (y + yMin) * dSin);
				ySource = (INT) ((y + yMin) * dCos - (x + xMin) * dSin);

				if ((xSource >= 0) && (xSource < (INT) m_uWidth) && (ySource >= 0) && (ySource < (INT) m_uHeight))
				{
					dwPixel = *((PDWORD)((PBYTE) m_pbBits + GetPitch() * ySource + xSource * 3)) & 0x00FFFFFF;
					*((PDWORD) (pbBits + _DibPitch24(iWidth) * y + x * 3)) |= dwPixel;
				}
				else
				{
					*((PDWORD) (pbBits + _DibPitch24(iWidth) * y + x * 3)) |= crColor;
				}
			}
		}
	}

	// ������ͼ��
	Attach(iWidth, iHeight, pbBits);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ͼ����ȥɫ����
VOID CImage::Decolourize()
{
	UINT x;
	UINT y;
	PBYTE pbBits;

	if (m_pbBits)
	{
		Backup();

		for (y = 0; y < m_uHeight; y++)
		{
			pbBits = GetBits(y);
			for (x = 0; x < m_uWidth; x++, pbBits += 3)
			{
				// ��Ȩ�㷨
				pbBits[0] = pbBits[1] = pbBits[2] = (11 *pbBits[0] + 59 *pbBits[1] + 30 *pbBits[2]) / 100;

				// ƽ���㷨
				//pbBits[0] = pbBits[1] = pbBits[2] = (pbBits[0] + pbBits[1] + pbBits[2]) / 3;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ͼ�����ع⴦��
VOID CImage::Expose(BYTE bValue)
{
	PBYTE pbBits;
	PBYTE pbStop;

	if (m_pbBits)
	{
		Backup();

		pbStop = m_pbBits + GetPitch() * m_uHeight;

		for (pbBits = m_pbBits; pbBits < pbStop; pbBits++)
		{
			if (*pbBits <= bValue)
			{
				*pbBits = ~(*pbBits);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ͼ����������
VOID CImage::Emboss()
{
	UINT x;
	UINT y;
	PBYTE pbBits;

	if (m_pbBits)
	{
		Backup();

		for (y = 0; y < m_uHeight; y++)
		{
			pbBits = GetBits(y);
			for (x = 2; x < m_uWidth; x++, pbBits += 3)
			{
				pbBits[0] = _Bound(pbBits[0] - pbBits[6] + 128, 0, 255);
				pbBits[1] = _Bound(pbBits[1] - pbBits[7] + 128, 0, 255);
				pbBits[2] = _Bound(pbBits[2] - pbBits[8] + 128, 0, 255);
			}
			pbBits[0] = pbBits[3] = _Bound(pbBits[0] - pbBits[3] + 128, 0, 255);
			pbBits[1] = pbBits[4] = _Bound(pbBits[1] - pbBits[4] + 128, 0, 255);
			pbBits[2] = pbBits[5] = _Bound(pbBits[2] - pbBits[5] + 128, 0, 255);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ɫ�ʵ���
VOID CImage::Adjust(INT iRed, INT iGreen, INT iBlue)
{
	UINT x;
	UINT y;
	PBYTE pbBits;

	if (m_pbBits)
	{
		Backup();

		for (y = 0; y < m_uHeight; y++)
		{
			pbBits = GetBits(y);
			for (x = 0; x < m_uWidth; x++, pbBits += 3)
			{
				pbBits[0] = _Bound(pbBits[0] + iBlue, 0, 255);
				pbBits[1] = _Bound(pbBits[1] + iGreen, 0, 255);
				pbBits[2] = _Bound(pbBits[2] + iRed, 0, 255);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��Ե���
VOID CImage::Fringe(BYTE bValue)
{
	UINT x;
	UINT y;
	PBYTE p;
	UINT uPitch;

#ifndef _TEST
	if (GetBits())
	{
		//Decolourize();
		uPitch = GetPitch();
#define Q 2
		for (y = 0; y < m_uHeight - Q; y += Q)
		{
			for (x = 0; x < m_uWidth - Q; x += Q)
			{

				UINT i, j, k;
				UINT n = 0;
				BYTE g[Q * Q] = {0};
				for (j = 0; j < Q; j++)
				{					
					for (i = 0; i < Q; i++)
					{
						p = GetBits(x + i, y + j);
						n += p[0] + p[1] + p[2];
						g[i * j] = (p[0] + p[1] + p[2]) / 3;
					}
				}
				n /= Q * Q * 3;
				n /= (256 / (Q * Q));

				BYTE o[Q * Q];
				CopyMemory(o, g, Q * Q);
				for (i = 0; i < Q * Q; i++)
				{
					for (j = i + 1; j < Q * Q; j++)
					{
						if (o[i] > o[j])
						{
							BYTE t = o[j];
							o[j] = o[i];
							o[i] = t;
						}
					}
				}
				
				k = 0;
				for (j = 0; j < Q; j++)
				{
					for (i = 0; i < Q; i++)
					{
						p = GetBits(x + i, y + j);
						if ((g[i * j] >= o[Q * Q - n]) && (k <= n))
						{
							p[0] = p[1] = p[2] = 255;
							k++;
						}
						else
						{
							p[0] = p[1] = p[2] =  0;
						}
					}
				}
			}
		}
	}
#else // _TEST
	if (GetBits())
	{
		Backup();

		uPitch = GetPitch();
		for (y = 0; y < m_uHeight - 2; y++)
		{
			p = m_pbBits + uPitch * y;
			for (x = 0; x < m_uWidth - 2; x++, p += 3)
			{
				BYTE bB1 = min(abs(p[0] - p[3]), abs(p[0] - p[6]));
				BYTE bG1 = min(abs(p[1] - p[4]), abs(p[1] - p[7]));
				BYTE bR1 = min(abs(p[2] - p[5]), abs(p[2] - p[8]));

				BYTE bB2 = min(abs(p[0] - p[uPitch + 0]), abs(p[0] - p[uPitch * 2 + 0]));
				BYTE bG2 = min(abs(p[1] - p[uPitch + 1]), abs(p[0] - p[uPitch * 2 + 1]));
				BYTE bR2 = min(abs(p[2] - p[uPitch + 2]), abs(p[0] - p[uPitch * 2 + 2]));

				if ((bB1 + bG1 + bR1 + bB2 + bG2 + bR2) > bValue)
				{
					p[0] = p[1] = p[2] = 0;
				}
				else
				{
					p[0] = p[1] = p[2] = 255;
				}
			}
		}
	}
#endif // _TEST
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
