#include "GlobalHelper.h"
class clsInkify
{
private:
	IplImage *pImgSource, *pImgDest, *pImgTemp,*pImgGrey;
	int iNR, iNC;
	int m_iFilterSize;
	float m_fSigmaDist,  m_fSigmaColor;
	float ** ppfSobelX; float **ppfSobelY;


	//void CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter);
	//float Gauss(int x, float fSigma);
	//void Convolution(float*pfFilter, float **ppfGradXY1,int iFilterSize, float fSigma,GlobalHelper &g_GH);
	void Diff(GlobalHelper &g_GH);
	void ApplyCanvas(GlobalHelper &g_GH);
	void LocalMaxima(int iBrushSize,GlobalHelper &g_GH);
	//void Threshold(float **ppfGradXY1,GlobalHelper &g_GH);
	void MakeStroke(float iBurshSize, int rowInitial, int colInitial,GlobalHelper &g_GH);
	float GetDifferenceClr( int row, int col ); 
	float GetDifference( int row, int col );
	float getGradientMagnitude( int row, int col );
	void getGradientDirection(  int row, int col, float *fGradDirX, float *fGradDirY );
	int Sobel(float **ppfSobelX,float **ppfSobelY,GlobalHelper &g_GH);
	void BezierCurve(CvPoint pnt[4],int iCtr,GlobalHelper &g_GH);

public:
	clsInkify();
	~clsInkify();
	void InkifyMain(IplImage **pImgSrc,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	  int*iErrCode,GlobalHelper &g_GH);
};