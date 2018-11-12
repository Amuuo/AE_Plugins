#pragma once
#include<cstdio>
#include<cstdlib>
#include<ctime>
#include "AEConfig.h"
#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_EffectCBSuites.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "Smart_Utils.h"
#include "AEGP_SuiteHandler.h"
#include "AEFX_SuiteHelper.h"
#include "PixelStruct.h"
#include "SortStruct.h"



void swap(PF_Fixed*, PF_Fixed*);

template<class T>
void insertionSort(PF_Fixed, std::vector<T>*&);

template<class T>
void quickSort(PF_Fixed, std::vector<T>*&);