
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define EXPORT __declspec(dllexport)

// Specify "C" linking to avoid C++ name mangling

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../2015/CLEM/CLEM/CoreFunction/GlobalVar.h"

	EXPORT void DLL_Init(CoreFunc_Global_var *, char*);
	EXPORT void RealTime_CL(double*, float *, unsigned short *, char*);
	EXPORT void DLL_Engage(int , char**, char*);
	EXPORT void DLL_Disengage(char*);
	EXPORT void SlowPeriodic_CL(double*, float *, unsigned short *, char*);

	CoreFunc_Global_var* Global_Var;			//pointer to all global variables in CLEM

#ifdef __cplusplus
}
#endif

