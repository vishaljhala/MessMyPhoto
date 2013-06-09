#include "stdafx.h"
#include "CartoonFilter.h"

#define PI 3.1415926535

extern "C" __declspec(dllexport) void BlacknWhite(char *szPath,int iSigmaColor, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	clsBlacknWhite cf;
	cf.FilterMain(&pImgSource,iSigmaColor,iErrCode,g_GH);
		
	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);


	*iErrCode = 0;
}

clsBlacknWhite::clsBlacknWhite()
{
}
clsBlacknWhite::~clsBlacknWhite()
{
}

void clsBlacknWhite::FilterMain(IplImage **pImgSrc, int iSigmaColor,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		*iErrCode = 1;
		return;
	}

	iNR = pImgSource->height;
	iNC = pImgSource->width;
	fSigmaColor = iSigmaColor;
	imgLab = g_GH.cvCreateImageProxy(cvSize(iNC, iNR),IPL_DEPTH_8U,3);
	if(!imgLab)
	{
		*iErrCode = 1;
		return;
	}
	g_GH.cvSmoothProxy(pImgSource,imgLab,CV_BILATERAL,19,0,30,29);

	g_GH.cvCvtColorProxy(imgLab,pImgSource,CV_BGR2Lab);
	g_GH.cvCopyProxy(pImgSource,imgLab,0);

	ppiCluster = g_GH.IntHeap2D(iNR*iNC,15);
	ppiRegion = g_GH.IntHeap2D(iNR,iNC);
	ppiFlag = g_GH.IntHeap2D(iNR,iNC);
	ppiList = g_GH.IntHeap2D(2,iNR*iNC);
	int iTmp;
	for(int i=0;i<iNR;i++)
		for(int j=0;j<iNC;j++)
		{
			iTmp = i*iNC+j;
			ppiCluster[iTmp][0] = 0;
			ppiCluster[iTmp][4] = ppiCluster[iTmp][5] = ppiCluster[iTmp][6] = ppiCluster[iTmp][7] = ppiCluster[iTmp][8]
			= ppiCluster[iTmp][9] = ppiCluster[iTmp][10] = ppiCluster[iTmp][11] = ppiCluster[iTmp][12] = ppiCluster[iTmp][13]
			= ppiCluster[iTmp][14] = -1;
		}
		
	SegmentRegion(g_GH);
	MeanClusterColor(g_GH);
	//g_GH.cvCvtColorProxy(imgLab,pImgSource,CV_Lab2BGR);
	
	/*char szTmpMsg[1024];
	for(int i=0;i<iRgnCnt;i++)
	{
		sprintf(szTmpMsg,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",i,ppiCluster[i][0],ppiCluster[i][5],ppiCluster[i][5]
		,ppiCluster[i][7],ppiCluster[i][8],ppiCluster[i][9],ppiCluster[i][10],ppiCluster[i][11],ppiCluster[i][12]
		,ppiCluster[i][13],ppiCluster[i][14]);
		g_GH.DebugOut(szTmpMsg,FALSE);
	}*/
	
	bFirstCall = TRUE;
	recAssignBnW(iBiggestRgn);
	for(int m=0;m<iRgnCnt;m++)
		if(ppiCluster[m][4]==-1)
		{	recAssignBnW(m);}

	AssignBnW(g_GH);

	*pImgSrc = pImgSource;

	//g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&imgLab);

	free(ppiCluster[0]);free(ppiCluster);
	free(ppiList[0]); free(ppiList);
	free(ppiFlag[0]); free(ppiFlag);
	free(ppiRegion[0]); free(ppiRegion);
	return;
}
void clsBlacknWhite::MergeSmallRegions()
{
	if(iNR*iNC/50000<=ppiCluster[iRgnCnt][0])
		return;

	int iBigRgn=-1;
	int iBigRgnSz = -1;
	for(int j=5;j<15;j++)
	{
		if (ppiCluster[iRgnCnt][j] != -1 && ppiCluster[ppiCluster[iRgnCnt][j]][0]>iBigRgnSz)
		{
			iBigRgnSz = ppiCluster[ppiCluster[iRgnCnt][j]][0];
			iBigRgn = ppiCluster[iRgnCnt][j];
		}
	}
	if(iBigRgn>=0)
	{
		for (int i=0;i<iEndPos;i++)
		{
			ppiRegion[ppiList[0][i]][ppiList[1][i]] = iBigRgn;
		}
		ppiCluster[iBigRgn][0] += ppiCluster[iRgnCnt][0];

	}
	ppiCluster[iRgnCnt][0] = 0;
	ppiCluster[iRgnCnt][4] = ppiCluster[iRgnCnt][5] = ppiCluster[iRgnCnt][6] = ppiCluster[iRgnCnt][7] = ppiCluster[iRgnCnt][8]
	= ppiCluster[iRgnCnt][9] = ppiCluster[iRgnCnt][10] = ppiCluster[iRgnCnt][11] = ppiCluster[iRgnCnt][12] = ppiCluster[iRgnCnt][13]
	= ppiCluster[iRgnCnt][14] = -1;

	iRgnCnt--;

}
void clsBlacknWhite::recAssignBnW(int iRgnNbr)
{
	int iBlack=0, iWhite=0;

	if(bFirstCall)
	{
		ppiCluster[iRgnNbr][4] = 0;
		bFirstCall = FALSE;
	}
	else
	{
		for(int j=5;j<15;j++)
		{
			if(ppiCluster[iRgnNbr][j] != -1 && ppiCluster[ppiCluster[iRgnNbr][j]][4] == 255)
				iWhite += ppiCluster[ppiCluster[iRgnNbr][j]][0];
			if(ppiCluster[iRgnNbr][j] != -1 &&  ppiCluster[ppiCluster[iRgnNbr][j]][4] == 0)
				iBlack += ppiCluster[ppiCluster[iRgnNbr][j]][0];
		}
		if(iBlack>iWhite)
			ppiCluster[iRgnNbr][4] = 255;
		else
			ppiCluster[iRgnNbr][4] = 0;
	}

	for(int j=5;j<15;j++)
	{
		if(ppiCluster[iRgnNbr][j] != -1)
		{
			return recAssignBnW(ppiCluster[iRgnNbr][j]);
		}
	}
}
void clsBlacknWhite::AssignBnW(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*imgLab->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = ppiCluster[ppiRegion[i][j]][4];
		}
	}

}
void clsBlacknWhite::MeanClusterColor(GlobalHelper &g_GH)
{
	int iL,iU,iV;
	int iWStep, iJStep;

	for(int i=0;i<iNR;i++)
	{
		iWStep = i*imgLab->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(imgLab->imageData + iWStep + iJStep + 2) = ppiCluster[ppiRegion[i][j]][3];///ppiCluster[ppiGraph[i][j]][0];
			*(uchar*)(imgLab->imageData + iWStep + iJStep + 1) = ppiCluster[ppiRegion[i][j]][2];///ppiCluster[ppiGraph[i][j]][0];
			*(uchar*)(imgLab->imageData + iWStep + iJStep + 0) = ppiCluster[ppiRegion[i][j]][1];///ppiCluster[ppiGraph[i][j]][0];
		}
	}

}
void clsBlacknWhite::SegmentRegion(GlobalHelper &g_GH)
{

	
	iRgnCnt = 0;
	int iMax;
	int iEnergyLevel;
	//int iStartPos, iEndPos,

	for(int i=0;i<iNR;i++)
		for(int j=0;j<iNC;j++)
			ppiRegion[i][j] = -1;

	iBiggestRgn = iBiggestRgnSz = -1;
	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppiRegion[i][j]<0)
			{
				//Clean up of arrays
				for(int k=0;k<iNR;k++)
					memset(ppiFlag[k],0,sizeof(int));
				//for(int k=0;k<256;k++)
					//iArr[k] = 0;
				memset(ppiList[0],0,sizeof(int)); memset(ppiList[1],0,sizeof(int));
				
				
				iStartPos = iEndPos = 0;
				ppiList[0][iStartPos]  = i;ppiList[1][iStartPos]  = j; 
				iEndPos++;
				
				//iEnergyLevel = val->data[i][j];

				while(iStartPos< iEndPos)
					recFindMe(g_GH);

				FindAdjecentRegion();
				if(ppiCluster[iRgnCnt][0]>iBiggestRgnSz)
				{
					iBiggestRgnSz = ppiCluster[iRgnCnt][0];
					iBiggestRgn = iRgnCnt;
				}
				MergeSmallRegions();
				//iMax = GetMaxSat();
				
				/*for(int k=0;k<iEndPos;k++)
				{
					if(abs(channel->data[ppiList[0][k]][ppiList[1][k]]-iMax)>16)
					{
						if(channel->data[ppiList[0][k]][ppiList[1][k]]>iMax)
							channel->data[ppiList[0][k]][ppiList[1][k]] -=16;
						else
							channel->data[ppiList[0][k]][ppiList[1][k]] +=16;
					}
					else
						channel->data[ppiList[0][k]][ppiList[1][k]] = iMax;
				}*/
				
				iRgnCnt++;
			}
		}
	}

}
void clsBlacknWhite::FindAdjecentRegion()
{

	int iNbrRgn;
	for(int i=0;i<iEndPos;i++)
	{
		if(ppiList[0][i]-1>=0)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]-1][ppiList[1][i]];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[0][i]-1>=0 && ppiList[1][i]-1>=0)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]-1][ppiList[1][i]-1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[0][i]-1>=0 && ppiList[1][i]+1<iNC)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]-1][ppiList[1][i]+1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[0][i]+1<iNR && ppiList[1][i]+1<iNC)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]+1][ppiList[1][i]+1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[0][i]+1<iNR)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]+1][ppiList[1][i]];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[0][i]+1<iNR && ppiList[1][i]-1>=0)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]+1][ppiList[1][i]-1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[1][i]-1>=0)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]][ppiList[1][i]-1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
		if(ppiList[1][i]+1<iNC)
		{	
			iNbrRgn = ppiRegion[ppiList[0][i]][ppiList[1][i]+1];
			
			if(iNbrRgn != -1 && iNbrRgn != iRgnCnt)
				AssignNeighbour(iNbrRgn);
		}
	}
	int messs=0;
}
void clsBlacknWhite::AssignNeighbour(int iNbrRgn)
{
	int iSmallest;
	BOOL bFlag;
	

	
	iSmallest = iNC*iNR;
	bFlag = FALSE;
	for(int j=5;j<15;j++)
	{
		if (ppiCluster[iRgnCnt][j] == -1)
		{
			ppiCluster[iRgnCnt][j] = iNbrRgn;
			bFlag = TRUE;
			break;
		}
		if(ppiCluster[iRgnCnt][j] != -1 && ppiCluster[ppiCluster[iRgnCnt][j]][0]< iSmallest )
		{
			iSmallest =  ppiCluster[ppiCluster[iRgnCnt][j]][0];
		}
	}
	if(!bFlag && ppiCluster[iNbrRgn][0]>iSmallest)
	{
		for(int j=5;j<15;j++)
		{
			if (ppiCluster[ppiCluster[iRgnCnt][j]][0]<ppiCluster[iNbrRgn][0])
				ppiCluster[iRgnCnt][j]= iNbrRgn;
		}
	}

}
void clsBlacknWhite::recFindMe(GlobalHelper &g_GH)
{
	int i = ppiList[0][iStartPos],j=ppiList[1][iStartPos];


	ppiRegion[i][j] = iRgnCnt;
	(iStartPos)++;
	//iArr[channel->data[i][j]]++;
	if(ppiCluster[iRgnCnt][0] ==0)
	{
		ppiCluster[iRgnCnt][1] = g_GH.GetByte(imgLab,i,j,0);
		ppiCluster[iRgnCnt][2] = g_GH.GetByte(imgLab,i,j,1);
		ppiCluster[iRgnCnt][3] = g_GH.GetByte(imgLab,i,j,2);
	}
	else
	{
		//ppiCluster[iRgnCnt][1] = ((float)ppiCluster[iRgnCnt][1]+g_GH.GetByte(imgLab,i,j,0))/2;
		//ppiCluster[iRgnCnt][2] = ((float)ppiCluster[iRgnCnt][2]+g_GH.GetByte(imgLab,i,j,1))/2;
		//ppiCluster[iRgnCnt][3] = ((float)ppiCluster[iRgnCnt][3]+g_GH.GetByte(imgLab,i,j,2))/2;
	}
	ppiCluster[iRgnCnt][0]++;


	if(i-1>=0)
	{
		if(j-1>=0)
			if(ppiRegion[i-1][j-1]<0 && !ppiFlag[i-1][j-1]	)
			{
				
				if(abs(g_GH.GetByte(imgLab,i-1,j-1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j-1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j-1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i-1;
					ppiList[1][iEndPos] = j-1;
					iEndPos++;
				}
			}
		if(j-1>=0)
			ppiFlag[i-1][j-1] = 1;

		if(ppiRegion[i-1][j]<0 && !ppiFlag[i-1][j])
		{
				if(abs(g_GH.GetByte(imgLab,i-1,j,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i-1;
					ppiList[1][iEndPos] = j;
					iEndPos++;
				}
		}
		ppiFlag[i-1][j]=1;

		if(j+1<iNC)
			if(ppiRegion[i-1][j+1]<0 && !ppiFlag[i-1][j+1])
			{
				if(abs(g_GH.GetByte(imgLab,i-1,j+1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j+1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i-1,j+1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i-1;
					ppiList[1][iEndPos] = j+1;
					iEndPos++;
				}
		
			}
			if(j+1<iNC)
			ppiFlag[i-1][j+1] = 1;

	}
	if(i+1<iNR)
	{
		if(j-1>=0)
			if(ppiRegion[i+1][j-1]<0 && !ppiFlag[i+1][j-1])
			{
				if(abs(g_GH.GetByte(imgLab,i+1,j-1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j-1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j-1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i+1;
					ppiList[1][iEndPos] = j-1;
					iEndPos++;
				}
			}
		if(j-1>=0)
			ppiFlag[i+1][j-1]=1;

		if(ppiRegion[i+1][j]<0 && !ppiFlag[i+1][j])
		{
				if(abs(g_GH.GetByte(imgLab,i+1,j,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i+1;
					ppiList[1][iEndPos] = j;
					iEndPos++;
				}
		}
		ppiFlag[i+1][j]=1;

		if(j+1<iNC)
			if(ppiRegion[i+1][j+1]<0 && !ppiFlag[i+1][j+1])
			{
				if(abs(g_GH.GetByte(imgLab,i+1,j+1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j+1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
				abs(g_GH.GetByte(imgLab,i+1,j+1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
				{
					ppiList[0][iEndPos] = i+1;
					ppiList[1][iEndPos] = j+1;
					iEndPos++;
				}
			}
		if(j+1<iNC)
			ppiFlag[i+1][j+1]=1;
	}
	if(j+1<iNC && ppiRegion[i][j+1]<0 && !ppiFlag[i][j+1])
	{
			if(abs(g_GH.GetByte(imgLab,i,j+1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
			abs(g_GH.GetByte(imgLab,i,j+1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
			abs(g_GH.GetByte(imgLab,i,j+1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
			{
				ppiList[0][iEndPos] = i;
				ppiList[1][iEndPos] = j+1;
				iEndPos++;
			}
	}
	if(j+1<iNC)
		ppiFlag[i][j+1]=1;

	if(j-1>=0 && ppiRegion[i][j-1]<0 && !ppiFlag[i][j-1])
	{
			if(abs(g_GH.GetByte(imgLab,i,j-1,0)-ppiCluster[iRgnCnt][1]) <= fSigmaColor &&
			abs(g_GH.GetByte(imgLab,i,j-1,1)-ppiCluster[iRgnCnt][2]) <= fSigmaColor &&
			abs(g_GH.GetByte(imgLab,i,j-1,2)-ppiCluster[iRgnCnt][3]) <= fSigmaColor)
			{
				ppiList[0][iEndPos] = i;
				ppiList[1][iEndPos] = j-1;
				iEndPos++;
			}
	}
	if(j-1>=0)
		ppiFlag[i][j-1]=1;
	
}

void clsBlacknWhite::QuantizeColor()
{
	int iL,iU,iV;
	int iWStep, iJStep;

	for(int i=0;i<iNR;i++)
	{
		iWStep = i*imgLab->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			iV = *(uchar*)(imgLab->imageData + iWStep + iJStep + 2);	
			iU = *(uchar*)(imgLab->imageData + iWStep + iJStep + 1);	
			iL = *(uchar*)(imgLab->imageData + iWStep + iJStep + 0);

			*(uchar*)(imgLab->imageData + iWStep + iJStep + 2) = iV - (iV%16-8);
			*(uchar*)(imgLab->imageData + iWStep + iJStep + 1) = iU - (iU%16-8);
			*(uchar*)(imgLab->imageData + iWStep + iJStep + 0) = iL - (iL%16-8);
		}
	}
}
/*void clsBlacknWhite::MergeCluster(int iSrc, int jSrc, int iDest,int jDest)
{
	ppiGraph[iSrc][jSrc] = ppiGraph[iDest][jDest];
	if(iSrc-1>=0 && ppiGraph[iSrc-1][jSrc] == ppiGraph[iDest][jDest])
		MergeCluster(iSrc-1,jSrc,iDest,jDest);
	if(jSrc-1>=0  && ppiGraph[iSrc][jSrc-1] == ppiGraph[iDest][jDest])
		MergeCluster(iSrc,jSrc-1,iDest,jDest);
}
void clsBlacknWhite::Clusterize(GlobalHelper &g_GH)
{
	iTotClusters = 0;

	int iL,iU,iV;
	int iLx,iUx,iVx;

	//float **fDist = g_GH.FloatHeap2D(4,2);
	float fL,fT;

	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			fT=fL= 32000;
			//fDist[0][1] = 0; fDist[1][1] = 1; fDist[2][1] = 2; fDist[3][1] = 3;
			
			iV = g_GH.GetByte(imgLab,i,j,2);
			iU = g_GH.GetByte(imgLab,i,j,1);	
			iL = g_GH.GetByte(imgLab,i,j,0);

			if(i==0 && j==0)
			{
				AddCluster(i,j,iL,iU,iV);
				continue;
			}
			if(i-1>=0)
			{
				iVx = ppiCluster[ppiGraph[i-1][j]][3];//g_GH.GetByte(imgLab,i-1,j,2);
				iUx = ppiCluster[ppiGraph[i-1][j]][2];//g_GH.GetByte(imgLab,i-1,j,1);	
				iLx = ppiCluster[ppiGraph[i-1][j]][1];//g_GH.GetByte(imgLab,i-1,j,0);
				fT = sqrt( (double)(iV-iVx)*(iV-iVx) + (iU-iUx)*(iU-iUx) +(iL-iLx)*(iL-iLx) );
			}
			/*if(i-1>=0 &&j-1>=0)
			{
				iVx = ppiCluster[ppiGraph[i-1][j-1]][3];//g_GH.GetByte(imgLab,i-1,j,2);
				iUx = ppiCluster[ppiGraph[i-1][j-1]][2];//g_GH.GetByte(imgLab,i-1,j,1);	
				iLx = ppiCluster[ppiGraph[i-1][j-1]][1];//g_GH.GetByte(imgLab,i-1,j,0);
//				iVx = g_GH.GetByte(imgLab,i-1,j-1,2);
	//			iUx = g_GH.GetByte(imgLab,i-1,j-1,1);	
		//		iLx = g_GH.GetByte(imgLab,i-1,j-1,0);
				fDist[1][0] = sqrt( (double)(iV-iVx)*(iV-iVx) + (iU-iUx)*(iU-iUx) +(iL-iLx)*(iL-iLx) );
			}
			if(i-1>=0 &&j+1<iNC)
			{
				iVx = ppiCluster[ppiGraph[i-1][j+1]][3];//g_GH.GetByte(imgLab,i-1,j,2);
				iUx = ppiCluster[ppiGraph[i-1][j+1]][2];//g_GH.GetByte(imgLab,i-1,j,1);	
				iLx = ppiCluster[ppiGraph[i-1][j+1]][1];//g_GH.GetByte(imgLab,i-1,j,0);
				//iVx = g_GH.GetByte(imgLab,i-1,j+1,2);
				//iUx = g_GH.GetByte(imgLab,i-1,j+1,1);	
				//iLx = g_GH.GetByte(imgLab,i-1,j+1,0);
				fDist[3][0] = sqrt( (double)(iV-iVx)*(iV-iVx) + (iU-iUx)*(iU-iUx) +(iL-iLx)*(iL-iLx) );
			}*//*
			if(j-1>=0)
			{
				iVx = ppiCluster[ppiGraph[i][j-1]][3];//g_GH.GetByte(imgLab,i-1,j,2);
				iUx = ppiCluster[ppiGraph[i][j-1]][2];//g_GH.GetByte(imgLab,i-1,j,1);	
				iLx = ppiCluster[ppiGraph[i][j-1]][1];//g_GH.GetByte(imgLab,i-1,j,0);
				//iVx = g_GH.GetByte(imgLab,i,j-1,2);
				//iUx = g_GH.GetByte(imgLab,i,j-1,1);	
				//iLx = g_GH.GetByte(imgLab,i,j-1,0);
				fL = sqrt( (double)(iV-iVx)*(iV-iVx) + (iU-iUx)*(iU-iUx) +(iL-iLx)*(iL-iLx) );
			}
			//Sort((float**)fDist);
			if(fL<=fSigmaColor && fT>fSigmaColor)
			{
				UpdateCluster(i,j,i,j-1,iL,iU,iV);
			}
			else if(fL>fSigmaColor && fT<=fSigmaColor)
			{
				UpdateCluster(i,j,i-1,j,iL,iU,iV);
			}
			else if(fL<=fSigmaColor && fT<=fSigmaColor)
			{
				int iTmpCluster = ppiGraph[i-1][j];
				UpdateCluster(i,j,i,j-1,iL,iU,iV);
				
				MergeCluster(i-1,j,i,j);
				
				ppiCluster[ppiGraph[i][j]][0] += ppiCluster[iTmpCluster][0];

				ppiCluster[ppiGraph[i][j]][1] = ((float)ppiCluster[ppiGraph[i][j]][1]+ppiCluster[iTmpCluster][1])/2;
				ppiCluster[ppiGraph[i][j]][2] = ((float)ppiCluster[ppiGraph[i][j]][2]+ppiCluster[iTmpCluster][2])/2;
				ppiCluster[ppiGraph[i][j]][3] = ((float)ppiCluster[ppiGraph[i][j]][3]+ppiCluster[iTmpCluster][3])/2;

			}
			else
			{
				AddCluster(i,j,iL,iU,iV);
			}
			/*if(fDist[0][0]<=fSigmaColor)
			{
				if(fDist[0][1]==0)
					UpdateCluster(i,j,i,j-1,iL,iU,iV);
				else if(fDist[0][1]==1)
					UpdateCluster(i,j,i-1,j-1,iL,iU,iV);
				else if(fDist[0][1]==2)
					UpdateCluster(i,j,i-1,j,iL,iU,iV);
				else if(fDist[0][1]==3)
					UpdateCluster(i,j,i-1,j+1,iL,iU,iV);
			}
			else if(fDist[1][0]<=fSigmaColor)
			{
				if(fDist[1][1]==0)
					UpdateCluster(i,j,i,j-1,iL,iU,iV);
				else if(fDist[1][1]==1)
					UpdateCluster(i,j,i-1,j-1,iL,iU,iV);
				else if(fDist[1][1]==2)
					UpdateCluster(i,j,i-1,j,iL,iU,iV);
				else if(fDist[1][1]==3)
					UpdateCluster(i,j,i-1,j+1,iL,iU,iV);
			}
			else if(fDist[2][0]<=fSigmaColor)
			{
				if(fDist[2][1]==0)
					UpdateCluster(i,j,i,j-1,iL,iU,iV);
				else if(fDist[2][1]==1)
					UpdateCluster(i,j,i-1,j-1,iL,iU,iV);
				else if(fDist[2][1]==2)
					UpdateCluster(i,j,i-1,j,iL,iU,iV);
				else if(fDist[2][1]==3)
					UpdateCluster(i,j,i-1,j+1,iL,iU,iV);
			}
			else if(fDist[3][0]<=fSigmaColor)
			{
				if(fDist[3][1]==0)
					UpdateCluster(i,j,i,j-1,iL,iU,iV);
				else if(fDist[3][1]==1)
					UpdateCluster(i,j,i-1,j-1,iL,iU,iV);
				else if(fDist[3][1]==2)
					UpdateCluster(i,j,i-1,j,iL,iU,iV);
				else if(fDist[3][1]==3)
					UpdateCluster(i,j,i-1,j+1,iL,iU,iV);
			}
			else
			{
				AddCluster(i,j,iL,iU,iV);
			}*//*

		}
	}
	//free(fDist[0]);free(fDist);
}*//*
void clsBlacknWhite::Sort(float **fDist)
{
	float fTemp1, fTemp2;
	for(int i=0;i<4;i++)
	{
		for(int j=i+1;j<4;j++)
		{
			if(fDist[i][0]>fDist[j][0])
			{
				fTemp1 = fDist[i][0];
				fTemp2 = fDist[i][1];
				fDist[i][0] = fDist[j][0];
				fDist[i][1] = fDist[j][1];
				fDist[j][0] = fTemp1;
				fDist[j][1] = fTemp2;
			}
		}
	}
}
void clsBlacknWhite::AddCluster(int i, int j,int iL,int iU, int iV)
{
	ppiGraph[i][j] = iTotClusters;
	ppiCluster[iTotClusters][0] += 1;

	ppiCluster[iTotClusters][1] = iL;
	ppiCluster[iTotClusters][2] = iU;
	ppiCluster[iTotClusters][3] = iV;

	iTotClusters++;

}
void clsBlacknWhite::UpdateCluster(int i, int j, int iMatch,int jMatch,int iL, int iU, int iV)
{

	ppiGraph[i][j] = ppiGraph[iMatch][jMatch];
	ppiCluster[ppiGraph[i][j]][0] += 1;

	ppiCluster[ppiGraph[i][j]][1] = ((float)ppiCluster[ppiGraph[i][j]][1]+iL)/2;
	ppiCluster[ppiGraph[i][j]][2] = ((float)ppiCluster[ppiGraph[i][j]][2]+iU)/2;
	ppiCluster[ppiGraph[i][j]][3] = ((float)ppiCluster[ppiGraph[i][j]][3]+iV)/2;
}
*/
