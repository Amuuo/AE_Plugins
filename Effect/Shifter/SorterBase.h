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
#include <vector>
#include "PixelSorter.h"
#include "SortSegment.h"
#include <assert.h>





class SorterBase : public PixelSorter {

public:

  SorterBase(){}
  ~SorterBase() = default;

  PF_ProgPtr	   ref;
  PF_SampPB  	   samp_pb{};
  PF_Boolean	   no_opB{};
  PF_Boolean     mapCreated = false;

  void init(PF_InData*);

};
