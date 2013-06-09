#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void Painterly(char *szPath, float fSigma_blur_factor,int fThreshold, int iMaxStrokeLn, 
	int iMinStrokeLn, int iMaxBrushSz,int bPreviewMode, int*iErrCode);

struct PainterlyParam
{
public:
	float fSigma_blur_factor;
	// fGridSize;
	//float fC;
	float fThreshold; 			
	int iMaxStrokeLength;
	int iMinStrokeLength;
	int iMaxBrushSize;
	//int iColorOpacity;			
	//float fhJitter;
	//float fsJitter;
	//float fvJitter;		
	//float frJitter;
	//float fgJitter;
	//float fbJitter;		
	//BOOL bDrawEdges; 
	//float fEdgeThreshold;


};

class clsPainterly
{
	
private:
	IplImage *pImgSource, *pImgDest, *pImgTemp;
	int iNR, iNC;
	float ** ppfSobelX; float **ppfSobelY;
	PainterlyParam m_param;
	//IMAGE srcR,srcG,srcB;
	//IMAGE tmpR,tmpG,tmpB;
	//IMAGE destR,destG,destB;
	//IplImage *destImg,*tmpImg;
	//BOOL m_bDoDrawDot;
	//int iNC;
	//int iNR;

public:
	clsPainterly();
	~clsPainterly();
	int PainterlyMain(PainterlyParam param,IplImage **pImgSrc,GlobalHelper &g_GH);
	int IdentifyStrokePoints( float iBrushSize,PainterlyParam param,GlobalHelper &g_GH,int **ppiStrkLst,int* iCtr ); 
	void MakeStroke(float iBurshSize, int rowInitial, int colInitial,PainterlyParam param,GlobalHelper &g_GH);
	void createStrokeColor( int row, int col );
	float GetDifferenceClr( int row, int col ); 
	float GetDifference( int row, int col );
	float getGradientMagnitude( int row, int col );
	void getGradientDirection(  int row, int col, float *fGradDirX, float *fGradDirY );
	void RenderStroke( float iBrushSize, CvPoint *path,int iPathSize) ;
	void RenderEdges(GlobalHelper &g_GH);

	int Sobel(IMAGE im,float **ppfSobelX,float **ppfSobelY,GlobalHelper &g_GH);
	void CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter);
	float Gauss(int x, float fSigma);
	void Convolution(float*pfFilter/*, float **ppfGradX1, float **ppfGradY1*/, int **ppfGradXY1,IMAGE channel,int iFilterSize, float fSigma,int iNR,int iNC);

};
