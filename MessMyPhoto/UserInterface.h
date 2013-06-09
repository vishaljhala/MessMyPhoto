#pragma once
class UserInterface
{
public:
	UserInterface(void);
	~UserInterface(void);
	BOOL LoadMyImage(WCHAR *szInpPath);

	HDC m_dc;
	HBITMAP m_bmp;
	long m_lWidth,m_lHeight;

	WCHAR m_szOrigImgPath[1024], m_szDestImgPath[1024];
	char *m_szOutImgPath;
	BOOL m_bImgLoaded;
	int m_iImgType;
	int m_iSmthLvl;
};

