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
#include <fstream>
#include "PixelStruct.h"
#include "SortStruct.h"
#include "quicksortFunc.h"




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





enum UserParameters {
  SORT_INPUT = 0, MAIN_GROUP_START, SORT_METHOD_DROPDOWN,
  SORT_BY_DROPDOWN, SORT_BY_COLOR_DROPDOWN, ORIENTAION_DROPDOWN,
  REVERSE_SORT_CHECKBOX, SORT_VALUE_RANGE, SORT_WIDTH_SLIDER,
  MAIN_GROUP_END, VARIABLE_SORT_GROUP_START, VARIABLE_SORT_CHECKBOX,
  VARIABLE_SLIDER, FAVOR_DARK_RANGES, VARIABLE_SORT_GROUP_END,
  MISC_GROUP_END, MANUAL_SORT_RANGE_GROUP_START, HIGH_RANGE_SORT_LIMIT,
  LOW_RANGE_SORT_LIMIT, MANUAL_SORT_RANGE_GROUP_END, MISC_GROUP_START,
  MIN_SORT_LENGTH_SLIDER, MIN_SORT_RAND_SLIDER, MIN_REVERSE_DIST_SLIDER,
  PIXEL_INTERPOLATION_CHECKBOX, SORT_NUM_PARAMS
};

enum SortOrientations { VERTICAL_ORIENTATION = 1, HORIZONTAL_ORIENTATION };
enum SortMethods { BASIC_SORT = 1, MANUAL_SORT };
enum SortByMenuOptions { SORT_BY_LUMINOSITY = 1, SORT_BY_RGB };
enum SortByColorOptions { RED_SORT = 1, GREEN_SORT, BLUE_SORT };
enum PixelProperties { HUE = 1, LUMINANCE, SATURATION, LIGHTNESS };
enum { SLIDER_ID, INPUT_ID };



typedef SortStruct* SortStructPtr;
typedef PixelStruct* PixelStructPtr;
//SortStructPtr sortStruct;


SPBasicSuite* _suite;
const void* colorSuite;
AEGP_PluginID pluginID;
PF_ParamDef params[SORT_NUM_PARAMS]{};

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
