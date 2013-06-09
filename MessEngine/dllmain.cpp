// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "GlobalHelper.h"
//#pragma comment(lib, "DelayImp.lib")


// Tell the linker that my DLL should be delay loaded
//#pragma comment(lib, "/DelayLoad:Dll.Dll")
//#pragma comment(linker, "/DELAYLOAD:opencv_imgproc220.dll")
//#pragma comment(lib, "opencv_core220.lib")
//#pragma comment(lib, "opencv_highgui220.lib")
//#pragma comment(lib, "opencv_imgproc220.lib")

// Tell the linker that I want to be able to unload my DLL
//#pragma comment(linker, "/Delay:unload")

//char g_pcszPath[] = "Z:\\home\\C339925\\messmyphoto\\imgTemplates\\"; 
char g_pcszPath[] = "D:\\home\\C339925\\messmyphoto.com\\imgTemplates\\"; 

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	break;
	case DLL_THREAD_ATTACH:
	break;
	case DLL_THREAD_DETACH:
	break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

