// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MESSENGINE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MESSENGINE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MESSENGINE_EXPORTS
#define MESSENGINE_API __declspec(dllexport)
#else
#define MESSENGINE_API __declspec(dllimport)
#endif

// This class is exported from the MessEngine.dll
class MESSENGINE_API CMessEngine {
public:
	CMessEngine(void);
	// TODO: add your methods here.
};

extern MESSENGINE_API int nMessEngine;

MESSENGINE_API int fnMessEngine(void);
