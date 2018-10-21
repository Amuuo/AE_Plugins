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

/*
  Shifter.cpp
  
  Purpose:   Demonstrates sub-pixel sampling and use of the 
        Iteration suites.
*/

#include "Shifter.h"


static SPBasicSuite *S_pica_suiteP = NULL;




static PF_Err 
About (PF_InData   *in_data,
       PF_OutData  *out_data,
       PF_ParamDef *params[],
       PF_LayerDef *output )
{
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  
  suites.ANSICallbacksSuite1()->sprintf(  
     out_data->return_msg, 
     "%s, v%d.%d\r%s",
     NAME, 
     MAJOR_VERSION, 
     MINOR_VERSION, 
     DESCRIPTION);

  return PF_Err_NONE;
}


static PF_Err 
GlobalSetup (PF_InData   *in_data,
             PF_OutData  *out_data,
             PF_ParamDef *params[],
             PF_LayerDef *output )
{
  
  AEGP_SuiteHandler  suites(in_data->pica_basicP); 
  suites.UtilitySuite6()->AEGP_RegisterWithAEGP(NULL, "Shifter", &pluginID);

  out_data->my_version = PF_VERSION(
    MAJOR_VERSION, 
    MINOR_VERSION,
    BUG_VERSION, 
    STAGE_VERSION, 
    BUILD_VERSION);


  out_data->out_flags =  PF_OutFlag_USE_OUTPUT_EXTENT |
                         PF_OutFlag_REFRESH_UI | 
                         PF_OutFlag_FORCE_RERENDER |
                         PF_OutFlag_I_USE_SHUTTER_ANGLE |
                         PF_OutFlag_WIDE_TIME_INPUT;

  out_data->out_flags2 = PF_OutFlag2_SUPPORTS_SMART_RENDER |
                         PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS | 
                         PF_OutFlag2_DOESNT_NEED_EMPTY_PIXELS | 
                         PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT;

  


  return PF_Err_NONE;
}





static PF_Err 
ParamsSetup (PF_InData    *in_data,
             PF_OutData   *out_data,
             PF_ParamDef  *params[],
             PF_LayerDef  *output)
{
  PF_Err      err = PF_Err_NONE;
  PF_ParamDef def;
  
  PF_ADD_TOPIC("Main", MAIN_GROUP_START);
  PF_ADD_POPUP("Sort Method", 2, 0, "Basic Sort|Manual Sort", SORT_METHOD_DROPDOWN);
  PF_ADD_POPUP("Sort By:", 2, 0, "Luminosity|Individual RGB", SORT_BY_DROPDOWN);
  PF_ADD_POPUP("Sort Orientation", 2, 0, "Vertical|Horizontal", ORIENTAION_DROPDOWN);  
  PF_ADD_CHECKBOX("Invert Sort", "Enabled", 0, NULL, REVERSE_SORT_CHECKBOX); 
  PF_ADD_FLOAT_SLIDER("Sort Value Range", 1, 765, 1, 765, 0, 100, PF_Precision_INTEGER, NULL, NULL, SORT_VALUE_RANGE);  
  PF_ADD_SLIDER("Sort Width", 1, 200, 1, 200, 5, SORT_WIDTH_SLIDER);  
  PF_END_TOPIC(MAIN_GROUP_END);

  PF_ADD_TOPIC("Variable Sort", VARIABLE_SORT_GROUP_START);  
  PF_ADD_CHECKBOX("Variable Sort", "Enabled", 0, NULL, VARIABLE_SORT_CHECKBOX);    
  PF_ADD_FLOAT_SLIDER("Variable Range", 0, 3, 0, 3, NULL, 1, PF_Precision_HUNDREDTHS, NULL, NULL, VARIABLE_SLIDER);    
  PF_ADD_CHECKBOX("Favor Dark Ranges", "Enabled", 0, NULL, FAVOR_DARK_RANGES); 
  PF_END_TOPIC(VARIABLE_SORT_GROUP_END); 
  
  PF_ADD_TOPIC("Manual Sort Range", MANUAL_SORT_RANGE_GROUP_START); 
  PF_ADD_FLOAT_SLIDER("High Range Sort Limit", 1, 765, 1, 765, 0, 500, PF_Precision_INTEGER, NULL, NULL, HIGH_RANGE_SORT_LIMIT); 
  PF_ADD_FLOAT_SLIDER("Low Range Sort Limit", 0, 765, 0, 765, 0, 350, PF_Precision_INTEGER, NULL, NULL, LOW_RANGE_SORT_LIMIT); 
  PF_END_TOPIC(MANUAL_SORT_RANGE_GROUP_END); 
  
  
  PF_ADD_TOPIC("Misc", MISC_GROUP_START);
  PF_ADD_FLOAT_SLIDER("Minimum Sort Length", 1, 200, 1, 200, 0, 1, PF_Precision_INTEGER, NULL, NULL, MIN_SORT_LENGTH_SLIDER);  
  PF_ADD_FLOAT_SLIDER("Minimum Sort Random", 1, 200, 1, 200, 0, 1, PF_Precision_INTEGER, NULL, NULL, MIN_SORT_RAND_SLIDER);  
  PF_ADD_FLOAT_SLIDER("Minimum Reverse Sort Distance", 0, 300, 0, 300, 0, 0, PF_Precision_INTEGER, NULL, NULL, MIN_REVERSE_DIST_SLIDER); 
  PF_ADD_CHECKBOX("Interpolate Pixel Ranges", "Enabled", 0, NULL, PIXEL_INTERPOLATION_CHECKBOX);
  PF_END_TOPIC(MISC_GROUP_END);
 
  out_data->num_params = SORT_NUM_PARAMS; 

  
  return err;
}





