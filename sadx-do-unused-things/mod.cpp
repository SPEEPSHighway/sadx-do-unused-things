#include "SADXModLoader.h"
#include "FunctionHook.h"
#include <IniFile.hpp>

extern "C"
{	
	//------------------
	//Circuit Act Select Menu
	//------------------

	typedef struct CtrlActParam
	{
		Sint16 xPos;
		Sint16 yPos;
	} CtrlActParam;
	CtrlActParam ctrlparam = { 5, 15 };
	DataPointer(TaskInfo, CartActTaskInfo, 0x7E7578);

	void initActSelect() {
		MirenSetTask(LEV_4, &CartActTaskInfo, &ctrlparam);
	}

	//------------------
	//SS Car Roadkill
	//------------------

	static CCL_INFO car_object_c_info[] = {
		{ 0, CI_FORM_RECTANGLE, 0x77, (char)0xE0, 0, { 13.0f, 3.5f, 0.0f }, 20.0f, 3.5f, 7.5f, 0.0f, 0, 0, 0 },
		{ 0, 1, 3, 0x2C, 0x400, { -4.0f, 3.0f, 0.0f }, 10.0f, 3.0f, 4.0f, 0.0f, 0, 0, 0 },
		{ 0, CI_FORM_RECTANGLE, 0x77, (char)0xE0, 0, { 13.0f, 3.5f, 0.0f }, 20.0f, 3.5f, 7.5f, 0.0f, 0, 0, 0 },
		{ 0, 1, 3, 0x2C, 0x400, { -4.0f, 3.0f, 0.0f }, 10.0f, 3.0f, 4.0f, 0.0f, 0, 0, 0 },
		{ 0, CI_FORM_RECTANGLE, 0x77, (char)0xE0, 0, { 13.0f, 3.5f, 0.0f }, 20.0f, 3.5f, 7.5f, 0.0f, 0, 0, 0 },
		{ 0, 1, 3, 0x2C, 0x400, { -4.0f, 3.0f, 0.0f }, 10.0f, 3.0f, 4.0f, 0.0f, 0, 0, 0 },
		{ 0, CI_FORM_RECTANGLE, 0x77, (char)0xE0, 0, { 13.0f, 3.5f, 0.0f }, 20.0f, 3.5f, 7.5f, 0.0f, 0, 0, 0 },
		{ 0, 1, 3, 0x2C, 0x400, { -4.0f, 3.0f, 0.0f }, 10.0f, 3.0f, 4.0f, 0.0f, 0, 0, 0 }
	};

	Float carspd = 0.1f; //required car speed to hit player
	Float plspd = 0.1f; //required player speed to be hit by car

	void replaceCarHitboxes(task* tp, CCL_INFO* info, int nbInfo, Uint32 id) {
		CCL_Init(tp, car_object_c_info, 2, CID_OBJECT);
	}

	//------------------
	//Kart Pass Animation
	//------------------
	Bool SeqCheck_KartPass() {

		if (ssActNumber == 5) {
			switch (GetPlayerNumber()) {
			case 2:
				return seqVars[FLAG_MILES_SS_ENTRANCE_CIRCUIT];
			case 3:
				return seqVars[FLAG_KNUCKLES_SS_ENTRANCE_CIRCUIT];
			case 6:
				return seqVars[FLAG_E102_SS_ENTRANCE_CIRCUIT];
			default:
				return 0;
			}
		}
		else if (ssActNumber == 3) {
			switch (GetPlayerNumber()) {
			case 2:
				return seqVars[FLAG_MILES_SS_TPARK_ELEVATOR];
			case 3:
				return seqVars[FLAG_KNUCKLES_SS_TPARK_ELEVATOR];
			case 6:
				return seqVars[FLAG_E102_SS_TPARK_ELEVATOR];
			default:
				return 0;
			}
		}
	}

	void doCardAnimation(task* tp) {

		taskwk* twp = tp->twp;

		SetContinue(tp); //Replaced a SetContinue so let's do that first.

		//Check if in position to unlock Twinkle Circuit.
		if (!SeqCheck_KartPass()
			&& ssActNumber == 5
			&& (twp->pos.z - 1740.0f) * (twp->pos.z - 1740.0f) + (twp->pos.x - 757.0f) * (twp->pos.x - 757.0f) < 225.0f)
		{
			twp->mode = 5;
		}

		if (!SeqCheck_KartPass()
			&& ssActNumber == 3
			&& (twp->pos.z - 1607.0) * (twp->pos.z - 1607.0) + (twp->pos.x - 356.0) * (twp->pos.x - 356.0) < 400.0)
		{
			twp->mode = 4;
		}
	}

	//------------------
	//Traffic Lights
	//------------------

	TaskFunc(dispSSSignal, 0x63C400);
	static FunctionHook<void, task*> dispSSSignal_hook(dispSSSignal);

	void doTrafficLights(task* tp) {
		taskwk* twp = tp->twp;

		if (twp->mode) {
			NJS_OBJECT* obj = (NJS_OBJECT*)twp->counter.ptr;
			NJS_MODEL* model = (NJS_MODEL*)obj->model;
			switch (twp->mode) {
			case 1:
				model->mats[6].attrflags |= NJD_FLAG_IGNORE_LIGHT; //Blue, active
				model->mats[7].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Yellow, inactive
				model->mats[8].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Red, inactive
				break;
			case 2:
				model->mats[6].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Blue, inactive
				model->mats[7].attrflags |= NJD_FLAG_IGNORE_LIGHT; //Yellow, active
				model->mats[8].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Red, inactive
				break;
			case 3:
				model->mats[6].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Blue, inactive
				model->mats[7].attrflags &= ~NJD_FLAG_IGNORE_LIGHT; //Yellow, inactive
				model->mats[8].attrflags |= NJD_FLAG_IGNORE_LIGHT; //Red, active
				break;
			}
		}

		dispSSSignal_hook.Original(tp);
	}

	//------------------

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		WriteData<1>((void*)0x79DDF1, 0x7C); //Moves dev act select from P4 to P1.
		WriteData<1>((void*)0x6375E8, 10); //Barricade HP
		WriteData((float**)0x63294E, &plspd); //Make the required speed to be hit by a car in SS lower
		WriteData((float**)0x632910, &carspd); //Make the required speed to be hit by a car in SS lower
		WriteCall((void*)0x6332DD, replaceCarHitboxes); //Add car bump
		dispSSSignal_hook.Hook(doTrafficLights); //Show traffic light colours
		WriteCall((void*)0x637999, doCardAnimation); //Add car bump
	}


	__declspec(dllexport) void __cdecl OnFrame()
	{
		if (GameState == 15 && ssStageNumber == STAGE_MG_CART) {
			if (per[0]->on & Buttons_A && per[0]->press & Buttons_Right) {
				initActSelect();
			}
		}
	}
}