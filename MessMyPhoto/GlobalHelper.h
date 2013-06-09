#pragma once

class GlobalHelper
{
public:
	GlobalHelper(void);
	~GlobalHelper(void);

	void GetInputImageFile(WCHAR* szImage);
	void GetLastErrorWrapper();
	int ToMultiBytes( char* &pStr, const WCHAR* pwStr, int len, BOOL IsEnd);

	
};