static PF_Err 
ShiftImage8 (void     *refcon, 
             A_long   xL, 
             A_long   yL, 
             PF_Pixel *inP, 
             PF_Pixel *outP)  
{
  
  register ShiftInfo *siP = reinterpret_cast<ShiftInfo*>(refcon);  
  PF_Err             err  = PF_Err_NONE;
            
  int x = dynamic_cast<PixelSorter*>(siP->pixelSorter)->param.orientation == 1 ? xL : yL;
  int y = dynamic_cast<PixelSorter*>(siP->pixelSorter)->param.orientation == 1 ? yL : xL;
  
  if (siP->mapCreated) 
  { 
    *outP = siP->pixelMap[x][y].pixel;
  } 
  else 
  {
    siP->pixelMap[x][y] = *inP;
  }

  return err;
}



static PF_Err 
Render (PF_InData   *in_data,
        PF_OutData  *out_data,
        PF_ParamDef *params[],
        PF_LayerDef *output )
{

  ShiftInfo       si;
  PF_Err          err        = PF_Err_NONE;
  PF_Fixed        sortLength = params[SORT_VALUE_RANGE]->u.fd.value;
  A_long          linesL     = in_data->height;
  PF_EffectWorld  *inputP    = &params[SORT_INPUT]->u.ld;


  AEFX_CLR_STRUCT(si);

  si.ref         = in_data->effect_ref;
  si.samp_pb.src = inputP;
  si.in_data     = *in_data;

  AEGP_SuiteHandler  suites(in_data->pica_basicP);    

  // iterate() checks for user interruptions.

  ERR(suites.Iterate8Suite1()->iterate(
    in_data,
    0, 
    linesL, 
    inputP, 
    &output->extent_hint,
    (void*)&si, 
    ShiftImage8, 
    output));

  
  ERR(PF_BLEND(output, inputP, sortLength, output));
    
  return err;
}





static PF_Err 
RespondtoAEGP (PF_InData    *in_data,
               PF_OutData   *out_data,
               PF_ParamDef  *params[],
               PF_LayerDef  *output,
               void*        extraP)
{
  PF_Err err = PF_Err_NONE;
  
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  
  suites.ANSICallbacksSuite1()->sprintf(
    out_data->return_msg, 
    "%s",  
    reinterpret_cast<A_char*>(extraP));
  
  return err;
}





static PF_Err
SmartRender(PF_InData*            in_data,
            PF_OutData*           out_data,
            PF_ParamDef*          params[],
            PF_SmartRenderExtra*  extra)

