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
  Main.cpp
*/

#include "Shifter.h"
#include "quicksortFunc.cpp"


static PF_Err About(PF_InData *in_data, PF_OutData *out_data, 
                    PF_ParamDef *params[], PF_LayerDef *output) {
  
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



static PF_Err GlobalSetup(PF_InData *in_data, PF_OutData *out_data,
                          PF_ParamDef *params[], PF_LayerDef *output) {

  AEGP_SuiteHandler suites(in_data->pica_basicP);
   
  
  suites.UtilitySuite6()->AEGP_RegisterWithAEGP(NULL, "Shifter", &pluginID);

  out_data->my_version = PF_VERSION(MAJOR_VERSION, MINOR_VERSION,
                                    BUG_VERSION, STAGE_VERSION, BUILD_VERSION);

  out_data->out_flags = PF_OutFlag_USE_OUTPUT_EXTENT| 
                        PF_OutFlag_WIDE_TIME_INPUT;

  out_data->out_flags2 = PF_OutFlag2_SUPPORTS_SMART_RENDER | 
                         PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS | 
                         PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT;

  
  return PF_Err_NONE;
}



static PF_Err ParamsSetup(PF_InData *in_data,PF_OutData *out_data, 
                          PF_ParamDef *params[], PF_LayerDef *output) {
  
  PF_Err      err = PF_Err_NONE;
  PF_ParamDef def{};

  PF_ADD_TOPIC("Main", MAIN_GROUP_START);
  PF_ADD_POPUP("Sort Method", 2, 0, "Basic Sort|Manual Sort", SORT_METHOD_DROPDOWN);
  PF_ADD_POPUP("Sort By:", 2, 0, "Luminance|Hue|Saturation|Lightness", SORT_BY_DROPDOWN);
  PF_ADD_POPUP("Sort By Color", 3, 0, "Red|Green|Blue", SORT_BY_COLOR_DROPDOWN);
  PF_ADD_POPUP("Sort Orientation", 2, 0, "Vertical|Horizontal", ORIENTAION_DROPDOWN);
  PF_ADD_CHECKBOX("Invert Sort", "Enabled", 0, NULL, REVERSE_SORT_CHECKBOX);
  PF_ADD_SLIDER("Sort Value Range", 1, 1000, 1, 765, 100, SORT_VALUE_RANGE);
  PF_ADD_SLIDER("Sort Width", 1, 1000, 1, 200, 5, SORT_WIDTH_SLIDER);
  PF_END_TOPIC(MAIN_GROUP_END);

  PF_ADD_TOPIC("Variable Sort", VARIABLE_SORT_GROUP_START);
  PF_ADD_CHECKBOX("Variable Sort", "Enabled", 0, NULL, VARIABLE_SORT_CHECKBOX);
  PF_ADD_FLOAT_SLIDER("Variable Range", 0.5, 5, 0.5, 5, NULL, 1, PF_Precision_TENTHS, NULL, NULL, VARIABLE_SLIDER);
  PF_ADD_CHECKBOX("Favor Dark Ranges", "Enabled", 0, NULL, FAVOR_DARK_RANGES);
  PF_END_TOPIC(VARIABLE_SORT_GROUP_END);

  PF_ADD_TOPIC("Manual Sort Range", MANUAL_SORT_RANGE_GROUP_START);
  PF_ADD_SLIDER("High Range Sort Limit", 1, 1000, 1, 765, 500, HIGH_RANGE_SORT_LIMIT);
  PF_ADD_SLIDER("Low Range Sort Limit", 1, 1000, 1, 765, 350, LOW_RANGE_SORT_LIMIT);
  PF_END_TOPIC(MANUAL_SORT_RANGE_GROUP_END);


  PF_ADD_TOPIC("Misc", MISC_GROUP_START);
  PF_ADD_SLIDER("Minimum Sort Length", 1, 1000, 1, 200, 1, MIN_SORT_LENGTH_SLIDER);
  PF_ADD_SLIDER("Minimum Sort Random", 1, 1000, 1, 200, 1, MIN_SORT_RAND_SLIDER);
  PF_ADD_SLIDER("Minimum Reverse Sort Distance", 0, 300, 0, 300, 0,  MIN_REVERSE_DIST_SLIDER);
  PF_ADD_CHECKBOX("Interpolate Pixel Ranges", "Enabled", 0, NULL, PIXEL_INTERPOLATION_CHECKBOX);
  PF_END_TOPIC(MISC_GROUP_END);

  out_data->num_params = SORT_NUM_PARAMS;


  return err;
}



static PF_Err RespondtoAEGP(PF_InData *in_data, PF_OutData *out_data,
                            PF_ParamDef *params[], PF_LayerDef  *output, void* extraP)

{
  PF_Err err = PF_Err_NONE;

  AEGP_SuiteHandler suites(in_data->pica_basicP);

  suites.ANSICallbacksSuite1()->sprintf(
    out_data->return_msg,
    "%s",
    reinterpret_cast<A_char*>(extraP));

  return err;
}











static PF_Err SortImagePixels(void *refcon, A_long xL, A_long yL,
                          PF_Pixel *inP, PF_Pixel *outP) {

  
  
  register SortStruct *siP = reinterpret_cast<SortStruct*>(refcon);
  PF_Err err = PF_Err_NONE;  


  if (siP->mapCreated) { 
    *outP = siP->pixArray[xL][yL]->pixel; 
  }
  
  else {
    switch (LIGHTNESS) {

      case HUE: siP->in_data->utils->colorCB.Hue(
          siP->in_data->effect_ref, inP, &(siP->pixArray[xL][yL].value)); break;

      case SATURATION: siP->in_data->utils->colorCB.Saturation(
          siP->in_data->effect_ref, inP, &(siP->pixArray[xL][yL].value)); break;

      case LIGHTNESS: siP->in_data->utils->colorCB.Lightness(
          siP->in_data->effect_ref, inP, &(siP->pixArray[xL][yL].value)); break;

      case LUMINANCE: siP->in_data->utils->colorCB.Luminance(
          siP->in_data->effect_ref, inP, &(siP->pixArray[xL][yL].value)); break;
    }
    siP->pixArray[xL][yL].set(*inP);
  }

  return err;
}




static PF_Err Render(PF_InData *in_data, PF_OutData *out_data, 
                     PF_ParamDef *params[], PF_LayerDef *output) {


  SortStruct      si;
  PF_Err          err = PF_Err_NONE;
  PF_Fixed        sortLength = params[SORT_VALUE_RANGE]->u.fd.value;
  A_long          linesL = in_data->height;
  PF_EffectWorld  *inputP = &params[SORT_INPUT]->u.ld;


  AEFX_CLR_STRUCT(si);


  AEGP_SuiteHandler  suites(in_data->pica_basicP);

  // iterate() checks for user interruptions.

  ERR(PF_BLEND(output, inputP, sortLength, output));

  return err;
}





static PF_Err SmartRender(PF_InData* in_data, PF_OutData* out_data, PF_SmartRenderExtra* extra) {


  PF_Err             err = PF_Err_NONE;
  PF_Err             err2 = PF_Err_NONE;
  AEGP_SuiteHandler  suites(in_data->pica_basicP);
  PF_EffectWorld*    input_worldP = NULL;
  PF_EffectWorld*    output_worldP = NULL;
  PF_EffectWorld*    sorterWorldP = NULL;




  SortStructPtr sortStruct = static_cast<SortStruct*>(suites.HandleSuite1()->
    host_lock_handle(static_cast<PF_Handle>(extra->input->pre_render_data)));

  sortStruct->in_data = in_data; 


  if(sortStruct->no_opB) {

    ERR((extra->cb->checkout_layer_pixels(in_data->effect_ref,
        SORT_INPUT, &input_worldP)));

    ERR(extra->cb->checkout_output(in_data->effect_ref, &output_worldP));
    
    //sortStruct->pixArray = new PixelStruct[sizeof(sorterWorldP->data) / pixelSize];

    if(!err && output_worldP) {


      ERR(suites.Iterate8Suite1()->iterate(in_data, 0, output_worldP->height,
          input_worldP, &input_worldP->extent_hint,
          (void*)(sortStruct), SortImagePixels, output_worldP));


      for (auto& p : *sortStruct->pixArray) {
        quickSort<PixelStruct>(p->size(), p);
      }
      sortStruct->mapCreated = true;

      ERR(suites.Iterate8Suite1()->iterate(in_data, 0, output_worldP->height,
          input_worldP, &output_worldP->extent_hint, (void*)(sortStruct),
          SortImagePixels, output_worldP));

      suites.HandleSuite1()->host_unlock_handle(
        static_cast<PF_Handle>(extra->input->pre_render_data));

    }
    else {
      ERR(PF_COPY(input_worldP, output_worldP, NULL, NULL));
    }
  }  

  extra->cb->checkin_layer_pixels(in_data->effect_ref, SORT_INPUT);


  return err;
}






static PF_Err PreRender(PF_InData* in_data,PF_OutData* out_data, PF_PreRenderExtra* extra) {

  PF_Err            err = PF_Err_NONE;  
  PF_CheckoutResult in_result;
  PF_RenderRequest  ref = extra->input->output_request;
  AEGP_SuiteHandler suites(in_data->pica_basicP);  
  PF_Handle         infoH;
  

  const PF_Fixed boolSize      = sizeof(PF_Boolean);
  const PF_Fixed pixStructSize = sizeof(PixelStruct);
  const PF_Fixed inDataSize    = sizeof(PF_InData);


  /*
  PF_Fixed sizeOfPixArray = (in_data->height * in_data->width * 
                             pixStructSize) + inDataSize + boolSize;
  */

  infoH = suites.HandleSuite1()->host_new_handle(sizeof(SortStruct));    
  
      
  if (infoH) {
    
    SortStructPtr sortStruct =
      static_cast<SortStructPtr>(suites.HandleSuite1()->host_lock_handle(infoH));

    sortStruct->pixArray = new std::vector<std::vector<PixelStruct>*>;
    sortStruct->pixArray->reserve(in_data->width);
    for (auto& p : *sortStruct->pixArray) {
      p = new std::vector<PixelStruct>;
      p->reserve(in_data->height);
    }
    //AEFX_CLR_STRUCT(*infoP);
    extra->output->pre_render_data = infoH;
    //if (infoP) {          

    if (!err) {                  

      
      

      ref.field = PF_Field_FRAME;
      
      ERR(extra->cb->checkout_layer(in_data->effect_ref, SORT_INPUT, 
          SORT_INPUT, &ref, in_data->current_time, in_data->local_time_step, 
          in_data->time_scale, &in_result));
      
      
      //sortStruct->pixArray = new PixelStruct[(PF_Fixed)(in_data->height*in_data->width)];

      if (!err) {
        UnionLRect(&in_result.result_rect, &extra->output->result_rect);
        UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);
      }

      suites.HandleSuite1()->host_unlock_handle(infoH);

    } else { err = PF_Err_OUT_OF_MEMORY; }
  }
  
  
  
  return err;
}















