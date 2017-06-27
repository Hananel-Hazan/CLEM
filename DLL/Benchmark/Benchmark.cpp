#include "Benchmark.h"
#include "VRand.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <iostream>
#include <fstream>

CoreFunc_Global_var* Global_Var;			//pointer to all global variables in CLEM

//_Work_load
std::vector <double> time_between_iteration_Work_load;
UINT64 Last_TimeStamp_Work_load;

// Close_Loop
UINT64 No_of_I_Channles_, No_of_I_Channles_minus1, buffer_per_I_channle;
bool outputing;
std::vector <double> time_between_IO;
UINT64 counter_IO;

const UINT64 Channle_of_Rebound_volt = 60;
double inputV;

UINT64 pointer_to_Electrod_data_in_memory;

UINT64 Last_TimeStamp_RAW_V;
bool just_loaded;

// work
VRand r;



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

		No_of_I_Channles_minus1 = Global_Var->Analog_Input_Channels - 1;
		No_of_I_Channles_ = Global_Var->Analog_Input_Channels;
		buffer_per_I_channle = Global_Var->buffer_Input_Per_Channel;
		pointer_to_Electrod_data_in_memory = Global_Var->Pointer_To_Electrode_In_RAW_Input_Volts[Channle_of_Rebound_volt];
		inputV = Global_Var->MAX_V / 2;
		just_loaded = true;
	}

	EXPORT void DLL_Engage(int argc, char** argv, char* str){
		just_loaded = false;

		//-------------
		// Initilazation parameters for the first run
		//====================
		time_between_iteration_Work_load.clear();
		time_between_IO.clear();

		r.seed();
		//-------------

		//_Work_load 
		Last_TimeStamp_Work_load = Global_Var->Time_Stamp_Counter;

		// close loop cheack ==> outputing 5v in Analog Output

		Last_TimeStamp_RAW_V = Global_Var->Location_of_Last_Value_of_RAW_Analog_Input;
		outputing = true;
		counter_IO = 0;
		//-----------
	}

	EXPORT void DLL_Disengage(char* str){
		if (just_loaded) return;

		std::string s;

		UINT32 str_length = 0;

		//_Work_load
		double sum = std::accumulate(time_between_iteration_Work_load.begin(), time_between_iteration_Work_load.end(), 0.0);
		double avg = sum / time_between_iteration_Work_load.size();

		double accum = 0.0;
		std::for_each(std::begin(time_between_iteration_Work_load), std::end(time_between_iteration_Work_load), [&](const double d) {
			double t = (d - avg);
			accum += t * t;
		});
		double std = sqrt(accum / (time_between_iteration_Work_load.size()));

		double time_cons_in_ms = (1 / Global_Var->Input_Hz) * 1000;

		avg = (avg)* time_cons_in_ms;
		std = (std)* time_cons_in_ms;

		s = "DLL Workload =" + std::to_string(time_between_iteration_Work_load.size()) + " iteration, average " + std::to_string(avg) + " ms with std (" + std::to_string(std) + ") ms \r\n";

		//_Close_Loop
		sum = std::accumulate(time_between_IO.begin(), time_between_IO.end(), 0.0);
		avg = sum / time_between_IO.size();

		accum = 0.0;
		std::for_each(std::begin(time_between_IO), std::end(time_between_IO), [&](const double d) {
			double t = (d - avg);
			accum += t * t;
		});
		std = sqrt(accum / (time_between_IO.size()));

		avg = (avg)* time_cons_in_ms;
		std = (std)* time_cons_in_ms;

		s += "lag time = " + std::to_string(time_between_IO.size()) + " times, average " + std::to_string(avg) + " ms with std(" + std::to_string(std) + ") ms";


		// save all resutls to text file
		std::ofstream myfile;
		myfile.open("DLL_Lagtime.txt");
		for (size_t i = 0, end = time_between_IO.size(); i < end; i++)
			myfile << std::to_string(time_between_IO[i]) << "\n";
		myfile.close();


		myfile.open("DLL_Workload.txt");
		for (size_t i = 0, end = time_between_iteration_Work_load.size(); i < end; i++)
			myfile << std::to_string(time_between_iteration_Work_load[i]) << "\n";
		myfile.close();


		strcpy_s(str, MAXTEXTMESSAGELEN, s.c_str());

	}

	EXPORT void RealTime_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
	{
		// check the work load of the DLL, meaning, chaking the time diffrence between each run to 
		double delta_time = (double)(Global_Var->Time_Stamp_Counter - Last_TimeStamp_Work_load);
		time_between_iteration_Work_load.push_back(delta_time);
		Last_TimeStamp_Work_load = Global_Var->Time_Stamp_Counter;

		// physically close loop check  ==> chack if outputing or slince time
		// !! IMPORTENT! connet the analog output 0 to analog input channle number in the Channle_of_Rebound_volt
		if (outputing){
			while (Last_TimeStamp_RAW_V != Global_Var->Location_of_Last_Value_of_RAW_Analog_Input)
			{
				Last_TimeStamp_RAW_V++;
				if (Last_TimeStamp_RAW_V >= Global_Var-> Samples_per_Channel)
					Last_TimeStamp_RAW_V = 0;

				if (Global_Var->RAW_Input_Volts[pointer_to_Electrod_data_in_memory + Last_TimeStamp_RAW_V] >= inputV){
					outputing = false;
					time_between_IO.push_back((double)counter_IO);
					counter_IO = 0;
					A_out[0] = Global_Var->MIN_V;
					break;
				}
				else
					counter_IO++;
			}
		}
		else{
			while (Last_TimeStamp_RAW_V != Global_Var->Location_of_Last_Value_of_RAW_Analog_Input)
			{
				Last_TimeStamp_RAW_V++;
				if (Last_TimeStamp_RAW_V >= Global_Var-> Samples_per_Channel)
					Last_TimeStamp_RAW_V = 0;

				if (Global_Var->RAW_Input_Volts[pointer_to_Electrod_data_in_memory + Last_TimeStamp_RAW_V] <= -inputV){
					outputing = true;
					time_between_IO.push_back((double)counter_IO);
					counter_IO = 0;
					A_out[0] = Global_Var->MAX_V;
					break;
				}
				else
					counter_IO++;
			}
		}

		// Compute somthing to make CPU work hard :-)
		const size_t l = 1000;
		double test[l];
		for (size_t i = 0; i < l; i++)
		{
			test[i] = r.pink() * 1000;
			test[i] = r.brown() * 1000;
			test[i] = r.white() * 1000;
		}

	}


	EXPORT void SlowPeriodic_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
	{

	}
}