{
  PF_Err err  = PF_Err_NONE;
  PF_Err err2 = PF_Err_NONE;
            
  AEGP_SuiteHandler  suites(in_data->pica_basicP);
  PF_EffectWorld*    input_worldP  = NULL; 
  PF_EffectWorld*    output_worldP = NULL;
  PF_WorldSuite2*    wsP           = NULL;
  PF_PixelFormat     format        = PF_PixelFormat_INVALID;
  PF_Point           origin;
  
  
  ShiftInfo *infoP{reinterpret_cast<ShiftInfo*>(
    suites.HandleSuite1()->host_lock_handle(
      reinterpret_cast<PF_Handle>(
        extra->input->pre_render_data))) };
  
  AEFX_SuiteScoper<PF_Iterate8Suite1> iterSuite {
    in_data,
    kPFIterate8Suite,
    kPFIterate8SuiteVersion1,
    out_data 
  };
  
  AEFX_SuiteScoper<PF_WorldSuite2> worldSuite {
    in_data, 
    kPFWorldSuite, 
    kPFWorldSuiteVersion2, 
    out_data 
  };

  


  if (infoP) 
  {
    if (!infoP->no_opB) 
    {      
      // checkout input & output buffers.
      ERR((extra->cb->checkout_layer_pixels(
        in_data->effect_ref, 
        SORT_INPUT, 
        &input_worldP)));

      ERR(extra->cb->checkout_output(
        in_data->effect_ref, 
        &output_worldP));
      

      if (!err && output_worldP) 
      {        
        infoP->ref     = in_data->effect_ref;
        infoP->in_data = *in_data;
        
        ERR(worldSuite->PF_GetPixelFormat(input_worldP, &format));

        origin.h = (A_short)(in_data->output_origin_x);        
        origin.v = (A_short)(in_data->output_origin_y);        
         
        ERR(iterSuite->iterate_origin(
          in_data, 
          0, 
          output_worldP->height,
          input_worldP, 
          &input_worldP->extent_hint,
          &origin,
          (void*)(infoP),
          ShiftImage8,
          output_worldP));

        infoP->mapCreated = true;
        dynamic_cast<PixelSorter*>(infoP->pixelSorter)->sortPixelMap();

        ERR(iterSuite->iterate_origin(
          in_data, 
          0, 
          output_worldP->height,
          input_worldP, 
          &output_worldP->extent_hint,                      
          &origin,
          (void*)(infoP),
          ShiftImage8,
          output_worldP));

        infoP->mapCreated = false;
 
      } 
    } 
    else
    {
      ERR(PF_COPY(&infoP->inputCopy, output_worldP, NULL, NULL));
    }
           
    suites.HandleSuite1()->host_unlock_handle(
      reinterpret_cast<PF_Handle>(
        extra->input->pre_render_data));
  }
  else 
  {
    err = PF_Err_BAD_CALLBACK_PARAM;
  }
  

  ERR2(AEFX_ReleaseSuite(
    in_data,
    out_data,
    kPFWorldSuite, 
    kPFWorldSuiteVersion2, 
    "Couldn't release suite."));



  for (auto pMap : infoP->pixelMap) 
  {
    pMap.clear();
  }

  infoP->pixelMap.clear();

  ERR(extra->cb->checkin_layer_pixels(in_data->effect_ref, SORT_INPUT));
  

  return err;
}



