#pragma once
using namespace cv;
struct image 
{
public:
		struct header *info;            /* Pointer to header */
		unsigned char **data;           /* Pixel values */
		~image()
		{
			if(data)
			{
				free(data[0]);
				free(data);
			}
		}
};

typedef struct image * IMAGE;
typedef void (*cvResizePtr)(const CvArr* src, CvArr* dst, int interpolation);
typedef IplImage* (*cvCreateImagePtr)(CvSize size, int depth, int channels);
typedef CvSize (*cvGetSizePtr)(const CvArr* arr);
typedef void (*cvReleaseImagePtr)(IplImage** image);
typedef IplImage* (*cvLoadImagePtr)(const char* filename, int iscolor);
typedef int (*cvSaveImagePtr)(const char* filename, const CvArr* image);
typedef void (*cvCvtColorPtr)( const CvArr* src, CvArr* dst, int code );
typedef CvScalar (*cvGet2DPtr)( const CvArr* arr, int idx0, int idx1 );
typedef void (*cvSet2DPtr)( CvArr* arr, int idx0, int idx1, CvScalar value );
typedef void (*cvSetPtr)( CvArr* arr, CvScalar value, const CvArr* mask );
typedef void (*cvLinePtr)( CvArr* img, CvPoint pt1, CvPoint pt2,
                     CvScalar color, int thickness,
                     int line_type, int shift );
typedef void (*cvSmoothPtr)( const CvArr* src, CvArr* dst,
                      int smoothtype,
                      int size1 ,
                      int size2 ,
                      double sigma1,
                      double sigma2);
typedef void (*cvCopyPtr)(const CvArr* src, CvArr* dst,const CvArr* mask  );
typedef void (*cvTransposePtr)( const CvArr* src, CvArr* dst   );
typedef void (*cvFlipPtr)( const CvArr* src, CvArr* dst,int flip_mode   );
typedef void (*cvDilatePtr)(const CvArr* src, CvArr* dst,IplConvKernel* element ,int iterations);
typedef void (*cvErodePtr)( const CvArr* src, CvArr* dst,IplConvKernel* element ,int iterations);
typedef Mat (*cvGetRotationMatrix2DPtr)( Point2f center, double angle, double scale );
typedef void (*cvWarpAffinePtr)( InputArray src, OutputArray dst,
                            InputArray M, Size dsize,
                            int flags,
                            int borderMode,
                            const Scalar& borderValue);
typedef void (*cvEllipsePtr)( CvArr* img, CvPoint center, CvSize axes,
                        double angle, double start_angle, double end_angle,
                        CvScalar color, int thickness ,
                        int line_type , int shift);
//=INTER_LINEAR
	//=BORDER_CONSTANT
	//=Scalar()
//typedef cvPolyLine

class GlobalHelper
{
public:
	GlobalHelper();
	~GlobalHelper(void);
	void Init(char *szFileName,int *iErrCode);

	// VC+++ Support functions
	void ReportStatus(HWND hDlg,BOOL bStepIT);
	void GetLastErrorWrapper();
	int WcharToChar( char* &pStr, const WCHAR* pwStr, int len, BOOL IsEnd);
	int CharToWchar( const char* pStr, WCHAR* &pwStr);
	void DebugOut(char* szMsg);
	//void OpenFileDialogWrapper(WCHAR* szImage);
	UCHAR GetByte(IplImage *img, int i, int j, int iChannel);
	void SetByte(IplImage *img, int i, int j,int iChannel,UCHAR byte);
	int GlobalHelper::GetHue(int iRGB);
	int getTimeDiff(SYSTEMTIME *st1, SYSTEMTIME *st2);

	HINSTANCE m_hOpenCVCore;
	HINSTANCE m_hOpenCVImgProc;
	HINSTANCE m_hOpenCVHighGui;
	HINSTANCE m_hTBB;
	//Load Image
	IMAGE load_image (char *name,BOOL bLoadPreview);
	int load_RGB (char *name,IplImage**ppImg,BOOL bLoadPreview);

	//Save Image
	int save_image (IMAGE x, char *name);
	int save_RGB (char *name,IplImage *pImg);

	struct image* newimage (int nr, int nc);
	float ** FloatHeap2D (int nr, int nc);
	int ** IntHeap2D (int nr, int nc);
	IplImage *toOpenCV (IMAGE x);
	IMAGE fromOpenCV (IplImage *x);
	int load_RGB (IMAGE *r, IMAGE *g, IMAGE *b, char *name);

	IplImage* convertImageRGBtoHSV(const IplImage *imageRGB);
	IplImage* convertImageHSVtoRGB(const IplImage *imageHSV);

	int get_RGB (IMAGE *r, IMAGE *g, IMAGE *b, IplImage *image);
	int get_HSV (IMAGE *r, IMAGE *g, IMAGE *b, IplImage *image);
	IplImage* set_RGB (IMAGE r, IMAGE g, IMAGE b);
	IplImage* set_HSV (IMAGE r, IMAGE g, IMAGE b);
	void BlendScreen(IplImage *pBottom,IplImage *pTop, int iPos);
	//void ScaleImage(IMAGE channel,float **ppfArrX,float **ppfArrY,float min,float max);
	//void ScaleImage(IMAGE channel,float **ppfArrX);

	cvResizePtr cvResizeProxy;
	cvCreateImagePtr cvCreateImageProxy;
	cvGetSizePtr cvGetSizeProxy;
	cvReleaseImagePtr cvReleaseImageProxy;
	cvLoadImagePtr cvLoadImageProxy;
	cvSaveImagePtr cvSaveImageProxy;
	cvCvtColorPtr cvCvtColorProxy;
	cvGet2DPtr cvGet2DProxy;
	cvSet2DPtr cvSet2DProxy;
	cvSetPtr cvSetProxy;
	cvLinePtr cvLineProxy;
	cvSmoothPtr cvSmoothProxy;
	cvCopyPtr cvCopyProxy;
	cvTransposePtr cvTransposeProxy;
	cvFlipPtr cvFlipProxy;
	cvDilatePtr cvDilateProxy;
	cvErodePtr cvErodeProxy;
	cvGetRotationMatrix2DPtr cvGetRotationMatrix2DProxy;
	cvWarpAffinePtr cvWarpAffineProxy;
	cvEllipsePtr cvEllipseProxy;

private:
	HANDLE fDebug;
	
};

struct header 
{
	int nr, nc;             /* Rows and columns in the image */
	int oi, oj;             /* Origin */
};
