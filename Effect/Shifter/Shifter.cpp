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
  PF_ADD_SLIDER("Sort Range Booster", 5, 350, 5, 350, 200, SORT_LENGTH_BOOSTER_SLIDER);
  PF_ADD_SLIDER("Minimum Sort Length", 5, 200, 5, 200, 50, MIN_SORT_LENGTH_SLIDER);
  PF_ADD_SLIDER("Sort Width", 1, 50, 1, 100, 35, SORT_WIDTH_SLIDER);
  PF_ADD_CHECKBOX("Variable Sort Length", "Variable Sort Length", 1, NULL, VARIABLE_SORT_CHECKBOX);
  PF_ADD_SLIDER("Variable Range", 0, 100, 0, 100, 50, VARIABLE_SLIDER);
  
 
  out_data->num_params = SHIFT_NUM_PARAMS; 

  
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
            

  if (siP->mapCreated) 
  {           
    //*inP = siP->pixelMap[xL][yL];
    outP->alpha = siP->pixelMap[xL][yL].alpha;
    outP->blue  = siP->pixelMap[xL][yL].blue;
    outP->green = siP->pixelMap[xL][yL].green;
    outP->red   = siP->pixelMap[xL][yL].red;
  } 
  else 
  {
    siP->pixelMap[xL][yL] = *inP;
  }
   
  return err;
}





inline int getSortLength(int rgbValue, ShiftInfo* shiftInfo)
{
  //float divider = static_cast<float>(shiftInfo->sortRangeBoosterSliderValue);
  float variability = pow(shiftInfo->variableValue/100, -1) * 700000;
  return static_cast<int>(pow(rgbValue, 3)/variability);
}





inline int getPixelValue(const PF_Pixel& pix)
{
  return pix.blue + pix.green + pix.red;
}





std::function<bool(const PF_Pixel&, const PF_Pixel&)> 
sortFunc = [&](const PF_Pixel& left, const PF_Pixel& right)
{
  return getPixelValue(left) < getPixelValue(right); 
};






inline void storeRowIters(int columnNumber, 
                          int columnPosition,
                          std::vector<std::vector<PF_Pixel>::iterator>& rowIters,
                          ShiftInfo* shiftInfo)
  {
    for(int k = columnNumber; k < (columnNumber+shiftInfo->sortWidthSliderValue) && 
        k < shiftInfo->in_data.width-1; ++k)
    {
      rowIters.push_back(shiftInfo->pixelMap[k].begin()+columnPosition);
    }
  }





inline int getColumnPixelAverage(int columnNumber, int rowNumber, ShiftInfo* shiftInfo)
{
  int rgbAverage{0};

  for(int k = columnNumber; 
      k < (columnNumber+shiftInfo->sortWidthSliderValue) && 
      k < shiftInfo->in_data.width-1; ++k)
  {
    rgbAverage += getPixelValue(shiftInfo->pixelMap[k][rowNumber]);
  }
  rgbAverage /= shiftInfo->sortWidthSliderValue;
  return rgbAverage;
}





