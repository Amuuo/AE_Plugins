#include "PixelSorter.h"


PixelSorter::PixelSorter()
{
}



PixelSorter::PixelSorter(ShiftInfo* shiftInfo, ParamValues param)  :     
    param         {param},    
    shiftInfoCopy {shiftInfo}
{
    pixelLines = param.orientation == 1 ? 
      shiftInfo->in_data.width : shiftInfo->in_data.height;
    
    linePixels = param.orientation == 1 ? 
      shiftInfo->in_data.height : shiftInfo->in_data.width;
}


ParamValues::ParamValues(){}

ParamValues::ParamValues(ShiftInfo* shiftInfo) :
  minSortLength         {shiftInfo->params[MIN_SORT_LENGTH_SLIDER].u.fs_d.value}, 
  sortValueRange        {shiftInfo->params[SORT_VALUE_RANGE].u.fs_d.value},
  sortWidth             {shiftInfo->params[SORT_WIDTH_SLIDER].u.fd.value},
  minReverseSortValue   {shiftInfo->params[MIN_REVERSE_DIST_SLIDER].u.fs_d.value},
  minSortRandValue      {shiftInfo->params[MIN_SORT_RAND_SLIDER].u.fs_d.value},
  highRangeLimit        {shiftInfo->params[HIGH_RANGE_SORT_LIMIT].u.fs_d.value},
  lowRangeLimit         {shiftInfo->params[LOW_RANGE_SORT_LIMIT].u.fs_d.value},     
  variableValue         {shiftInfo->params[VARIABLE_SLIDER].u.fs_d.value},
  selectedReverseSort   {shiftInfo->params[REVERSE_SORT_CHECKBOX].u.bd.value},
  iterpolatePixelRanges {shiftInfo->params[PIXEL_INTERPOLATION_CHECKBOX].u.bd.value},
  favorsDarkRanges      {shiftInfo->params[FAVOR_DARK_RANGES].u.bd.value},
  selectedVariableSort  {shiftInfo->params[VARIABLE_SORT_CHECKBOX].u.bd.value},
  sortMethodMenuChoice  {shiftInfo->params[SORT_METHOD_DROPDOWN].u.pd.value},
  orientation           {shiftInfo->params[ORIENTAION_DROPDOWN].u.pd.value},
  sortByMenuChoice      {shiftInfo->params[SORT_BY_DROPDOWN].u.pd.value},
  sortByColorChoice     {shiftInfo->params[SORT_BY_COLOR_DROPDOWN].u.pd.value}
  
{

}

Base::~Base(){}

ShiftInfo::ShiftInfo(){}


PixelStruct::PixelStruct(){}
PixelStruct::PixelStruct(PF_Pixel& pixel) : pixel{pixel}, 
                                 pixelValue{                                  
                                 static_cast<PF_FpLong>(pixel.blue) + 
                                 static_cast<PF_FpLong>(pixel.green)+ 
                                 static_cast<PF_FpLong>(pixel.red)} {}

PixelSorter::~PixelSorter(){}




                                                                 

inline void PixelSorter::
storeBeginRowIters()
{ 
  for (PF_Fixed j=lineCounter; j<(lineCounter+param.sortWidth)&&j<pixelLines-1; ++j)
  {        
    current_segment.beginItems.push_back(
      {shiftInfoCopy->pixelMap[j].begin()+pixelCounter, j, pixelCounter});
  }
}






inline void PixelSorter::
storeEndRowIters()
{  
  for (PF_Fixed j = lineCounter; 
       j < (lineCounter + param.sortWidth) && j < pixelLines - 1; ++j) 
  { 
    current_segment.endItems.push_back(
      {shiftInfoCopy->pixelMap[j].begin()+pixelCounter, j, pixelCounter});
  }  
}





inline void PixelSorter::
getSortLength()
{
  sortLength = param.sortValueRange;

  if (param.selectedVariableSort)
  { 
    if(param.favorsDarkRanges)
    {
      sortLength = (sortLength/4) * 
        pow((abs(current_segment.luminosity_sort.low_value-MAX_RBG_VALUE)/MAX_RBG_VALUE+1), 
            2+(param.variableValue-1));
    }
    else
    {
      sortLength = (sortLength/4) * 
        pow((current_segment.luminosity_sort.high_value/MAX_RBG_VALUE+1), 
            2+(param.variableValue-1));
    }
  }
}






