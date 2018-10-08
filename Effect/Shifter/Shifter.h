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
#include <queue>
#include <functional>


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
	SHIFT_INPUT = 0,	// default input layer 
  SORT_BUTTON,
  SORT_LENGTH_BOOSTER_SLIDER,
  MIN_SORT_LENGTH_SLIDER,
  MIN_SORT_RAND_SLIDER,
  SORT_WIDTH_SLIDER,
  VARIABLE_SORT_CHECKBOX,
  VARIABLE_SLIDER,
  MIN_REVERSE_DIST_SLIDER,
  REVERSE_SORT_CHECKBOX,
	SHIFT_NUM_PARAMS
};

enum {
	DISPLACE_DISK_ID = 1,
	BLEND_DISK_ID,
	USE_TRANSFORM_DISK_ID,
  BUTTON_ID,
  INPUT_ID
};
#define SHIFT_DISPLACE_X_DFLT	(10L)
#define	SHIFT_DISPLACE_Y_DFLT	(10L)

#define	SHIFT_BLEND_MIN		0.0f
#define	SHIFT_BLEND_MAX		100.0f
#define	SHIFT_BLEND_DFLT	50.0f 

#define RESTRICT_BOUNDS		0
#define SLIDER_PRECISION	2


typedef struct {
  PF_ParamDef    sortSliderParam; 
  PF_ParamDef    minSortSlider; 
  PF_ParamDef    minSortRandomSlider;
  PF_ParamDef    shiftSortButton;
  PF_ParamDef    sortWidthSlider;
  PF_ParamDef    variableSortCheckbox;
  PF_ParamDef    variableSlider;
  PF_ParamDef    minReverseSortSlider;
  PF_ParamDef    reverseSortCheckbox;
  PF_Fixed       sortRangeBoosterSliderValue;
  PF_Fixed       minSortLengthSliderValue;
  PF_Fixed       sortWidthSliderValue;
  PF_Fixed       minSortRandValue;
  float          variableValue;
  PF_ParamDef    sortButton;
  bool           variableSortOn;
  PF_EffectWorld originalCopy;
  PF_EffectWorld inputCopy;
	PF_ProgPtr	   ref;
	PF_SampPB  	   samp_pb;
	PF_InData 	   in_data;
	PF_Boolean	   no_opB;
  bool           mapCreated{false};

  std::vector<std::vector<PF_Pixel>> pixelMap; 

} ShiftInfo;


struct queueCompare 
{
  bool operator()(const int& left, const int& right) 
  {
    return right > left;
  }
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