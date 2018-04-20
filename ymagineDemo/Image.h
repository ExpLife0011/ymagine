


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ԥ����
#pragma once
#include <Windows.h>

// ���Ͷ���
#ifndef DOUBLE
typedef double DOUBLE;
#endif
#ifndef PCTSTR
typedef LPCTSTR PCTSTR;
#endif

// ���ú�����
#define _HeapAlloc(n)				HeapAlloc(GetProcessHeap(), 0, n)
#define _HeapFree(p)				HeapFree(GetProcessHeap(), 0, (PVOID) p)
#define _CreateFileForWrite(f)		CreateFile(f, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)

// ����Ԫ�ظ���
#define _NumberOf(v)				(sizeof(v) / sizeof(v[0]))

// DIB ����
#define _DibPitch(w, i)				((((w * i) + 31) & ~31) / 8)
#define _DibSize(w, i, h)			(_DibPitch(w, i) * h)
#define _DibBits(p, w, i, x, y)		(p + _DibPitch(w, i) * y + x * 3)
#define _DibPitch24(w)				((w + w + w + 3) & 0xFFFFFFFC)
#define _DibSize24(w, h)			(_DibPitch24(w) * h)
#define _DibBits24(p, w, x, y)		(p + _DibPitch24(w) * y + x * 3)

// ������ֵ��Χ
#define _Bound(v, vMin, vMax)		(((v) > (vMax)) ? (vMax) : ((v < vMin) ? (vMin) : (v)))

// ���ο�Ⱥ͸߶�
#define _RectWidth(r)				((r).right - (r).left)
#define _RectHeight(r)				((r).bottom - (r).top)

// �ַ���ת������
#define _WStrToAStrN(a, w, n)		WideCharToMultiByte(CP_ACP, 0, w, -1, a, n, NULL, NULL)
#define _AStrToWStrN(w, a, n)		MultiByteToWideChar(CP_ACP, 0, a, -1, w, n)
#define _StrToStrN(t1, t2, n)		lstrcpyn(t1, t2, n)
#ifdef _UNICODE
#define _StrToAStrN(a, t, n)		_WStrToAStrN(a, t, n)
#define _StrToWStrN(w, t, n)		lstrcpyn(w, t, n)
#define _AStrToStrN(t, a, n)		_AStrToWStrN(t, a, n)
#define _WStrToStrN(t, w, n)		lstrcpyn(t, w, n)
#else // _UNICODE
#define _StrToAStrN(a, t, n)		lstrcpyn(a, t, n)
#define _StrToWStrN(w, t, n)		_AStrToWStrN(w, t, n)
#define _AStrToStrN(t, a, n)		lstrcpyn(t, a, n)
#define _WStrToStrN(t, w, n)		_WStrToAStrN(t, w, n)
#endif // _UNICODE

// �ַ���ת���������ַ�����
#define _WStrToAStr(a, w)			_WStrToAStrN(a, w, _NumberOf(a))
#define _AStrToWStr(w, a)			_AStrToWStrN(w, a, _NumberOf(w))
#define _StrToStr(t1, t2)			_StrToStrN(t1, t2, _NumberOf(t1))
#define _StrToAStr(a, t)			_StrToAStrN(a, t, _NumberOf(a))
#define _StrToWStr(w, t)			_StrToWStrN(w, t, _NumberOf(w))
#define _AStrToStr(t, a)			_AStrToStrN(t, a, _NumberOf(t))
#define _WStrToStr(t, w)			_WStrToStrN(t, w, _NumberOf(t))
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CImage ������
class CImage
{
protected:
	// ��Ա����
	UINT m_uWidth;					// ͼ����
	UINT m_uHeight;					// ͼ��߶�
	PBYTE m_pbBits;					// ͼ������ָ��
	DOUBLE m_dScale;				// ���ڻ���ͼ�������ϵ��

	UINT m_uBackupWidth;			// ����ͼ����
	UINT m_uBackupHeight;			// ����ͼ��߶�
	PBYTE m_pbBackupBits;			// ����ͼ������ָ��

	TCHAR m_tzFileName[MAX_PATH];	// ��ǰͼ���ļ�����

public:
	// ���캯��
	CImage()
	{
		m_uWidth = 0;
		m_uHeight = 0;
		m_pbBits = NULL;
		m_dScale = 1;
		m_uBackupWidth = 0;
		m_uBackupHeight = 0;
		m_pbBackupBits = NULL;
		m_tzFileName[0] = 0;
	}

	// ��������
	virtual inline CImage::~CImage()
	{
		Destroy();
		DestroyBackup();
	}

	// ��ȡͼ����
	inline UINT GetWidth()
	{
		return m_uWidth;
	}

	// ��ȡͼ��߶�
	inline UINT GetHeight()
	{
		return m_uHeight;
	}

	// ��ȡ���ƿ��
	inline UINT GetScaleWidth()
	{
		return (UINT) (GetWidth() * GetScale());
	}

	// ��ȡͼ��߶�
	inline UINT GetScaleHeight()
	{
		return (UINT) (GetHeight() * GetScale());
	}

	// ��ȡ���ڻ���ͼ�������ϵ��
	inline DOUBLE GetScale()
	{
		return m_dScale;
	}

	// ��ȡ���ڻ���ͼ�������ϵ��
	inline VOID SetScale(DOUBLE dScale = 1)
	{
		m_dScale = dScale;
	}

	// ��ȡ��ǰͼ����ļ���������ֵָ���ļ���������ָ�룬��������
	inline PTSTR GetFileName()
	{
		return m_tzFileName;
	}