inline void PixelSorter::
getLineWidthPixelAverage()
{
  columnAvg = 0;
  auto columnWidthSpan = lineCounter + param.sortWidth;
  

  for (auto i=lineCounter; i<columnWidthSpan && i<pixelLines-1; ++i) 
  {    
    columnAvg += shiftInfoCopy->pixelMap[i][pixelCounter].pixelValue;
  }
  columnAvg /= param.sortWidth;


  for(auto i=lineCounter; i<columnWidthSpan && i<pixelLines-1; ++i) 
  {
    shiftInfoCopy->pixelMap[i][pixelCounter].pixel.alpha = 255;
  }  
   

  if (currPixDistance != 0 ) { startingRGBValue = columnAvg; } 
 
}




inline void PixelSorter::
getLineWidthColorAverage()
{
  auto columnWidthSpan=lineCounter+param.sortWidth;

  PF_FpLong r_avg{0};
  PF_FpLong g_avg{0};
  PF_FpLong b_avg{0};



  for (auto i = lineCounter; i<columnWidthSpan && i<pixelLines-1; ++i)
  {
    r_avg += shiftInfoCopy->pixelMap[i][pixelCounter].pixel.red;
    g_avg += shiftInfoCopy->pixelMap[i][pixelCounter].pixel.green;
    b_avg += shiftInfoCopy->pixelMap[i][pixelCounter].pixel.blue;
  }
  r_avg /= param.sortWidth;
  g_avg /= param.sortWidth;
  b_avg /= param.sortWidth;

  


  if (r_avg>current_segment.rgb_sort.high_value.red)  
    current_segment.rgb_sort.high_value.red = r_avg;
  
  if (g_avg>current_segment.rgb_sort.high_value.green)  
    current_segment.rgb_sort.high_value.green = g_avg;
  
  if (b_avg>current_segment.rgb_sort.high_value.blue)
    current_segment.rgb_sort.high_value.blue = b_avg;


  if (r_avg<current_segment.rgb_sort.low_value.red || 
      current_segment.rgb_sort.low_value.red==0)
    current_segment.rgb_sort.low_value.red = r_avg;
  
  if (g_avg<current_segment.rgb_sort.low_value.green || 
      current_segment.rgb_sort.low_value.green==0)
    current_segment.rgb_sort.low_value.green = g_avg;
  
  if (b_avg<current_segment.rgb_sort.low_value.blue || 
      current_segment.rgb_sort.low_value.blue==0)
    current_segment.rgb_sort.low_value.blue = b_avg;

  
}





void PixelSorter::
sortPixelMap()
{
  

  for (lineCounter=0; lineCounter<pixelLines; lineCounter+=param.sortWidth) 
  { 
    resetSortingVariables();
    storeBeginRowIters();
    for (pixelCounter=0; pixelCounter<linePixels; ++pixelCounter, ++currPixDistance) 
    {   
      if (current_segment.isEmpty) 
      {
        resetSortingVariables();
        storeBeginRowIters();                
        getAndStorePixelValue();  
        current_segment.isEmpty = false;
      }
      if (pixelDistanceIsLongEnoughToSort()) 
      { 
        current_segment.segmentLength=currPixDistance;
        storeEndRowIters();
        sortPixelSegments();                    
        resetSortingVariables();
        continue;
      } 
      else
      {
        getAndStorePixelValue();     
      }
    }           
  }
  resetSortingVariables();
}





inline void PixelSorter::resetSortingVariables()
{

  current_segment.reset();
  startingRGBValue = 0;
  currPixDistance = 0;  
  pixValueAverage = 0;
  columnAvg = 0;
  lengthIsShortEnoughForFlip = false;
  minLength = 0;
  currentPixelValueDistance = 0;
  
}






