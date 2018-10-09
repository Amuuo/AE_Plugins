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


  out_data->out_flags =  PF_OutFlag_PIX_INDEPENDENT  |
                         PF_OutFlag_DEEP_COLOR_AWARE |
                         PF_OutFlag_NON_PARAM_VARY |
                         PF_OutFlag_SEND_UPDATE_PARAMS_UI |
                         PF_OutFlag_USE_OUTPUT_EXTENT |
                         PF_OutFlag_REFRESH_UI | 
                         PF_OutFlag_FORCE_RERENDER;

  out_data->out_flags2 = PF_OutFlag2_FLOAT_COLOR_AWARE |
                         PF_OutFlag2_SUPPORTS_SMART_RENDER |
                         PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS;

  


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

  
  PF_ADD_BUTTON("Sort", "Sort", PF_ParamFlag_SUPERVISE, NULL, SORT_BUTTON);
  PF_ADD_SLIDER("Sort Value Range", 1, 765, 1, 765, 100, SORT_VALUE_RANGE);
  PF_ADD_SLIDER("Minimum Sort Length", 1, 200, 1, 200, 1, MIN_SORT_LENGTH_SLIDER);
  PF_ADD_SLIDER("Minimum Sort Random", 1, 200, 1, 200, 1, MIN_SORT_RAND_SLIDER);
  PF_ADD_SLIDER("Sort Width", 1, 200, 1, 200, 5, SORT_WIDTH_SLIDER);
  PF_ADD_CHECKBOX("Variable Sort Length", "Variable Sort Length", 0, NULL, VARIABLE_SORT_CHECKBOX);
  PF_ADD_SLIDER("Variable Range", 0, 300, 0, 300, 50, VARIABLE_SLIDER);
  PF_ADD_SLIDER("Minimum Reverse Sort Distance", 0, 300, 0, 300, 0, MIN_REVERSE_DIST_SLIDER);
  PF_ADD_CHECKBOX("Reverse Sort", "Reverse Sort", 0, NULL, REVERSE_SORT_CHECKBOX);
  PF_ADD_CHECKBOX("Switch Orientation", "Switch Orientation", 0, NULL, SWITCH_ORIENTAION_CHECKBOX);
  
 
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
            
  int x = siP->pixelSorter->verticalOrientation ? xL : yL;
  int y = siP->pixelSorter->verticalOrientation ? yL : xL;
  
  if (siP->mapCreated) 
  {           
    //*inP = siP->pixelMap[xL][yL];
    //outP->alpha = siP->pixelMap[xL][yL].alpha;
    //outP->blue  = siP->pixelMap[xL][yL].blue;
    //outP->green = siP->pixelMap[xL][yL].green;
    //outP->red   = siP->pixelMap[xL][yL].red;
    
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
    out_data };
  
  AEFX_SuiteScoper<PF_WorldSuite2> worldSuite {
    AEFX_SuiteScoper<PF_WorldSuite2>{
      in_data, 
      kPFWorldSuite, 
      kPFWorldSuiteVersion2, 
      out_data} };

  

  if (infoP) 
  {    
    if (!infoP->no_opB) 
    {      
      // checkout input & output buffers.
      ERR((extra->cb->checkout_layer_pixels(
        in_data->effect_ref, 
        INPUT_ID, 
        &input_worldP)));

      ERR(extra->cb->checkout_output(
        in_data->effect_ref, 
        &output_worldP));


      if (!err && output_worldP) 
      {        
        infoP->ref         = in_data->effect_ref;
        infoP->in_data     = *in_data;
        infoP->inputCopy   = *input_worldP;
        infoP->samp_pb.src = input_worldP;
        
        
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
   
        vector<vector<PixelStruct>> pixelMapCopy{infoP->pixelMap};
        infoP->mapCreated = true;
        infoP->pixelSorter->sortPixelMap();
              

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


        infoP->mapCreated = false;
 
      }
        /*ERR(suites.WorldTransformSuite1()->copy_hq(
            in_data->effect_ref,
            input_worldP,
            output_worldP,
            NULL,
            NULL));
        */
    } 
    else           
      ERR(PF_COPY(&infoP->inputCopy, output_worldP, NULL, NULL));
    
                 
    suites.HandleSuite1()->host_unlock_handle(
      reinterpret_cast<PF_Handle>(
        extra->input->pre_render_data));
  }
  else   
    err = PF_Err_BAD_CALLBACK_PARAM;
  

  ERR2(AEFX_ReleaseSuite(
    in_data,
    out_data,
    kPFWorldSuite, 
    kPFWorldSuiteVersion2, 
    "Couldn't release suite."));


  for (auto pMap : infoP->pixelMap)  
    pMap.clear();

  infoP->pixelMap.clear();


  ERR(extra->cb->checkin_layer_pixels(in_data->effect_ref, INPUT_ID));

  return err;
}