	// ��ȡһ��ͼ����ռ���ֽ���
	inline UINT GetPitch()
	{
		return _DibPitch24(m_uWidth);
	}

	// ��ȡͼ����ռ���ֽ���
	inline UINT GetSize()
	{
		return _DibSize24(m_uWidth, m_uHeight);
	}

	// ��ȡ�� y �е�ͼ������ָ��
	inline PBYTE GetBits()
	{
		return m_pbBits;
	}

	// ��ȡ�� y �е�ͼ������ָ��
	inline PBYTE GetBits(UINT y)
	{
		return m_pbBits + GetPitch() * y;
	}

	// ��ȡ�� (x, y) ���ͼ������ָ��
	inline PBYTE GetBits(UINT x, UINT y)
	{
		return m_pbBits + GetPitch() * y + x * 3;
	}

	// ��ȡ BITMAPINFOHEADER ��Ϣ
	inline VOID GetBih(PBITMAPINFOHEADER pbihInfo)
	{
		pbihInfo->biSize = sizeof(BITMAPINFOHEADER);
		pbihInfo->biWidth = m_uWidth;
		pbihInfo->biHeight = m_uHeight;
		pbihInfo->biPlanes = 1;
		pbihInfo->biBitCount = 24;
		pbihInfo->biCompression = BI_RGB;
		pbihInfo->biSizeImage = GetSize();
		pbihInfo->biXPelsPerMeter = 0;
		pbihInfo->biYPelsPerMeter = 0;
		pbihInfo->biClrUsed = 0;
		pbihInfo->biClrImportant = 0;
	}

	// ����ͼ�񡣷��� TRUE ��ʾ�ɹ���������ͼ�����ݣ����� FALSE ��ʾʧ�ܣ�ԭ��ͼ�����ݲ��ᱻ����
	inline BOOL Create(UINT uWidth, UINT uHeight, const PBYTE pbBits = NULL)
	{
		PBYTE pbTemp;

		pbTemp = (PBYTE) _HeapAlloc(_DibSize24(uWidth, uHeight));
		if (pbTemp)
		{
			Destroy();

			m_uWidth = uWidth;
			m_uHeight = uHeight;
			m_pbBits = pbTemp;

			if (pbBits)
			{
				CopyMemory(m_pbBits, pbBits, GetSize());
			}

			return TRUE;
		}

		return FALSE;
	}

	// ����ͼ�񡣷��� TRUE ��ʾ�ɹ���������ͼ�����ݣ����� FALSE ��ʾʧ�ܣ�ԭ��ͼ�����ݲ��ᱻ����
	inline BOOL Attach(UINT uWidth, UINT uHeight, PBYTE pbBits)
	{
		if (pbBits)
		{
			m_uWidth = uWidth;
			m_uHeight = uHeight;

			if (m_pbBits)
			{
				_HeapFree(m_pbBits);
			}

			m_pbBits = pbBits;

			return TRUE;
		}

		return FALSE;
	}

	// ����ͼ������
	inline VOID Destroy()
	{
		if (m_pbBits)
		{
			_HeapFree(m_pbBits);
			m_pbBits = NULL;
			m_uWidth = 0;
			m_uHeight = 0;
			m_dScale = 1;
		}
	}

	// ��ȡ����ͼ����
	inline UINT GetBackupWidth()
	{
		return m_uBackupWidth;
	}

	// ��ȡ����ͼ��߶�
	inline UINT GetBackupHeight()
	{
		return m_uBackupHeight;
	}

	// ��ȡ����ͼ������
	inline PBYTE GetBackupBits()
	{
		return m_pbBackupBits;
	}

	// ����ͼ������
	inline VOID Backup()
	{
		DestroyBackup();
		m_uBackupWidth = m_uWidth;
		m_uBackupHeight = m_uHeight;
		m_pbBackupBits = (PBYTE) _HeapAlloc(GetSize());
		CopyMemory(m_pbBackupBits, m_pbBits, GetSize());
	}

	// �ָ�ͼ������
	inline VOID Restore()
	{
		Attach(m_uBackupWidth, m_uBackupHeight, m_pbBackupBits);
		m_pbBackupBits = NULL;
	}

	// ���ٱ���ͼ������
	inline VOID DestroyBackup()
	{
		if (m_pbBackupBits)
		{
			_HeapFree(m_pbBackupBits);
			m_pbBackupBits = NULL;
			m_uBackupWidth = 0;
			m_uBackupHeight = 0;
		}
	}

	// �ñ������ݸ���ԭ���ݣ��������ݲ�ɾ��
	inline void CopyBackDataToCurData()
	{
		if (!m_pbBackupBits) return;
		memcpy(m_pbBits,m_pbBackupBits,GetSize());
	}

	// ��ȡ�����漰����ͼ��
	BOOL Load(PCTSTR ptzFileName);
	BOOL Save(PCTSTR ptzFileName, LPCRECT prtRect = NULL);
	VOID Draw(HDC hDC, INT iLeft = 0, INT iTop = 0);

	// ͼ��任
	VOID Stretch(UINT uWidth, UINT uHeight);
	VOID FlipHorizon();
	VOID FlipVertical();
	VOID Rotate(UINT uAngle, COLORREF crColor = 0x00FFFFFF);

	// ͼ����
	VOID Decolourize();
	VOID Expose(BYTE bValue = 128);
	VOID Emboss();
	VOID Adjust(INT iRed, INT iGreen, INT iBlue);

	// ��Ե���
	VOID Fringe(BYTE bValue = 64);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