static PF_Err
PreRender(PF_InData*         in_data,
          PF_OutData*        out_data,
          PF_ParamDef        *params[],
          PF_PreRenderExtra* extra)
{

  PF_Err            err = PF_Err_NONE;
  PF_RenderRequest  req = extra->input->output_request;
  PF_CheckoutResult in_result;
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  PF_Handle         infoH = suites.HandleSuite1()->host_new_handle(sizeof(ShiftInfo));

  if (infoH)
  {    
    ShiftInfo *infoP = reinterpret_cast<ShiftInfo*>(
      suites.HandleSuite1()->host_lock_handle(infoH));

    AEFX_CLR_STRUCT(*infoP);


    if (infoP)
    {
      extra->output->pre_render_data = infoH; 

      for (int i = 0; i < SORT_NUM_PARAMS; ++i)
      {
        PF_CHECKOUT_PARAM(in_data, i, in_data->current_time, 
          in_data->time_step, in_data->time_scale, &infoP->params[i]);
      }

      if (!err)
      {
        req.field = PF_Field_FRAME;        

        ERR(extra->cb->checkout_layer(  
          in_data->effect_ref,
          SORT_INPUT,
          SORT_INPUT,
          &req,
          in_data->current_time,
          in_data->local_time_step,
          in_data->time_scale,
          &in_result));
        
        in_data->shutter_angle = 180;
        in_data->shutter_phase = 0;
        
        infoP->in_data = *in_data;         
        infoP->pixelSorter = new PixelSorter{infoP, {infoP}};
        
        PixelSorter* tmpSort = dynamic_cast<PixelSorter*>(infoP->pixelSorter);

        if (!err)
        {                    
          for (int i = 0; i < dynamic_cast<PixelSorter*>(infoP->pixelSorter)->pixelLines; ++i) 
          {
            infoP->pixelMap.push_back(vector<PixelStruct>{});
            
            for (int j = 0; j < dynamic_cast<PixelSorter*>(infoP->pixelSorter)->linePixels; ++j) 
              infoP->pixelMap[i].push_back(PixelStruct{});
          }
          UnionLRect(&in_result.result_rect,     &extra->output->result_rect);
          UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);  
          
          //  Notice something missing, namely the PF_CHECKIN_PARAM to balance
          //  the old-fashioned PF_CHECKOUT_PARAM, above?  
          //  For SmartFX, AE automagically checks in any params checked out 
          //  during PF_Cmd_SMART_PRE_RENDER, new or old-fashioned.
        }
      }
      for (int i = 0; i < SORT_NUM_PARAMS; ++i)
      {
        PF_CHECKIN_PARAM(in_data, &infoP->params[i]);
      }
      suites.HandleSuite1()->host_unlock_handle(infoH);
    } 
    else
    {
      err = PF_Err_OUT_OF_MEMORY;    
    }
  } 
  else 
  {
    err = PF_Err_OUT_OF_MEMORY;
  }
  
  return err;  
}





DllExport  PF_Err 
EntryPointFunc (PF_Cmd      cmd,
                PF_InData   *in_data,
                PF_OutData  *out_data,
                PF_ParamDef *params[],
                PF_LayerDef *output,
                void        *extraP)
{
  PF_Err err = PF_Err_NONE;
  AEGP_SuiteHandler suite{in_data->pica_basicP};

  try
  {
    switch (cmd) 
    {      
      case PF_Cmd_ABOUT:        
        err = About(in_data, out_data, params, output);
        break;
      
      case PF_Cmd_GLOBAL_SETUP:        
        err = GlobalSetup(in_data, out_data,params, output);        
        break;
      
      case PF_Cmd_PARAMS_SETUP:        
        err = ParamsSetup(in_data, out_data, params, output);       
        break;      

      case PF_Cmd_RENDER:        
        err = Render(in_data, out_data, params, output);        
        break;
            
      case PF_Cmd_SMART_PRE_RENDER:        
        err = PreRender(in_data, out_data,params,
          reinterpret_cast<PF_PreRenderExtra*>(extraP));        
        break;
        
      case PF_Cmd_SMART_RENDER:       
        err = SmartRender(in_data, out_data,params,
          reinterpret_cast<PF_SmartRenderExtra*>(extraP));        
        break;

      case PF_Cmd_COMPLETELY_GENERAL:        
        err = RespondtoAEGP(in_data, out_data, params, output, extraP);
    }
  }
  catch(PF_Err &thrown_err)
  {
    err = thrown_err;
  }
  return err;
}









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

ParamValues::ParamValues()
{
}

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
  sortByMenuChoice      {shiftInfo->params[SORT_BY_DROPDOWN].u.pd.value}
  
{

}


void PixelSorter::operator=(ShiftInfo & shiftInfo)
{
  *this = shiftInfo;
}





PixelSorter::~PixelSorter()
{
  delete this;
}




                                                                 

inline void PixelSorter::
storeBeginRowIters()
{ 
  for (PF_Fixed j = lineCounter; j < (lineCounter + param.sortWidth) && j < pixelLines - 1; ++j) 
  { 
    current_segment.beginItems.push_back(
      sortSegment::BeginItems{
        (shiftInfoCopy->pixelMap[j].begin()+pixelCounter), j, pixelCounter});     
  }
}






