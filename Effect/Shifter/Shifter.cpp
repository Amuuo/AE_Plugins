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
  out_data->my_version = PF_VERSION(MAJOR_VERSION, 
                                    MINOR_VERSION,
                                    BUG_VERSION, 
                                    STAGE_VERSION, 
                                    BUILD_VERSION);


  out_data->out_flags =  PF_OutFlag_PIX_INDEPENDENT  |
                         PF_OutFlag_DEEP_COLOR_AWARE |
                         PF_OutFlag_NON_PARAM_VARY |
                         PF_OutFlag_SEND_UPDATE_PARAMS_UI;

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
  
  def.flags = PF_ParamFlag_SUPERVISE;

  PF_ADD_BUTTON("Sort", "Sort", PF_ParamFlag_SUPERVISE, NULL, SORT_BUTTON);
  PF_ADD_SLIDER("Sort Range Booster", 5, 350, 5, 350, 75, SORT_LENGTH_BOOSTER_SLIDER);
  PF_ADD_SLIDER("Minimum Sort Length", 5, 200, 5, 200, 5, MIN_SORT_LENGTH_SLIDER);
  PF_ADD_SLIDER("Sort Width", 1, 50, 1, 100, 20, SORT_WIDTH_SLIDER);
  
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
    *inP = siP->pixelMap[xL][yL];
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





inline int getMinSortLength(int rgbValue, ShiftInfo* shiftInfo)
{
  int divider = shiftInfo->sortRangeBoosterSliderValue*2;
  int multiplier = shiftInfo->minSortLengthSliderValue;
  return static_cast<int>(pow(rgbValue/(divider*multiplier), 2));
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
  using highestPixelValueQueue = std::priority_queue<int, std::vector<int>, queueCompare>;
  using iteratorVector         = std::vector<std::vector<PF_Pixel>::iterator>;


  int sortSteps2    = 20;
  int minSortLength = shiftInfo->minSortLengthSliderValue;
  int sortSteps     = shiftInfo->sortRangeBoosterSliderValue;
  int sortWidth     = shiftInfo->sortWidthSliderValue;
  
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
        sortSteps2 = getMinSortLength(mostQueue.top(), shiftInfo);
      }
      else
      {                               
        if (std::abs(mostQueue.top()-getColumnPixelAverage(i, j, shiftInfo)) > sortSteps
           /*sortSteps2*/ || j == shiftInfo->in_data.height - 1) 
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
          //sortSteps2 = getMinSortLength(mostQueue.top(), shiftInfo);                
        }
      }
    }           
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
  A_long          linesL     = 0;
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
  
  suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, 
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
          NULL,
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
          NULL,
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
        extra->cb->checkin_layer_pixels(in_data->effect_ref, INPUT_ID);        
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

  return err;
}





static PF_Err
PreRender(PF_InData         *in_data,
          PF_OutData        *out_data,
          PF_PreRenderExtra *extra)
{

  PF_Err            err = PF_Err_NONE;
  PF_ParamDef       sort_slider_param; 
  PF_ParamDef       min_sort_slider; 
  PF_ParamDef       shift_sort_button;
  PF_ParamDef       sort_width_slider;
  PF_RenderRequest  req = extra->input->output_request;
  PF_CheckoutResult in_result;
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  PF_Handle         infoH = suites.HandleSuite1()->host_new_handle(sizeof(ShiftInfo));
    
  
  shift_sort_button.param_type = PF_Param_BUTTON;
  shift_sort_button.flags = PF_ParamFlag_SUPERVISE;
  


  if (infoH)
  {    
    ShiftInfo *infoP = reinterpret_cast<ShiftInfo*>(
      suites.HandleSuite1()->host_lock_handle(infoH));
    
    
    if (infoP)
    {
      extra->output->pre_render_data = infoH;
      
      AEFX_CLR_STRUCT(sort_slider_param);
      AEFX_CLR_STRUCT(min_sort_slider);
      AEFX_CLR_STRUCT(sort_width_slider);
      AEFX_CLR_STRUCT(shift_sort_button);
      
      
      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_BUTTON,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &shift_sort_button));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_LENGTH_BOOSTER_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &sort_slider_param));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        MIN_SORT_LENGTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &min_sort_slider));

      ERR(PF_CHECKOUT_PARAM(
        in_data, 
        SORT_WIDTH_SLIDER,
        in_data->current_time,
        in_data->time_step, 
        in_data->time_scale,
        &sort_width_slider));

      

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
          infoP->sortRangeBoosterSliderValue = sort_slider_param.u.fd.value;
          infoP->minSortLengthSliderValue    = min_sort_slider.u.fd.value;
          infoP->sortWidthSliderValue        = sort_width_slider.u.fd.value;
          infoP->sortButton                  = shift_sort_button;

          
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