inline void PixelSorter::reverseSortIfTrue(PF_Boolean needToReverse, PF_Fixed index)
{
  if (needToReverse)
    reverse(current_segment.beginItems[index].beginIter, 
            current_segment.endItems[index].endIter);
}





inline bool PixelSorter::pixelDistanceIsLongEnoughToSort()
{
  getSortLength();
  getUserSetMinLength();

  switch (param.sortMethodMenuChoice)
  {
    case USER_MANUAL_SORT:
  
      if(current_segment.luminosity_sort.high_value>=param.highRangeLimit && 
          current_segment.luminosity_sort.high_value<=param.lowRangeLimit)
      {
        if (currentPixelValueDistance>=sortLength || pixelCounter==linePixels-1)
        {
          return true;  
        }
      }
      break;

    case USER_MAIN_SORT:
      
      if (((currentPixelValueDistance >=sortLength) && 
            (currPixDistance>minLength)) ||
             (pixelCounter==linePixels-1))
      {
        lengthIsShortEnoughForFlip = 
          currPixDistance < param.minReverseSortValue ? true:false; 
        return true;
      }
      break;

    default: break;
         
  }
  return false;
}







inline void PixelSorter::sortPixelSegments()
{ 
  switch (param.sortByMenuChoice)
  {
    case SORT_BY_LUMINOSITY:
  
      for (auto h = 0; h < current_segment.beginItems.size(); ++h)
      {
        sort(current_segment.beginItems[h].beginIter, 
             current_segment.endItems[h].endIter, sortFunc);        
        reverseSortIfTrue(param.selectedReverseSort||lengthIsShortEnoughForFlip,h);
               
        switch (param.sortByColorChoice)
        {
          
        
          case RED_SORT:
          {
            PF_FpLong rgbBeginVal=current_segment.beginItems[h].beginIter->pixel.red;
            PF_FpLong rgbEndVal=current_segment.endItems[h].endIter->pixel.red;
            PF_FpLong rgbStep=(rgbBeginVal-rgbEndVal)/currPixDistance;

            for (auto pix=current_segment.beginItems[h].beginIter; 
                  pix!=current_segment.endItems[h].endIter; ++pix)
            {
              rgbBeginVal-=rgbStep;
              pix->pixel.red=static_cast<A_u_char>(rgbBeginVal);
            }
           
            break;
          }
          
          
          case GREEN_SORT:
          {
            PF_FpLong rgbBeginVal=current_segment.beginItems[h].beginIter->pixel.green;
            PF_FpLong rgbEndVal=current_segment.endItems[h].endIter->pixel.green;
            PF_FpLong rgbStep=(rgbBeginVal-rgbEndVal)/currPixDistance;

            for (auto pix=current_segment.beginItems[h].beginIter; 
                  pix!=current_segment.endItems[h].endIter; ++pix)
            {
              rgbBeginVal-=rgbStep;
              pix->pixel.green=static_cast<A_u_char>(rgbBeginVal);
            }
           
            break;
          }
          
          
          case BLUE_SORT:
          {
            PF_FpLong rgbBeginVal=current_segment.beginItems[h].beginIter->pixel.blue;
            PF_FpLong rgbEndVal=current_segment.endItems[h].endIter->pixel.blue;
            PF_FpLong rgbStep=(rgbBeginVal-rgbEndVal)/currPixDistance;

            for (auto pix=current_segment.beginItems[h].beginIter; 
                  pix!=current_segment.endItems[h].endIter; ++pix)
            {
              rgbBeginVal-=rgbStep;
              pix->pixel.blue=static_cast<A_u_char>(rgbBeginVal);
            }
           
            break;
          }
          default:break;          
        }
      }
      break;
      
    case SORT_BY_RGB:

      for (auto line = 0, pixel=0; line<current_segment.beginItems.size(); ++line)
      {
        if (param.iterpolatePixelRanges)
        {
          for (auto k=current_segment.beginItems[line].beginIter;
               k!=current_segment.endItems[line].endIter; ++k,++pixel)
          {
            k->pixel=current_segment.replacementPixelsVecs[line][pixel].pixel;
          }
          pixel=0;
        }
        else
        {          
          switch (param.sortByColorChoice)
          {
            case RED_SORT:
              sort(current_segment.beginItems[line].beginIter,
                   current_segment.endItems[line].endIter,sortFuncRed);
              break;
          
            case GREEN_SORT:
              sort(current_segment.beginItems[line].beginIter,
                   current_segment.endItems[line].endIter,sortFuncGreen);
              break;
            case BLUE_SORT:
              sort(current_segment.beginItems[line].beginIter,
                 current_segment.endItems[line].endIter,sortFuncBlue);
              break;
          
            default:break;
          }
          
        }
      }
      break;

    default: break;
  }
}






