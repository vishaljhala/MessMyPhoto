// MessEngine.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MessEngine.h"


// This is an example of an exported variable
MESSENGINE_API int nMessEngine=0;

// This is an example of an exported function.
MESSENGINE_API int fnMessEngine(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see MessEngine.h for the class definition
CMessEngine::CMessEngine()
{
	return;
}
