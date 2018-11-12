#pragma once
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


class SortStruct
{
public:
  SortStruct() {}
  PF_InData* in_data;
  PixelStruct* pixArray{nullptr};
  PF_Boolean mapCreated = false;
};

