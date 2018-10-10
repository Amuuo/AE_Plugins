/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/


#pragma once

#ifndef SHIFTER_H
#define SHIFTER_H

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
#include <queue>
#include <functional>

using namespace std;

#ifdef AE_OS_WIN
	#include <Windows.h>
#endif

#define	NAME "Shifter"
#define DESCRIPTION	"Blend in a shifted copy of the image.\nCopyright 1994-2007 Adobe Systems Incorporated."

#define	MAJOR_VERSION		2
#define	MINOR_VERSION		0
#define	BUG_VERSION			0
#define	STAGE_VERSION		PF_Stage_DEVELOP
#define	BUILD_VERSION		1
#define MAX_RBG_VALUE  765.0f

enum {
	SORT_INPUT = 0,	// default input layer 
  SORT_BUTTON,
  SORT_VALUE_RANGE,
  MIN_SORT_LENGTH_SLIDER,
  MIN_SORT_RAND_SLIDER,
  SORT_WIDTH_SLIDER,
  VARIABLE_SORT_CHECKBOX,
  VARIABLE_SLIDER,
  FAVOR_DARK_RANGES,
  MIN_REVERSE_DIST_SLIDER,
  REVERSE_SORT_CHECKBOX,
  PIXEL_INTERPOLATION_CHECKBOX,
  ORIENTAION_DROPDOWN,
  HIGH_RANGE_SORT_LIMIT,
  LOW_RANGE_SORT_LIMIT,
  SORT_METHOD_DROPDOWN,
	SORT_NUM_PARAMS
};

enum {
  BUTTON_ID=1,
  INPUT_ID,
  INPUT_ID2
};






struct PixelStruct{
  
  PixelStruct(){}
  PixelStruct(const PF_Pixel& pixel) : 
    pixel{pixel}, 
    pixelValue{static_cast<PF_FpLong>(pixel.blue + pixel.green+ pixel.red)} {}

  PF_FpLong pixelValue;
  PF_Pixel  pixel;
};




class Base { 

  public:

  virtual ~Base(){} 
};




class ShiftInfo : public Base 
{

  public:

  ShiftInfo(){};

  PF_ParamDef    sortValueRange; 
  PF_ParamDef    minSortSlider; 
  PF_ParamDef    minSortRandomSlider;
  PF_ParamDef    shiftSortButton;
  PF_ParamDef    sortWidthSlider;
  PF_ParamDef    variableSortCheckbox;
  PF_ParamDef    variableSlider;
  PF_ParamDef    favorDarkRangesCheckbox;
  PF_ParamDef    minReverseSortSlider;
  PF_ParamDef    reverseSortCheckbox;
  PF_ParamDef    orientationDropdown;
  PF_ParamDef    iterpolatePixelCheckbox;
  PF_ParamDef    highRangeSortLimit;
  PF_ParamDef    lowRangeSortLimit;
  PF_ParamDef    sortMethodDropbox;

  PF_EffectWorld originalCopy;
  PF_EffectWorld inputCopy;
	PF_ProgPtr	   ref;
	PF_SampPB  	   samp_pb;
	PF_InData 	   in_data;
	PF_Boolean	   no_opB;
  PF_Boolean     mapCreated{0};

  vector<vector<PixelStruct>> pixelMap; 
  Base* pixelSorter;





};

class PixelSorter : public Base 
{  
  public:

  PixelSorter();
  PixelSorter(ShiftInfo*);
  PixelSorter(ShiftInfo& shiftInfo);
  void operator =(ShiftInfo& shiftInfo);
  ~PixelSorter();

  enum SortOrientations
  {
    VERTICAL_ORIENTATION=1,
    HORIZONTAL_ORIENTATION
  };

  enum SortMethods
  {
    SORT_BY_VARIABLE_RANGE=1,
    USER_RANGE
  };
  
  using highestPixelValueQueue = priority_queue<PF_Fixed,vector<PF_Fixed>,less<PF_Fixed>>;
  using lowestPixelValueQueue  = priority_queue<PF_Fixed,vector<PF_Fixed>,greater<PF_Fixed>>;
  using iteratorVector         = vector<vector<PixelStruct>::iterator>;

  PF_ParamValue  userSelectedReverseSort;
  PF_ParamValue  userSelectedVariableSort;
  ShiftInfo*     shiftInfoCopy;
  random_device  random;
  
  PF_FpLong  minSortLengthRand{0};
  PF_FpLong  pixValueAverage{0};
  PF_FpLong  pixAvg{0};
  PF_FpLong  variableValue;
  PF_FpLong  sortLength;
    
  PF_Boolean  lengthIsShortEnoughForFlip{0};
  PF_Boolean  userFavorsDarkRanges{0};
    
  PF_Fixed    minSortLength;
  PF_Fixed    sortValueRange;
  PF_Fixed    sortWidth;
  PF_Fixed    userMinReverseSortValue;
  PF_Fixed    minSortRandValue;
  PF_Fixed    layerWidth;
  PF_Fixed    layerHeight;
  PF_Fixed    pixelLines;
  PF_Fixed    linePixels;
  PF_Fixed    currentPixelValueDistance{0};
  PF_Fixed    columnAvg;
  PF_Fixed    lineCounter{0};
  PF_Fixed    pixelCounter{0};
  PF_Fixed    currPixDistance{0};
  PF_Fixed    userMinLength;
  PF_Fixed    verticalOrientation;
  PF_Fixed    highRangeLimit;
  PF_Fixed    lowRangeLimit;
  PF_Fixed    sortMethodMenuChoice;
  PF_Boolean  iterpolatePixelRanges;

  highestPixelValueQueue mostQueue{};
  lowestPixelValueQueue  leastQueue{};
  iteratorVector         rowBeginIters;
  iteratorVector         rowEndIters;

  inline void storeRowIters(iteratorVector&);  
  inline void storeBeginRowIters();  
  inline void storeEndRowIters();  
  inline void getSortLength();    
  inline void getLineWidthPixelAverage();
  inline void sortPixelMap();
  inline void resetSortingVariables();
  inline void reverseSortIfTrue(PF_Boolean,PF_Fixed);
  inline bool pixelDistanceIsLongEnoughToSort();
  inline void sortPixelSegments();
  inline void getAndStorePixelValue();
  inline void getUserSetMinLength();
  inline void getInterpolationValues();
    
  function<bool(const PixelStruct&, const PixelStruct&)> sortFunc = 
    [](const PixelStruct& left , const PixelStruct& right)
    {
      return left.pixelValue < right.pixelValue;
    };

};

















AEGP_PluginID pluginID;

extern "C" {

DllExport	PF_Err 
EntryPointFunc (
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extraP);

}

#endif // SHIFTER_H