inline void PixelSorter::getAndStorePixelValue()
{
  
  switch (param.sortByMenuChoice)
  {  
    case SORT_BY_LUMINOSITY:
      getLineWidthPixelAverage();
      break;
    
    case SORT_BY_RGB:
      getLineWidthPixelAverage();
      getLineWidthColorAverage();
      break;

    default: break;
  }
  
  
  PF_FpLong* mostValue = &current_segment.luminosity_sort.high_value;
  PF_FpLong* leastValue = &current_segment.luminosity_sort.low_value;


  *mostValue  = *mostValue<columnAvg?columnAvg:*mostValue;
  *leastValue = *leastValue>columnAvg?columnAvg:*leastValue;        
    
  currentPixelValueDistance = static_cast<PF_Fixed>(*mostValue - *leastValue);
}






inline void PixelSorter::getUserSetMinLength()
{
  minLength = param.minSortRandValue;
  if (param.minSortRandValue >= 2) 
  {
    minLength += param.minSortLength; 
    
    minLength += 
      (random() % (int)(param.minSortRandValue/2)) - param.minSortRandValue;
    
    minLength *= (MAX_RBG_VALUE-columnAvg)/MAX_RBG_VALUE + 1;
  }
}






PixelSorter::SortSegment::SortSegment()
{
}

inline void PixelSorter::SortSegment::getRGBInterpolatedVectors()
{

  PF_FpLong red_range = rgb_sort.high_value.red - rgb_sort.low_value.red;  
  PF_FpLong red_interpolation_slope = red_range/segmentLength;
  
  PF_FpLong green_range = rgb_sort.high_value.green - rgb_sort.low_value.green;  
  PF_FpLong green_interpolation_slope = green_range/segmentLength;
  
  PF_FpLong blue_range = rgb_sort.high_value.blue - rgb_sort.low_value.blue;  
  PF_FpLong blue_interpolation_slope = blue_range/segmentLength;

  
  PF_FpLong red_start   = rgb_sort.low_value.red;
  PF_FpLong green_start = rgb_sort.low_value.green;
  PF_FpLong blue_start  = rgb_sort.low_value.blue;
  
  for (int i=0; i<beginItems.size(); ++i)
  {

      replacementPixelsVecs[beginItems[i].x][beginItems[i].y] = PF_Pixel{255,
                                             static_cast<A_u_char>(red_start),
                                             static_cast<A_u_char>(green_start),
                                             static_cast<A_u_char>(blue_start)};

      red_start=(red_start+red_interpolation_slope<=255)?
        (red_start+=red_interpolation_slope):red_start;

      green_start=(green_start+green_interpolation_slope<=255)?
        (green_start+=green_interpolation_slope):green_start;

      blue_start=(blue_start+blue_interpolation_slope<=255)?
        (blue_start+=blue_interpolation_slope):blue_start;
    }
}



inline void PixelSorter::SortSegment::reset()
{
  *this = SortSegment{};
}

PixelSorter::BeginItem::BeginItem()
{
}

PixelSorter::BeginItem::BeginItem(vector<PixelStruct>::iterator iter,
                                  PF_Fixed x,
                                  PF_Fixed y) : x{ x }, y{ y }, beginIter{ iter }{}

PixelSorter::EndItem::EndItem()
{
}

PixelSorter::EndItem::EndItem(vector<PixelStruct>::iterator iter,
                              PF_Fixed x,
                              PF_Fixed y) : x{ x }, y{ y }, endIter{ iter }{}
