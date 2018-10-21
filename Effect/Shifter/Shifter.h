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
#include <map>
#include <deque>

using namespace std;
using namespace placeholders;

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
// default input layer
  SORT_INPUT=0,
  MAIN_GROUP_START,
  SORT_METHOD_DROPDOWN,
  SORT_BY_DROPDOWN,
  ORIENTAION_DROPDOWN,
  REVERSE_SORT_CHECKBOX,
  SORT_VALUE_RANGE,
  SORT_WIDTH_SLIDER,
  MAIN_GROUP_END,
  VARIABLE_SORT_GROUP_START,
  VARIABLE_SORT_CHECKBOX,
  VARIABLE_SLIDER,
  FAVOR_DARK_RANGES,
  VARIABLE_SORT_GROUP_END,
  MISC_GROUP_END,
  MANUAL_SORT_RANGE_GROUP_START,
  HIGH_RANGE_SORT_LIMIT,
  LOW_RANGE_SORT_LIMIT,
  MANUAL_SORT_RANGE_GROUP_END,
  MISC_GROUP_START,
  MIN_SORT_LENGTH_SLIDER,
  MIN_SORT_RAND_SLIDER,
  MIN_REVERSE_DIST_SLIDER,
  PIXEL_INTERPOLATION_CHECKBOX,
	SORT_NUM_PARAMS
};

enum {  
  BUTTON_ID=0,
  INPUT_ID,
  INPUT_ID2
};










class Base { 

  public:

  virtual ~Base(){} 
};






class ShiftInfo : public Base 
{

  public:
  
    ShiftInfo(){}

    PF_ParamDef    params[SORT_NUM_PARAMS];
    PF_EffectWorld originalCopyP{};
    PF_EffectWorld inputCopy{};
    PF_ProgPtr	   ref{};
    PF_SampPB  	   samp_pb{};
    PF_InData 	   in_data{};
    PF_Boolean	   no_opB{};
    PF_Boolean     mapCreated{0};

    vector<vector<PixelStruct>> pixelMap; 
    Base* pixelSorter;
};







struct ParamValues
{
  ParamValues();
  ParamValues(ShiftInfo*);
  
  
  PF_FpLong      minSortLength;
  PF_FpLong      minSortRandValue;
  PF_Fixed       sortWidth;
  PF_ParamValue  selectedVariableSort;
  PF_FpLong      variableValue;
  PF_FpLong      sortValueRange;
  PF_ParamValue  favorsDarkRanges{0};
  PF_FpLong      minReverseSortValue;
  PF_ParamValue  selectedReverseSort;
  PF_ParamValue  iterpolatePixelRanges;
  PF_ParamValue  sortMethodMenuChoice;
  PF_ParamValue  orientation;
  PF_ParamValue  sortByMenuChoice;
  PF_FpLong      highRangeLimit;
  PF_FpLong      lowRangeLimit;

};







struct PixelStruct{
  
  PixelStruct(){}
  PixelStruct(const PF_Pixel& pixel) : pixel{pixel}, 
                                       pixelValue{static_cast<PF_FpLong>(
                                         pixel.blue + 
                                         pixel.green+ 
                                         pixel.red)} {}


  PF_FpLong  pixelValue;
  PF_Pixel*  avgValuePtr;
  PF_Pixel   pixel;
  
};






class PixelSorter : public Base 
{  
  public:

    PixelSorter();
    PixelSorter(ShiftInfo*, ParamValues);

    void operator =(ShiftInfo& shiftInfo);
    ~PixelSorter();
    
    

    using highestPixelValueQueue = priority_queue<PF_FpLong,vector<PF_FpLong>,less<PF_FpLong>>;
    using lowestPixelValueQueue  = priority_queue<PF_FpLong,vector<PF_FpLong>,greater<PF_FpLong>>;
    using iteratorVector         = vector<vector<PixelStruct>::iterator>;
    
    
    random_device random{};
    ParamValues   param{};
    ShiftInfo*    shiftInfoCopy{};
  
    PF_FpLong   minSortLengthRand{};
    PF_FpLong   pixValueAverage{};
    PF_FpLong   pixAvg{};
    PF_FpLong   sortLength{};
    PF_FpLong   minLength{};  
    PF_Boolean  lengthIsShortEnoughForFlip{};
  
    PF_Fixed    pixelLines{};
    PF_Fixed    linePixels{};
    PF_Fixed    currentPixelValueDistance{};
    PF_FpLong   columnAvg{};
    PF_Fixed    lineCounter{};
    PF_Fixed    pixelCounter{};
    PF_Fixed    currPixDistance{};
    PF_FpLong   startingRGBValue{};



    
    class sortSegment 
    {            

      public:

        sortSegment(){}

        union 
        {   
          //UNION PART 1--------
          struct 
          {
            struct 
            {
              PF_FpLong red{};
              PF_FpLong green{};
              PF_FpLong blue{};
            
            } high_value;

            struct 
            {
              PF_FpLong red{};
              PF_FpLong green{};
              PF_FpLong blue{};
            
            } low_value;        
          
          } rgb_sort;
          //____________________


          //UNION PART 2--------  
          struct
          {
            PF_FpLong high_value{};
            PF_FpLong low_value{};
          
          } luminosity_sort;      
          //____________________
        };
      


        struct BeginItems 
        {
          BeginItems(iteratorVector& iter, 
                     PF_Fixed x, 
                     PF_Fixed y) : beginIters{iter}, x{x}, y{y}{}
          
          iteratorVector beginIters;
          PF_Fixed x, y;
        };

        struct EndItems 
        {
          EndItems(iteratorVector& iter, 
                     PF_Fixed x, 
                     PF_Fixed y) : endIters{iter}, x{x}, y{y}{}
          iteratorVector endIters;
          PF_Fixed x, y;
        };



        PF_FpLong segmentLength;
        PF_Boolean isEmpty = true;
        vector<vector<PixelStruct>> replacementPixelsVecs;
        vector<BeginItems> beginItems;
        vector<EndItems> endItems;

        inline void getRGBInterpolatedVectors();
        inline void reset();
        
    
    } current_segment;
    
    

    enum SortOrientations
    {
      VERTICAL_ORIENTATION=1,
      HORIZONTAL_ORIENTATION
    };

    enum SortMethods
    {
      USER_MAIN_SORT=1,
      USER_MANUAL_SORT,
      USER_RANGE_LOW
    };

    enum SortByMenuOptions
    {
      SORT_BY_LUMINOSITY=1,
      SORT_BY_RGB
    };
  

    

 
    inline void storeBeginRowIters();  
    inline void storeEndRowIters();  
    inline void getSortLength();    
    inline void getLineWidthPixelAverage();
    inline void getLineWidthColorAverage();
    inline void sortPixelMap();
    inline void resetSortingVariables();
    inline void reverseSortIfTrue(PF_Boolean,PF_Fixed);
    inline bool pixelDistanceIsLongEnoughToSort();
    inline void sortPixelSegments();
    inline void getAndStorePixelValue();
    inline void getUserSetMinLength();

    
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