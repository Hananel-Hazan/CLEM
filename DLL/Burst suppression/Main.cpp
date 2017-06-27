#include "Main.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <iostream>
#include <fstream>

UINT64 Number_of_windows, Window_Size_in_ms, Refactory_time_in_ms, Refactory_time_end,
Minimum_Spikes_Activity, Iteration_to_stimulate, Minimum_elec_active;
UINT64 Window_Size_in_Iteration, Size_of_Time_Window_to_check, Stimulate_counter, Refactory_time, Spike_size_in_Iteration;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// Get rid of name mangling
extern "C"
{

	EXPORT void DLL_Init(CoreFunc_Global_var *corefunc_global_var, char* str){
		Global_Var = corefunc_global_var;
		Global_Var->DLL_Slow_Process_Running_Freq = 1.0; // Slow process running speed
	}

	EXPORT void DLL_Engage(int argc, char** argv, char* str){
		// Init Parameters
		Number_of_windows = 2;
		Window_Size_in_ms = 7;
		Minimum_Spikes_Activity = 10;
		Minimum_elec_active = 3;
		Iteration_to_stimulate = 1;
		Refactory_time_in_ms = 120;
		//---------------
		Stimulate_counter = Iteration_to_stimulate;
		Window_Size_in_Iteration = UINT64(std::round(Window_Size_in_ms * (Global_Var->Input_Hz / 1000)));
		Refactory_time = UINT64(std::round(Refactory_time_in_ms * (Global_Var->Input_Hz / 1000)));
		Size_of_Time_Window_to_check = Window_Size_in_Iteration * Number_of_windows;
	}

	EXPORT void DLL_Disengage(char* str){
	}

	EXPORT void RealTime_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
	{
		// silent digital output when the iteration finish
		if (Stimulate_counter < Iteration_to_stimulate)
			Stimulate_counter++;
		else
			*D_Out = 0;
		
		// refactory time ater burst detection
		if (Refactory_time_end > Global_Var->Time_Stamp_Counter) return;


		UINT64 Current_Time = Global_Var->Time_Stamp_Counter,
			Start_of_check_Time = Current_Time - Size_of_Time_Window_to_check,
			*wcounter = new UINT64[Number_of_windows],
			Spike_Location_of_Last_Event = (Global_Var->Location_of_Last_Event_Analog_Spike == 0) ?
			Global_Var->Size_Of_Events_In_Memory-1 : Global_Var->Location_of_Last_Event_Analog_Spike - 1,
			elec_activity_count[64];
		std::uninitialized_fill(elec_activity_count, elec_activity_count + 64, 0);
		std::uninitialized_fill(wcounter, wcounter + Number_of_windows, 0);

		// empty list
		if (Global_Var->Analog_Spikes_Events[Spike_Location_of_Last_Event].timestamp == 0) return;

		do{
			if (Start_of_check_Time >= Global_Var->Analog_Spikes_Events[Spike_Location_of_Last_Event].timestamp) break;

			UINT64 p = (Current_Time - (Global_Var->Analog_Spikes_Events[Spike_Location_of_Last_Event].timestamp)) / Window_Size_in_Iteration;
			wcounter[p]++;

			elec_activity_count[Global_Var->Analog_Spikes_Events[Spike_Location_of_Last_Event].channel]++;


			if (Spike_Location_of_Last_Event == 0)
				Spike_Location_of_Last_Event = Global_Var->Size_Of_Events_In_Memory;
			else
				Spike_Location_of_Last_Event--;

		} while (true);

		// check minimum spiks in the last window
		bool flag = (wcounter[Number_of_windows - 1] > Minimum_Spikes_Activity) ? true : false;
		
		// output activity counter to the user graph 
		U_out[0] = wcounter[Number_of_windows - 1] / 100.0f;

		for (size_t i = 1; i < Number_of_windows; i++)
			flag = flag && ((wcounter[i] > wcounter[i - 1] * 1.5) ? true : false);

		// check if the minimum electrods activity count is above the minimum define in the parameters
		UINT64 counter = 0;
		for (size_t i = 0; i < 64; i++)
			counter += elec_activity_count[i];
		flag = flag && (counter >= Minimum_elec_active) ? true : false;

		// output activated electrod counter to the user graph 
		U_out[1] = counter / 60.0f;

		if (flag && Stimulate_counter == Iteration_to_stimulate){
			*D_Out = 1;
			Stimulate_counter = 0;
			Refactory_time_end = Global_Var->Time_Stamp_Counter + Refactory_time;
		}

		delete[] wcounter;

	}


	EXPORT void SlowPeriodic_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
	{


	}
}