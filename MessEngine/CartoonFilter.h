#include "GlobalHelper.h"
class clsBlacknWhite
{
private:
	int iNC,iNR;
	IplImage *pImgSource;
	int **ppiRegion,**ppiList,**ppiFlag,**ppiCluster;
	IplImage * imgLab;
	int iRgnCnt;
	int fSigmaColor;
	int iStartPos,iEndPos;
	int iBiggestRgn,iBiggestRgnSz;
	BOOL bFirstCall;

public:
	clsBlacknWhite();
	~clsBlacknWhite();

	void FilterMain(IplImage **pImgSrc,int iSigmaColor, int*iErrCode,GlobalHelper &g_GH);
	
	void SegmentRegion(GlobalHelper &g_GH);
	void recFindMe(GlobalHelper &g_GH);
	void FindAdjecentRegion();
	void AssignNeighbour(int iNbrRgn);
	void recAssignBnW(int iRgnNbr);
	void MergeSmallRegions();
	void QuantizeColor();
	void AssignBnW(GlobalHelper &g_GH);
	//void Clusterize(GlobalHelper &g_GH);
	//void CartoonColors();
	void MeanClusterColor(GlobalHelper &g_GH);
	//void Sort(float **fDist);
	//void AddCluster(int i, int j,int iL,int iU, int iV);
	//void UpdateCluster(int i, int j, int iNew, int jNew, int iL, int iU, int iV);
	//void MergeCluster(int iSrc, int jSrc, int iDest,int jDest);

};
