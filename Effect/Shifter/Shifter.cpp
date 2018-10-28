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
#include "PixelSorter.h"



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

  AEGP_SuiteHandler  suites(in_data->pica_basicP);
  suites.UtilitySuite6()->AEGP_RegisterWithAEGP(NULL, "Shifter", &pluginID);

  out_data->my_version = PF_VERSION(MAJOR_VERSION, MINOR_VERSION,
                                    BUG_VERSION, STAGE_VERSION, BUILD_VERSION);

  out_data->out_flags =
    PF_OutFlag_CUSTOM_UI | PF_OutFlag_USE_OUTPUT_EXTENT | PF_OutFlag_REFRESH_UI |
    PF_OutFlag_FORCE_RERENDER | PF_OutFlag_I_USE_SHUTTER_ANGLE | PF_OutFlag_WIDE_TIME_INPUT |
    PF_OutFlag_SEND_UPDATE_PARAMS_UI | PF_OutFlag_REFRESH_UI;

  out_data->out_flags2 =
    PF_OutFlag2_SUPPORTS_SMART_RENDER | PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS |
    PF_OutFlag2_DOESNT_NEED_EMPTY_PIXELS | PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT;

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
  PF_ADD_FLOAT_SLIDER("Variable Range", 0.01, 5, 0.01, 3, NULL, 1, PF_Precision_TENTHS, NULL, NULL, VARIABLE_SLIDER);
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

  
  
  register SorterBase *siP = reinterpret_cast<SorterBase*>(refcon);
  PF_Err err = PF_Err_NONE;  


  if (siP->mapCreated) { 
    *outP = siP->pixelMap[xL][yL].pixel; 
  }
  
  else { 

    switch (siP->params[SORT_BY_DROPDOWN].u.pd.value) {

      case HUE:
        reinterpret_cast<const PF_ColorCallbacks*>(siP->colorSuite)->
          Hue(siP->ref, inP,
              &(siP->pixelMap[xL][yL].value));
        break;

      case SATURATION:
        reinterpret_cast<const PF_ColorCallbacks*>(siP->colorSuite)->
          Saturation(siP->ref, inP,
                     &(siP->pixelMap[xL][yL].value));
        break;

      case LIGHTNESS:
        reinterpret_cast<const PF_ColorCallbacks*>(siP->colorSuite)->
          Lightness(siP->ref, inP,
                    &(siP->pixelMap[xL][yL].value));
        break;

      case LUMINANCE:
        reinterpret_cast<const PF_ColorCallbacks*>(siP->colorSuite)->
          Luminance(siP->ref, inP,
                    &(siP->pixelMap[xL][yL].value));
        break;

      default:break;
    }
    
    siP->pixelMap[xL][yL](*inP);
  }

  return err;
}










static PF_Err SmartRender(PF_InData* in_data, PF_OutData* out_data,
                          PF_ParamDef* params[], PF_SmartRenderExtra* extra) {


  PF_Err             err  = PF_Err_NONE;
  PF_Err             err2 = PF_Err_NONE;
  AEGP_SuiteHandler  suites(in_data->pica_basicP);
  PF_EffectWorld*    input_worldP = NULL;
  PF_EffectWorld*    output_worldP = NULL;
  PF_Handle          infoH = suites.HandleSuite1()->host_new_handle(sizeof(SorterBase));



  SorterBase *infoP{ reinterpret_cast<SorterBase*>(suites.HandleSuite1()->
    host_lock_handle(reinterpret_cast<PF_Handle>(extra->input->pre_render_data)))};

  AEFX_SuiteScoper<PF_Iterate8Suite1> iterSuite{ in_data,kPFIterate8Suite,
                                                 kPFIterate8SuiteVersion1,out_data };

  if (infoP) {
    if (!infoP->no_opB) {
      // checkout input & output buffers.
      ERR((extra->cb->checkout_layer_pixels(in_data->effect_ref,
                                            SORT_INPUT, &input_worldP)));

      ERR(extra->cb->checkout_output(in_data->effect_ref, &output_worldP));


      if (!err && output_worldP) {
        infoP->ref = in_data->effect_ref;
        
        infoP->in_data.reset(in_data);
        infoP->out_data.reset(out_data);
        infoP->setupParams();


        ERR(iterSuite->iterate(in_data, 0, output_worldP->height, input_worldP,
              &input_worldP->extent_hint, (void*)(infoP),SortImagePixels, output_worldP));

        infoP->mapCreated = true;
        infoP->sortPixelMap();


        ERR(iterSuite->iterate(in_data, 0, output_worldP->height,
              input_worldP, &output_worldP->extent_hint,(void*)(infoP), 
                SortImagePixels, output_worldP));


        infoP->mapCreated = false;
      }
    } else { ERR(PF_COPY(input_worldP, output_worldP, NULL, NULL)); }
  } else { err = PF_Err_BAD_CALLBACK_PARAM; }


  suites.HandleSuite1()->host_dispose_handle(reinterpret_cast<PF_Handle>(
    extra->input->pre_render_data));

  ERR(extra->cb->checkin_layer_pixels(in_data->effect_ref, SORT_INPUT));

  return err;
}










static PF_Err PreRender(PF_InData* in_data,PF_OutData* out_data,
                        PF_ParamDef *params[], PF_PreRenderExtra* extra) {

  PF_Err            err = PF_Err_NONE;
  PF_RenderRequest  req = extra->input->output_request;
  PF_CheckoutResult in_result;
  AEGP_SuiteHandler suites(in_data->pica_basicP);
  PF_Handle         infoH = suites.HandleSuite1()->host_new_handle(sizeof(SorterBase));


  if (infoH) { SorterBase *infoP = reinterpret_cast<SorterBase*>(
    suites.HandleSuite1()->host_lock_handle(infoH));

    AEFX_CLR_STRUCT(*infoP);

    if (infoP) {
      extra->output->pre_render_data = infoH;

      for (int i = 0; i < SORT_NUM_PARAMS; ++i) {
        PF_CHECKOUT_PARAM(in_data, i, in_data->current_time, in_data->time_step, 
                          in_data->time_scale, &infoP->params[i]);
      }

      if (!err) {

        req.field = PF_Field_FRAME;

        
                
        ERR(extra->cb->checkout_layer(in_data->effect_ref, SORT_INPUT, SORT_INPUT,
                                      &req, in_data->current_time, in_data->local_time_step,
                                      in_data->time_scale, &in_result));        


        if (!err) {
          UnionLRect(&in_result.result_rect, &extra->output->result_rect);
          UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);
        }
      }
      suites.HandleSuite1()->host_unlock_handle(infoH);      

    } else { err = PF_Err_OUT_OF_MEMORY; }
  } else { err = PF_Err_OUT_OF_MEMORY; }
  return err;
}















DllExport PF_Err EntryPointFunc(PF_Cmd cmd,PF_InData *in_data, PF_OutData *out_data, 
                                PF_ParamDef *params[], PF_LayerDef *output,void *extraP) {
  PF_Err err = PF_Err_NONE;
  AEGP_SuiteHandler suite{ in_data->pica_basicP };

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
        err = PreRender(in_data, out_data, params,
                        reinterpret_cast<PF_PreRenderExtra*>(extraP));          
        break;

      case PF_Cmd_SMART_RENDER:
        err = SmartRender(in_data, out_data, params,
                          reinterpret_cast<PF_SmartRenderExtra*>(extraP));

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











