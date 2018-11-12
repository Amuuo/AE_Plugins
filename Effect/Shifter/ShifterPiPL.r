#include "AEConfig.h"
#include "AE_EffectVers.h"
#include "AE_General.r"

resource 'PiPL' (16000) {
	{	/* array properties: 12 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
			"Shifter"
		},
		/* [3] */
		Category {
			"Sample Plug-ins"
		},

		CodeWin64X86 {"EntryPointFunc"},
		
		/* [6] */
		AE_PiPL_Version {
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			PF_PLUG_IN_VERSION,
			PF_PLUG_IN_SUBVERS
		},
		/* [8] */
		AE_Effect_Version {
			1048577	/* 2.0 */
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
			0x044a8042
		},
		AE_Effect_Global_OutFlags_2 {
			0x00020441
		},
		/* [11] */
		AE_Effect_Match_Name {
			"ADBEShifter"
		},
		/* [12] */
		AE_Reserved_Info {
			8
		}
	}
};

