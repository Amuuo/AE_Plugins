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
#include <vector>
#include <random>
#include <utility>
#include <algorithm>
#include <array>
#include <queue>
#include <functional>
#include <map>
#include <cmath>
#include <array>
#include <fstream>
#include "PixelStruct.h"




using namespace std;



class SortSegment {


public:
  using iteratorPair =
    vector<pair<vector<PixelStruct>::iterator, vector<PixelStruct>::iterator>>;

  SortSegment();
  ~SortSegment() = default;
  


  PixelStruct  highValue;
  PixelStruct  lowValue;
  PF_FpLong    segmentLength;
  PF_Boolean   isEmpty = true;
  A_long       columnAvg;
  
  iteratorPair borderIters{
    pair<vector<PixelStruct>::iterator,vector<PixelStruct>::iterator>({},{})};


  virtual void getRGBInterpolatedVectors();
  void reset();

};
