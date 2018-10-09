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

enum {
	SORT_INPUT = 0,	// default input layer 
  SORT_BUTTON,
  SORT_VALUE_RANGE,
  MIN_SORT_LENGTH_SLIDER,
  MIN_SORT_RAND_SLIDER,
  SORT_WIDTH_SLIDER,
  VARIABLE_SORT_CHECKBOX,
  VARIABLE_SLIDER,
  MIN_REVERSE_DIST_SLIDER,
  REVERSE_SORT_CHECKBOX,
  SWITCH_ORIENTAION_CHECKBOX,
	SORT_NUM_PARAMS
};

enum {
  BUTTON_ID=1,
  INPUT_ID
};






struct PixelStruct{
  
  PixelStruct(){}
  PixelStruct(const PF_Pixel& pixel) : 
    pixel{pixel}, 
    pixelValue{pixel.blue + pixel.green+ pixel.red} {}

  int      pixelValue;
  PF_Pixel pixel;
};



class ShiftInfo{

  public:

  PF_ParamDef    sortValueRange; 
  PF_ParamDef    minSortSlider; 
  PF_ParamDef    minSortRandomSlider;
  PF_ParamDef    shiftSortButton;
  PF_ParamDef    sortWidthSlider;
  PF_ParamDef    variableSortCheckbox;
  PF_ParamDef    variableSlider;
  PF_ParamDef    minReverseSortSlider;
  PF_ParamDef    reverseSortCheckbox;
  PF_ParamDef    switchOrientationCheckbox;

  PF_EffectWorld originalCopy;
  PF_EffectWorld inputCopy;
	PF_ProgPtr	   ref;
	PF_SampPB  	   samp_pb;
	PF_InData 	   in_data;
	PF_Boolean	   no_opB;
  bool           mapCreated{false};

  vector<vector<PixelStruct>> pixelMap; 
  



  class PixelSorter
  {  
    public:

    PixelSorter(ShiftInfo*);
    ~PixelSorter();

  
    using highestPixelValueQueue = priority_queue<int, vector<int>, std::less<int>>;
    using lowestPixelValueQueue  = priority_queue<int, vector<int>, std::greater<int>>;
    using iteratorVector         = vector<vector<PixelStruct>::iterator>;

    PF_ParamValue  userSelectedReverseSort;
    PF_ParamValue  userSelectedVariableSort;
    ShiftInfo*     shiftInfoCopy;
    random_device  random;
  
    float  minSortLengthRand{0};
    float  pixValueAverage{0};
    float  pixAvg{0};
    float  variableValue;
    bool   lengthIsShortEnoughForFlip{false};
    int    sortLength;
    int    minSortLength;
    int    sortValueRange;
    int    sortWidth;
    int    userMinReverseSortValue;
    int    minSortRandValue;
    int    layerWidth;
    int    layerHeight;
    int    pixelLines;
    int    linePixels;
    int    currentPixelValueDistance{0};
    int    columnAvg;
    int    lineCounter{0};
    int    pixelCounter{0};
    int    currPixDistance{0};
    int    userMinLength;
    bool   verticalOrientation;

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
    inline void reverseSortIfTrue(bool,int);
    inline bool pixelDistanceIsLongEnoughToSort();
    inline void sortPixelSegments();
    inline void getAndStorePixelValue();
    inline void getUserSetMinLength();
    
    function<bool(const PixelStruct&, const PixelStruct&)> sortFunc = 
      [](const PixelStruct& left , const PixelStruct& right)
      {
        return left.pixelValue < right.pixelValue;
      };

    
  
  }* pixelSorter;


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