static PF_Err
PreRender(PF_InData*         in_data,
          PF_OutData*        out_data,
          PF_PreRenderExtra* extra)
{

  PF_Err            err = PF_Err_NONE;
  PF_RenderRequest  req = extra->input->output_request;
  PF_CheckoutResult in_result;
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  PF_Handle         infoH = suites.HandleSuite1()->host_new_handle(sizeof(ShiftInfo));
    

  /*
  AEGP_LayerH currentLayer;
  AEGP_ItemH currentItem;
  AEGP_ItemType itemType;
  A_long itemWidth;
  A_long itemHeight;
  A_long numOfEffects;
  AEGP_MaskRefH maskRef;
  AEGP_MaskRefH maskRefNew;
  AEGP_EffectRefH effectRef;
  AEGP_StreamRefH streamRef;
  PF_EffectWorld testEffectWorld;
  A_long numOfParams;
  AEGP_WorldH newWorld;
  AEGP_MaskOutlineValH maskOutline;
  AEGP_MaskVertex maskVertex;
  AEGP_StreamValue2 streamValue;
  AEGP_StreamType streamType;
  A_FloatRect layerMaskRect;
  A_long numOfMasks;
  A_Time currentATime;
  AEGP_MaskIDVal maskID;
  A_long newMaskIndex;


  suites.LayerSuite8()->AEGP_GetActiveLayer(&currentLayer);
  suites.LayerSuite8()->AEGP_GetLayerMaskedBounds(currentLayer, AEGP_LTimeMode_CompTime, &currentATime, &layerMaskRect);
  suites.ItemSuite9()->AEGP_GetActiveItem(&currentItem);
  suites.StreamSuite4()->AEGP_GetNewLayerStream(pluginID, currentLayer, AEGP_LayerStream_POSITION, &streamRef);
 // suites.MaskSuite6()->AEGP_GetLayerMaskByIndex(currentLayer, 1, &maskRef);
  //suites.MaskSuite6()->AEGP_GetMaskID(maskRef, &maskID);
  suites.MaskSuite6()->AEGP_CreateNewMask(currentLayer, &maskRefNew, &newMaskIndex);
  suites.ItemSuite9()->AEGP_GetItemType(currentItem, &itemType);
  suites.ItemSuite9()->AEGP_GetItemDimensions(currentItem, &itemWidth, &itemHeight);
  suites.EffectSuite4()->AEGP_GetLayerNumEffects(currentLayer, &numOfEffects);
  suites.EffectSuite4()->AEGP_GetLayerEffectByIndex(pluginID, currentLayer, 0, &effectRef);
  suites.StreamSuite4()->AEGP_GetEffectNumParamStreams(effectRef, &numOfParams);
  
  suites.WorldSuite3()->AEGP_New(pluginID, AEGP_WorldType_8, 300, 300, &newWorld); 
  suites.WorldSuite3()->AEGP_FillOutPFEffectWorld(newWorld, &testEffectWorld);
  
  suites.MaskSuite6()->AEGP_GetLayerMaskByIndex(currentLayer, 0, &maskRef);
  suites.StreamSuite4()->AEGP_GetStreamType(streamRef, &streamType);
  suites.StreamSuite4()->AEGP_GetNewStreamValue(pluginID, streamRef, AEGP_LTimeMode_LayerTime, &currentATime, true, &streamValue);
  suites.MaskSuite6()->AEGP_GetLayerNumMasks(currentLayer, &numOfMasks);
  */

  if (infoH)
  {    
    ShiftInfo *infoP = reinterpret_cast<ShiftInfo*>(
      suites.HandleSuite1()->host_lock_handle(infoH));
    
    AEFX_CLR_STRUCT(*infoP);

    if (infoP)
    {
      extra->output->pre_render_data = infoH;
    
      
      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_BUTTON,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->shiftSortButton)); 

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_VALUE_RANGE,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->sortValueRange));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        MIN_SORT_LENGTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->minSortSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        MIN_SORT_RAND_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->minSortRandomSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_WIDTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->sortWidthSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        VARIABLE_SORT_CHECKBOX,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->variableSortCheckbox));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        VARIABLE_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->variableSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        MIN_REVERSE_DIST_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->minReverseSortSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        REVERSE_SORT_CHECKBOX,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->reverseSortCheckbox));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SWITCH_ORIENTAION_CHECKBOX,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &infoP->switchOrientationCheckbox));
      

      if (!err)
      {
        // Hey, we care about zero alpha
        req.preserve_rgb_of_zero_alpha = TRUE;  
        // We want checkout_layer to provide a complete frame for sampling
        req.field = PF_Field_FRAME;        

        ERR(extra->cb->checkout_layer(  
          in_data->effect_ref,
          SORT_INPUT,
          INPUT_ID,
          &req,
          in_data->current_time,
          in_data->time_step,
          in_data->time_scale,
          &in_result));
        
        
        PF_CHECKIN_PARAM(in_data, &infoP->variableSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->variableSortCheckbox);
        PF_CHECKIN_PARAM(in_data, &infoP->minSortSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->minSortRandomSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->sortWidthSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->variableSortCheckbox);
        PF_CHECKIN_PARAM(in_data, &infoP->variableSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->minReverseSortSlider);
        PF_CHECKIN_PARAM(in_data, &infoP->reverseSortCheckbox);
        PF_CHECKIN_PARAM(in_data, &infoP->switchOrientationCheckbox);
        
        
        infoP->in_data = *in_data;
        infoP->pixelSorter = new ShiftInfo::PixelSorter{infoP};
        
        if (!err)
        {
          for (int i = 0; i < infoP->pixelSorter->pixelLines; ++i) 
          {
            infoP->pixelMap.push_back(std::vector<PixelStruct>{});
            
            for (int j = 0; j < infoP->pixelSorter->linePixels; ++j) 
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

      //suites.HandleSuite1()->host_unlock_handle(infoH);
      suites.HandleSuite1()->host_unlock_handle(infoH);
    } 
    else 
      err = PF_Err_OUT_OF_MEMORY;    
  } 
  else 
    err = PF_Err_OUT_OF_MEMORY;
  
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
        
        err = About(
          in_data, 
          out_data, 
          params, 
          output);

        break;
      
      case PF_Cmd_GLOBAL_SETUP:
        
        err = GlobalSetup(
          in_data, 
          out_data,
          params, 
          output);
        
        break;
      
      case PF_Cmd_PARAMS_SETUP:
        
        err = ParamsSetup(
          in_data, 
          out_data, 
          params, 
          output);
        
        break;

      case PF_Cmd_USER_CHANGED_PARAM:
                        
        break;

      case PF_Cmd_SEQUENCE_RESETUP:

        break;

      case PF_Cmd_RENDER:
        
        err = Render(
          in_data, 
          out_data,
          params, 
          output);
        
        break;

      case PF_Cmd_FRAME_SETUP:
        
        
        break;

      case PF_Cmd_EVENT:
        
        
        break;

      case PF_Cmd_SMART_PRE_RENDER:
        
        err = PreRender(
          in_data, 
          out_data, 
          reinterpret_cast<PF_PreRenderExtra*>(extraP));
        
        break;
        
      case PF_Cmd_SMART_RENDER:
       
        err = SmartRender(
          in_data, 
          out_data,
          params,
          reinterpret_cast<PF_SmartRenderExtra*>(extraP));
        
        break;

      case PF_Cmd_COMPLETELY_GENERAL:
        
        err = RespondtoAEGP(
          in_data, 
          out_data, 
          params, 
          output, 
          extraP);
    }
  }
  catch(PF_Err &thrown_err)
  {
    err = thrown_err;
  }
  return err;
}