DllExport PF_Err EntryPointFunc(PF_Cmd cmd,PF_InData *in_data, PF_OutData *out_data, 
                                PF_ParamDef *params[], PF_LayerDef *output,void *extraP) {
  
  PF_Err err = PF_Err_NONE;
  

  try {
    switch (cmd) {
      case PF_Cmd_ABOUT:
        err = About(in_data, out_data, params, output);
        break;

      case PF_Cmd_GLOBAL_SETUP:
        err = GlobalSetup(in_data, out_data, params, output);
        break;

      case PF_Cmd_PARAMS_SETUP:
        err = ParamsSetup(in_data, out_data, params, output);
        break;

      case PF_Cmd_SMART_PRE_RENDER:
        err = PreRender(in_data, out_data,reinterpret_cast<PF_PreRenderExtra*>(extraP));          
        break;

      case PF_Cmd_RENDER:
        err = Render(in_data, out_data, params, output);
        break;
      
      case PF_Cmd_SMART_RENDER:
        err = SmartRender(in_data, out_data,reinterpret_cast<PF_SmartRenderExtra*>(extraP));

        break;

      case PF_Cmd_COMPLETELY_GENERAL:
        err = RespondtoAEGP(in_data, out_data, params, output, extraP);
    }
  }
  catch (PF_Err &thrown_err) {
    err = thrown_err;
  }
    
  return err;
}