void 
sortPixelMap(ShiftInfo* shiftInfo, 
             PF_InData* in_data) 
{  
  using namespace std;
  using highestPixelValueQueue = priority_queue<int, vector<int>, queueCompare>;
  using iteratorVector         = vector<vector<PF_Pixel>::iterator>;


  int sortLength        = 20;
  int minSortLength     = shiftInfo->minSortLengthSliderValue;
  int sortBoosterValue  = shiftInfo->sortRangeBoosterSliderValue;
  int sortWidth         = shiftInfo->sortWidthSliderValue;
  int maxPixelValueDistance{0};


  highestPixelValueQueue mostQueue;
  iteratorVector         rowBeginIters;
  iteratorVector         rowEndIters;



  // sorting routine
  for (int i{0}; i < in_data->width; i+=sortWidth) 
  {    
    rowBeginIters.clear();
    storeRowIters(i, 0, rowBeginIters, shiftInfo);

    for (int j{0}; j < in_data->height; ++j) 
    {                                                 
      if (mostQueue.empty()) 
      {
        rowBeginIters.clear();
        storeRowIters(i, j, rowBeginIters, shiftInfo);        
        mostQueue.push(getColumnPixelAverage(i, j, shiftInfo));
        sortLength = getSortLength(mostQueue.top(), shiftInfo);
      }
      else
      {  
        maxPixelValueDistance = abs(mostQueue.top()-getColumnPixelAverage(i,j,shiftInfo));
        
        if (!shiftInfo->variableSortOn)
        {
          sortLength = sortBoosterValue;
        }

        if (maxPixelValueDistance > sortLength && 
              maxPixelValueDistance > minSortLength|| 
                j == shiftInfo->in_data.height - 1) 
        {       
          storeRowIters(i, j, rowEndIters, shiftInfo);          
          
          for (int h{0}; h < rowBeginIters.size(); ++h)
          {
            std::sort(rowBeginIters[h], rowEndIters[h], sortFunc);                                                                  
          }
          
          for (auto k = mostQueue.size(); k > 0; --k) 
          {
            mostQueue.pop();
          }
          
          rowBeginIters.clear();
          rowEndIters.clear();
          continue;
        } 
        else
        {
          mostQueue.push(getColumnPixelAverage(i, j, shiftInfo));
          sortLength = getSortLength(mostQueue.top(), shiftInfo);                
        }
      }
    }           
  }
  rowBeginIters.clear();
  rowEndIters.clear();
  for (int i = 0; i<mostQueue.size(); ++i)
  {
    mostQueue.pop();
  }
}





static PF_Err 
Render (PF_InData   *in_data,
        PF_OutData  *out_data,
        PF_ParamDef *params[],
        PF_LayerDef *output )
{

  ShiftInfo       si;
  PF_Err          err        = PF_Err_NONE;
  PF_Fixed        sortLength = params[SORT_LENGTH_BOOSTER_SLIDER]->u.fd.value;
  A_long          linesL     = in_data->height;
  PF_EffectWorld  *inputP    = &params[SHIFT_INPUT]->u.ld;


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
SmartRender(PF_InData           *in_data,
            PF_OutData          *out_data,
            PF_SmartRenderExtra *extra)

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
        extra->input->pre_render_data)))};
  
  
  AEFX_SuiteScoper<PF_Iterate8Suite1> iterSuite {
    in_data,
    kPFIterate8Suite,
    kPFIterate8SuiteVersion1,
    out_data
  };
  
  AEFX_SuiteScoper<PF_WorldSuite2> worldSuite {
    AEFX_SuiteScoper<PF_WorldSuite2>{
      in_data, 
      kPFWorldSuite, 
      kPFWorldSuiteVersion2, 
      out_data}
  };




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
 
        iterSuite->iterate_origin(
          in_data, 
          0, 
          output_worldP->height,
          input_worldP, 
          &input_worldP->extent_hint,
          &origin,
          (void*)(infoP),
          ShiftImage8,
          output_worldP);

              
        infoP->mapCreated = true;
        sortPixelMap(infoP, in_data);
              

        iterSuite->iterate_origin(
          in_data, 
          0, 
          output_worldP->height,
          input_worldP, 
          &input_worldP->extent_hint,
          &origin,
          (void*)(infoP),
          ShiftImage8,
          output_worldP);


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
    {
      // copy input buffer;
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

  for (auto map:infoP->pixelMap)
  {
    map.clear();
  }
  infoP->pixelMap.clear();


  extra->cb->checkin_layer_pixels(in_data->effect_ref, INPUT_ID);        


  return err;
}