ShiftInfo::PixelSorter::
PixelSorter(ShiftInfo * shiftInfo)  : 
    minSortLength           {shiftInfo->minSortSlider.u.fd.value}, 
    sortValueRange          {shiftInfo->sortValueRange.u.fd.value},
    sortLength              {shiftInfo->sortValueRange.u.fd.value},
    sortWidth               {shiftInfo->sortWidthSlider.u.fd.value},
    userMinReverseSortValue {shiftInfo->minReverseSortSlider.u.fd.value},
    minSortRandValue        {shiftInfo->minSortRandomSlider.u.fd.value},
    userSelectedReverseSort {shiftInfo->reverseSortCheckbox.u.bd.value},
    layerWidth              {shiftInfo->in_data.width}, 
    layerHeight             {shiftInfo->in_data.height},
    variableValue           {(float)shiftInfo->variableSlider.u.fd.value},
    userSelectedVariableSort{shiftInfo->variableSortCheckbox.u.bd.value},
    verticalOrientation     {(bool)shiftInfo->switchOrientationCheckbox.u.bd.value},
    shiftInfoCopy           {shiftInfo}
{
    pixelLines = verticalOrientation  ? layerWidth : layerHeight;
    linePixels = verticalOrientation ? layerHeight : layerWidth;
}







