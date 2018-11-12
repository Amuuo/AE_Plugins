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




class PixelStruct {

public:

  PixelStruct();
  PixelStruct(PF_Pixel);
  ~PixelStruct() = default;


  void set(PF_Pixel);
  PF_Boolean operator <(const PF_Fixed&);
  //PF_Boolean operator <(const PixelStruct&);
  
  PF_Boolean operator >(const PF_Fixed&);
  //PF_Boolean operator >(const PixelStruct&);
  
  PF_Boolean operator ==(const PF_Fixed&);
  //PF_Boolean operator ==(const PixelStruct&);
  
  PF_Boolean operator <=(const PF_Fixed&);
  //PF_Boolean operator <=(const PixelStruct&);
  
  PF_Boolean operator >=(const PF_Fixed&);
  //PF_Boolean operator >=(const PixelStruct&);

  PF_Boolean operator !=(const PF_Fixed&);

  PF_Pixel pixel{};
  A_long value{};

};