inline void PixelSorter::
storeEndRowIters()
{  
  for (PF_Fixed j = lineCounter; j < (lineCounter + param.sortWidth) && j < pixelLines - 1; ++j) 
  { 
    current_segment.endItems.push_back(
      sortSegment::EndItems{
        (shiftInfoCopy->pixelMap[j].begin()+pixelCounter), j, pixelCounter});     
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
  auto columnWidthSpan = lineCounter+param.sortWidth;  
  
  PF_FpLong r_avg; 
  PF_FpLong g_avg; 
  PF_FpLong b_avg;



  for (auto i = lineCounter; i<columnAvg && i<pixelLines-1; ++i)
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


  if (r_avg<current_segment.rgb_sort.high_value.red)
    current_segment.rgb_sort.high_value.red = r_avg;
  
  if (g_avg<current_segment.rgb_sort.high_value.green)
    current_segment.rgb_sort.high_value.green = g_avg;
  
  if (b_avg<current_segment.rgb_sort.high_value.blue)
    current_segment.rgb_sort.high_value.blue = b_avg;

  
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
      if (!current_segment.isEmpty) 
      {
        resetSortingVariables();
        storeBeginRowIters();                
        getAndStorePixelValue();  
        current_segment.isEmpty = false;
      }
      if (pixelDistanceIsLongEnoughToSort()) 
      {                                        
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
    reverse(current_segment.beginItems[index], current_segment.endItems[index]);
}





inline bool PixelSorter::pixelDistanceIsLongEnoughToSort()
{
  getSortLength();
  getUserSetMinLength();

  switch (param.sortByMenuChoice)
  {
    case SORT_BY_LUMINOSITY:
      break;

    case SORT_BY_RGB:
      break;
  }


  switch (param.sortMethodMenuChoice)
  {
    case USER_MANUAL_SORT:
  
      if(current_segment.luminosity_sort.high_value>=param.highRangeLimit && 
          current_segment.luminosity_sort.high_value<=param.lowRangeLimit)
      {
        if (currentPixelValueDistance>=sortLength || 
              pixelCounter==linePixels-1)
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
        sort(current_segment.beginItems[h], current_segment.endItems[h], sortFunc);        
        reverseSortIfTrue(param.selectedReverseSort||lengthIsShortEnoughForFlip,h);
      }
      break;
      
    case SORT_BY_RGB:

      for (auto h = 0; h<current_segment.beginItems.size(); ++h)
      {
        for (auto k = current_segment.beginItems[h]; k!=current_segment.endItems[h]; ++k)
        {
          current_segment.beginItems.
          shiftInfoCopy->pixelMap
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
      break;
    
    case SORT_BY_RGB:
      getLineWidthColorAverage();
      break;

    default: break;
  }
  

  getLineWidthPixelAverage();
  
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






inline void PixelSorter::sortSegment::getRGBInterpolatedVectors() 
{

  PF_FpLong red_range = rgb_sort.high_value.red - rgb_sort.low_value.red;  
  PF_FpLong red_interpolation_slope = red_range/segmentLength;
  
  PF_FpLong green_range = rgb_sort.high_value.green - rgb_sort.low_value.green;  
  PF_FpLong green_interpolation_slope = red_range/segmentLength;
  
  PF_FpLong blue_range = rgb_sort.high_value.blue - rgb_sort.low_value.blue;  
  PF_FpLong blue_interpolation_slope = blue_range/segmentLength;

  
  PF_FpLong red_start   = rgb_sort.low_value.red;
  PF_FpLong green_start = rgb_sort.low_value.green;
  PF_FpLong blue_start  = rgb_sort.low_value.blue;
  
  for (int i = 0; i < beginItems.size(); ++i) {
    
    replacementPixelsVecs.push_back(PF_Pixel{255, 
                                         static_cast<A_u_char>(red_start), 
                                         static_cast<A_u_char>(green_start), 
                                         static_cast<A_u_char>(blue_start)});
    
    red_start   = (red_start+red_interpolation_slope<=255) ? 
                    (red_start+=red_interpolation_slope) : red_start;
    
    green_start = (green_start+green_interpolation_slope<=255) ? 
                    (green_start+=green_interpolation_slope) : green_start;
    
    blue_start  = (blue_start+blue_interpolation_slope<=255) ? 
                    (blue_start+=blue_interpolation_slope) : blue_start;        
  }
}



inline void PixelSorter::sortSegment::reset()
{
  *this = sortSegment{};
}


