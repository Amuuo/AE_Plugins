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
#include <vector>


class SortStruct
{
public:
  SortStruct() {}
  PF_ProgPtr	 ref;
  PF_SampPB    samp_pb{};
  PF_Boolean	 no_opB{};
  PF_InData*   in_data;
  std::vector<std::vector<PixelStruct>*>* pixArray;
  //PixelStruct  pixArray**;
  PF_Boolean   mapCreated = false;
};