ShiftInfo::PixelSorter::~PixelSorter()
{
  delete this;
}





inline void ShiftInfo::PixelSorter::
storeRowIters(iteratorVector & rowIters)
{
  for(int j = lineCounter; j < (lineCounter+sortWidth) && j < pixelLines-1; ++j)
    rowIters.push_back(shiftInfoCopy->pixelMap[j].begin()+pixelCounter);
}



                                                                 

inline void ShiftInfo::PixelSorter::
storeBeginRowIters()
{
  storeRowIters(rowBeginIters);
}






inline void ShiftInfo::PixelSorter::
storeEndRowIters()
{
  storeRowIters(rowEndIters);
}





inline void ShiftInfo::PixelSorter::
getSortLength()
{
  sortLength = sortValueRange;
  if (userSelectedVariableSort)
  {
    double variability = pow(variableValue/100, -1) * 800000;
    variability = static_cast<int>(pow(mostQueue.top(), 3)/variability);  
    sortLength *= sortValueRange/15;        
  }
}






inline void ShiftInfo::PixelSorter::
getLineWidthPixelAverage()
{
  columnAvg = 0;
  int columnWidthSpan = lineCounter + sortWidth;
  for(int i = lineCounter; i < columnWidthSpan && i < pixelLines-1; ++i)  
    columnAvg += shiftInfoCopy->pixelMap[i][pixelCounter].pixelValue;
  
  columnAvg /= sortWidth;  
}






void ShiftInfo::PixelSorter::
sortPixelMap()
{
  for (lineCounter=0; lineCounter<pixelLines; lineCounter+=sortWidth) 
  { 
    resetSortingVariables();
    storeBeginRowIters();
    for (pixelCounter=0; pixelCounter<linePixels; ++pixelCounter, ++currPixDistance) 
    {   
      if (mostQueue.empty()) 
      {
        resetSortingVariables();
        storeBeginRowIters();                
        getAndStorePixelValue();        
      }
      else if (pixelDistanceIsLongEnoughToSort()) 
      {                                        
        storeEndRowIters();
        sortPixelSegments();                    
        resetSortingVariables();
        continue;
      } 
      else
        getAndStorePixelValue();     
    }           
  }
  resetSortingVariables();
}





inline void ShiftInfo::PixelSorter::resetSortingVariables()
{
  while (!mostQueue.empty()) mostQueue.pop();          
  while (!leastQueue.empty()) leastQueue.pop();
          
  currPixDistance = 0;
  rowBeginIters.clear();
  rowEndIters.clear();
  pixValueAverage = 0;
  columnAvg = 0;
  lengthIsShortEnoughForFlip = false;
  userMinLength = 0;
  currentPixelValueDistance = 0;
}






inline void ShiftInfo::PixelSorter::reverseSortIfTrue(bool needToReverse, int index)
{
  if (needToReverse)
    reverse(rowBeginIters[index], rowEndIters[index]);
}





inline bool ShiftInfo::PixelSorter::pixelDistanceIsLongEnoughToSort()
{
  getSortLength();
  getUserSetMinLength();

  if (((currentPixelValueDistance >= sortLength) && 
        (currPixDistance>userMinLength))||
          (pixelCounter==linePixels-1))
  {
      
    lengthIsShortEnoughForFlip = currPixDistance < userMinReverseSortValue ? true:false; 
    return true;
  }
  else
  {
    return false;
  }
}






inline void ShiftInfo::PixelSorter::sortPixelSegments()
{
  for (int h = 0; h < rowBeginIters.size(); ++h)
  {
    sort(rowBeginIters[h], rowEndIters[h], sortFunc);        
    reverseSortIfTrue(userSelectedReverseSort||lengthIsShortEnoughForFlip,h);
  }
}





inline void ShiftInfo::PixelSorter::getAndStorePixelValue()
{
  getLineWidthPixelAverage();
  mostQueue.push(columnAvg);
  leastQueue.push(columnAvg);
  currentPixelValueDistance = mostQueue.top()-leastQueue.top();
}






inline void ShiftInfo::PixelSorter::getUserSetMinLength()
{
  userMinLength = minSortRandValue;
  if (minSortRandValue >= 2) 
  {
    userMinLength += minSortLength; 
    userMinLength += random() % (minSortRandValue/2) - minSortRandValue;
    userMinLength *= (765-columnAvg)/765 + 1;
  }
}



