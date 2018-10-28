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
#include <iostream>



class PixelStruct {

public:

  PixelStruct();
  PixelStruct(PF_Pixel);


  void operator()(PF_Pixel);

  PF_Pixel pixel{};
  A_long value{};

};