static PF_Err
PreRender(PF_InData         *in_data,
          PF_OutData        *out_data,
          PF_PreRenderExtra *extra)
{

  PF_Err            err = PF_Err_NONE;
  PF_ParamDef       sortSliderParam; 
  PF_ParamDef       minSortSlider; 
  PF_ParamDef       shiftSortButton;
  PF_ParamDef       sortWidthSlider;
  PF_ParamDef       variableSortCheckbox;
  PF_ParamDef       variableSlider;
  PF_RenderRequest  req = extra->input->output_request;
  PF_CheckoutResult in_result;
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  PF_Handle         infoH = suites.HandleSuite1()->host_new_handle(sizeof(ShiftInfo));
    
  
  shiftSortButton.param_type = PF_Param_BUTTON;
  shiftSortButton.flags = PF_ParamFlag_SUPERVISE;
  


  if (infoH)
  {    
    ShiftInfo *infoP = reinterpret_cast<ShiftInfo*>(
      suites.HandleSuite1()->host_lock_handle(infoH));
    
    
    if (infoP)
    {
      extra->output->pre_render_data = infoH;
      
      AEFX_CLR_STRUCT(sortSliderParam);
      AEFX_CLR_STRUCT(minSortSlider);
      AEFX_CLR_STRUCT(sortWidthSlider);
      AEFX_CLR_STRUCT(shiftSortButton);
      AEFX_CLR_STRUCT(variableSortCheckbox);
      
      
      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_BUTTON,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &shiftSortButton)); 

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_LENGTH_BOOSTER_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &sortSliderParam));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        MIN_SORT_LENGTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &minSortSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_WIDTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &sortWidthSlider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        VARIABLE_SORT_CHECKBOX,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &variableSortCheckbox));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        VARIABLE_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &variableSlider));

      

      if (!err)
      {
        // Hey, we care about zero alpha
        req.preserve_rgb_of_zero_alpha = TRUE;  
        // We want checkout_layer to provide a complete frame for sampling
        req.field = PF_Field_FRAME;        

        ERR(extra->cb->checkout_layer(  
          in_data->effect_ref,
          SHIFT_INPUT,
          INPUT_ID,
          &req,
          in_data->current_time,
          in_data->time_step,
          in_data->time_scale,
          &in_result));
        

        if (!err)
        {
          AEFX_CLR_STRUCT(*infoP);
          infoP->sortRangeBoosterSliderValue = sortSliderParam.u.fd.value;
          infoP->minSortLengthSliderValue    = minSortSlider.u.fd.value;
          infoP->sortWidthSliderValue        = sortWidthSlider.u.fd.value;
          infoP->sortButton                  = shiftSortButton;
          infoP->variableSortOn              = variableSortCheckbox.u.fd.value;
          infoP->variableValue               = static_cast<float>(variableSlider.u.fd.value);
          
          for (int i = 0; i < in_data->width; ++i) 
          {
            infoP->pixelMap.push_back(std::vector<PF_Pixel>{});
            for (int j = 0; j < in_data->height; ++j) 
            {
              infoP->pixelMap[i].push_back(PF_Pixel{});
            }
          }

          UnionLRect(&in_result.result_rect,     &extra->output->result_rect);
          UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);  

          //  Notice something missing, namely the PF_CHECKIN_PARAM to balance
          //  the old-fashioned PF_CHECKOUT_PARAM, above? 
          
          //  For SmartFX, AE automagically checks in any params checked out 
          //  during PF_Cmd_SMART_PRE_RENDER, new or old-fashioned.
        }

        PF_CHECKIN_PARAM(in_data, &variableSlider);
        PF_CHECKIN_PARAM(in_data, &variableSortCheckbox);
        PF_CHECKIN_PARAM(in_data, &sortWidthSlider);
        PF_CHECKIN_PARAM(in_data, &minSortSlider);
        PF_CHECKIN_PARAM(in_data, &variableSlider);
        PF_CHECKIN_PARAM(in_data, &sortSliderParam);
      }

      //suites.HandleSuite1()->host_unlock_handle(infoH);
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

