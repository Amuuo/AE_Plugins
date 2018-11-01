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
#include "SortSegment.h"




#ifdef AE_OS_WIN
#include <Windows.h>
#endif

const int MAX_RBG_VALUE = 765;

using namespace std;


enum UserParameters { SORT_INPUT = 0, MAIN_GROUP_START, SORT_METHOD_DROPDOWN,
                      SORT_BY_DROPDOWN, SORT_BY_COLOR_DROPDOWN, ORIENTAION_DROPDOWN,
                      REVERSE_SORT_CHECKBOX, SORT_VALUE_RANGE, SORT_WIDTH_SLIDER,
                      MAIN_GROUP_END, VARIABLE_SORT_GROUP_START,VARIABLE_SORT_CHECKBOX,
                      VARIABLE_SLIDER, FAVOR_DARK_RANGES, VARIABLE_SORT_GROUP_END,
                      MISC_GROUP_END, MANUAL_SORT_RANGE_GROUP_START, HIGH_RANGE_SORT_LIMIT,
                      LOW_RANGE_SORT_LIMIT, MANUAL_SORT_RANGE_GROUP_END, MISC_GROUP_START,
                      MIN_SORT_LENGTH_SLIDER, MIN_SORT_RAND_SLIDER, MIN_REVERSE_DIST_SLIDER,
                      PIXEL_INTERPOLATION_CHECKBOX, SORT_NUM_PARAMS
};

enum SortOrientations { VERTICAL_ORIENTATION = 1, HORIZONTAL_ORIENTATION };
enum SortMethods { BASIC_SORT=1, MANUAL_SORT };
enum SortByMenuOptions { SORT_BY_LUMINOSITY = 1, SORT_BY_RGB };
enum SortByColorOptions { RED_SORT = 1, GREEN_SORT, BLUE_SORT };
enum PixelProperties { HUE=1, LUMINANCE, SATURATION, LIGHTNESS };
enum { SLIDER_ID, INPUT_ID};






class PixelSorter : public SortSegment {

public:

  using pixMap = vector<vector<PixelStruct>>;

  PixelSorter();
  ~PixelSorter() = default;

  random_device random{};

  PF_Fixed   minSortLengthRand;
  PF_Fixed   pixValueAverage;
  PF_Fixed   pixAvg;
  PF_Fixed   sortLength;
  PF_Fixed   minLength;
  PF_Fixed   lengthIsShortEnoughForFlip;
  PF_Fixed   pixelLines;
  PF_Fixed   linePixels;
  PF_Fixed   currentPixelValueDistance;
  PF_Fixed   lineCounter;
  PF_Fixed   pixelCounter;
  PF_Fixed   currPixDistance;
  PF_Fixed   startingRGBValue;
  PF_ParamDef params[SORT_NUM_PARAMS];
  
  PF_InData* in_data;
  pixMap pixelMap;


  void storeBeginRowIters();
  void storeEndRowIters();
  void getSortLength();
  void getLineWidthPixelAverage();
  void sortPixelMap();
  void resetSortingVariables();
  void reverseSortIfTrue(PF_Boolean, PF_Fixed);
  bool pixelDistanceIsLongEnoughToSort();
  void sortPixelSegments();
  void getAndStorePixelValue();
  void getUserSetMinLength();


  function<bool(const PixelStruct&, const PixelStruct&)> sortFunc =
    [](const PixelStruct& left, const PixelStruct& right) {
    return left.value < right.value;
  };


};
