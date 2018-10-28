#pragma once
      
#ifndef PIXEL_SORTER_H
#define PIXEL_SORTER_H



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
#ifdef AE_OS_WIN
	#include <Windows.h>
#endif


#define MAX_RBG_VALUE  765.0f  

using namespace std;





enum {
// default input layer
  SORT_INPUT=0,
  MAIN_GROUP_START,
  SORT_METHOD_DROPDOWN,
  SORT_BY_DROPDOWN,
  SORT_BY_COLOR_DROPDOWN,
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
  
enum SortByColorOptions
{
  RED_SORT=1,
  GREEN_SORT,
  BLUE_SORT
};





struct PixelStruct{
  
  PixelStruct();
  PixelStruct(PF_Pixel&);


  PF_FpLong  pixelValue{};
  PF_Pixel*  avgValuePtr{};
  PF_Pixel   pixel{};
  
};






class Base { 

  public:

  virtual ~Base();
};

           
 



class ShiftInfo : public Base 
{

  public:
  
  ShiftInfo();
    //ShiftInfo(PF_Fixed num_of_params) : params{PF_ParamDef[num_of_params]}{}

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
  PF_ParamValue  sortByColorChoice;
  PF_FpLong      highRangeLimit;
  PF_FpLong      lowRangeLimit;

};




class PixelSorter : public Base 
{  
  public:

    PixelSorter();
    PixelSorter(ShiftInfo*, ParamValues);

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



    struct BeginItem 
    {
      BeginItem();
      BeginItem(vector<PixelStruct>::iterator iter,
                PF_Fixed x,
                PF_Fixed y);
          
      vector<PixelStruct>::iterator beginIter;
      PF_Fixed x, y;
    };

    struct EndItem 
    {
      EndItem();
      EndItem(vector<PixelStruct>::iterator iter,
              PF_Fixed x,
              PF_Fixed y);
          
      vector<PixelStruct>::iterator endIter;
      PF_Fixed x, y;
    };

    
    class SortSegment 
    {            

      public:

      SortSegment();

      struct 
      {
        struct 
        {
          PF_FpLong red{0};
          PF_FpLong green{0};
          PF_FpLong blue{0};
            
        } high_value;

        struct 
        {
          PF_FpLong red{0};
          PF_FpLong green{0};
          PF_FpLong blue{0};
            
        } low_value;        
          
      } rgb_sort;

      struct
      {
        PF_FpLong high_value{0};
        PF_FpLong low_value{0};
          
      } luminosity_sort;      



      PF_FpLong segmentLength{0};
      PF_Boolean isEmpty = true;
      vector<vector<PixelStruct>> replacementPixelsVecs{};
      vector<BeginItem> beginItems{};
      vector<EndItem> endItems{};

       void getRGBInterpolatedVectors();
       void reset();
        
    
    } current_segment;
    
    


     void storeBeginRowIters();  
     void storeEndRowIters();  
     void getSortLength();    
     void getLineWidthPixelAverage();
     void getLineWidthColorAverage();
    void sortPixelMap();
    void resetSortingVariables();
     void reverseSortIfTrue(PF_Boolean,PF_Fixed);
     bool pixelDistanceIsLongEnoughToSort();
     void sortPixelSegments();
     void getAndStorePixelValue();
     void getUserSetMinLength();


    
    function<bool(const PixelStruct&, const PixelStruct&)> sortFunc = 
      [](const PixelStruct& left , const PixelStruct& right)
      {
        return left.pixelValue < right.pixelValue;
      };

    function<bool(const PixelStruct&, const PixelStruct&)> sortFuncRed = 
      [](const PixelStruct& left , const PixelStruct& right)
      {
        return left.pixel.red<right.pixel.red;
      };
    function<bool(const PixelStruct&, const PixelStruct&)> sortFuncGreen = 
      [](const PixelStruct& left , const PixelStruct& right)
      {
        return left.pixel.green<right.pixel.green;
      };
    function<bool(const PixelStruct&, const PixelStruct&)> sortFuncBlue = 
      [](const PixelStruct& left , const PixelStruct& right)
      {
        return left.pixel.blue<right.pixel.blue;
      };

};

#endif // !PIXEL_SORTER_H