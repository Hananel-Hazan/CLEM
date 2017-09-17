#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "GlobalVar.h"	

#define EXPORT __declspec(dllexport)


/* some definitions */

#define		CHAINLEN					5
#define		TRIGGER_DURATION_MSEC		5
#define		REFRACTORY_PERIOD_MSEC		1000
#define		FULLRANGE					1000

/* DLL-wide global variables */

CoreFunc_Global_var		*Global_Var;			// pointer to all global variables in CLEM
int						ElectrodeChain[CHAINLEN];
__int64					MaxInterval, TrigDuration, RefractoryPeriod;
unsigned __int64		OneMinuteInSamples;


BOOL APIENTRY 
DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
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

EXPORT void 
DLL_Init(CoreFunc_Global_var *corefunc_global_var, char* str)
{
	Global_Var = corefunc_global_var;
	Global_Var->DLL_Slow_Process_Running_Freq = 1.0; // Slow process call frequency
	
	// Write initialization code here

	strcpy_s(str, MAXTEXTMESSAGELEN, "Initialized OK\n");
	
}

EXPORT void 
DLL_Engage(int argc, char** argv, char* str)
{
	int			i;

	/* verify a valid number of arguments */
	
	if (argc != CHAINLEN + 1)
	{
		str[0] = 0;
		strcpy_s (str, MAXTEXTMESSAGELEN," arguments: max_interval first_electrode1 .. lastelectrode\n");
		return;
	}

	/* get maximal interval */

	MaxInterval = atoi(argv[0]);

	/* convert to samples */

	MaxInterval = (__int64) (((double) MaxInterval * Global_Var->Input_Hz) / 1000.0);

	/* get electrode numbers and store in chain (internally these are zero based, hence the "-1") */

	for (i = 1; i < argc; ++i)
		ElectrodeChain[i - 1] = atoi(argv[i]) - 1;

	/* pre-calculate a few time intervals */

	TrigDuration = (__int64)(((double)TRIGGER_DURATION_MSEC * Global_Var->Input_Hz) / 1000.0);
	RefractoryPeriod = (__int64)(((double)REFRACTORY_PERIOD_MSEC * Global_Var->Input_Hz) / 1000.0);
	OneMinuteInSamples = (__int64)(60 * Global_Var->Input_Hz);

	strcpy_s(str, MAXTEXTMESSAGELEN, "Plugin Engaged Successfully\n");
}


EXPORT void DLL_Disengage(char* str)
{
	// Write your post-closed loop run code here

	strcpy_s(str, MAXTEXTMESSAGELEN, "Plugin Disengaged Successfully\n");
}


EXPORT void RealTime_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
{
	static __int64	last_end_of_chain, trigger_start_sample;
	static int		trigger_in_progress;
	__int64			currentspike, lastADsample, timestamp_now, timestamp_next, delta_samples, i;
	int				n, chainlength;
	
	i = Global_Var->Spike_count;
	if (i == 0)
		return;

	/* get the most recent AD sample timestamp. Data is stored in round robin fashion, so wraparound is possible */

	lastADsample = (Global_Var->Location_of_Last_Value_of_RAW_Analog_Input - 1 + Global_Var->Samples_per_Channel) % Global_Var->Samples_per_Channel;
	timestamp_now = Global_Var->Time_Stamp_Counter_Array[lastADsample];

	/* if a trigger is in progress and has been so for more than a predefined time, reset the appropriate DIO line to 0 */

	if (trigger_in_progress)
	{
		if ((timestamp_now - trigger_start_sample) > TrigDuration)
		{
			*D_Out = 0x0000;
			trigger_in_progress = 0;
		}
	}
		
	/* get the last entry in the Time-Electrode table. Here too, data is stored in round robin fashion, so wraparound is possible */

	currentspike = Global_Var->Location_of_Last_Event_Analog_Spike - 1;
	if (currentspike < 0)
		currentspike = Global_Var->Size_Of_Events_In_Memory - 1;

	/* start by searching for a spike recorded from the last electrode in the chain */

	delta_samples = 0;
	chainlength = 0;
	while (i > 0 && chainlength == 0)
	{
		/* how long ago did the spike occur? if too long ago, stop here */

		delta_samples = timestamp_now - Global_Var->Analog_Spikes_Events[currentspike].timestamp;
		if (delta_samples > 1000) // arbitrary number
			return;
		
		/* was the spike recorded from the last electrode on the chain? */

		if (Global_Var->Analog_Spikes_Events[currentspike].channel != ElectrodeChain[CHAINLEN - 1])
		{
			/* no - continue the search */

			currentspike--;
			if (currentspike < 0)
				currentspike = Global_Var->Size_Of_Events_In_Memory - 1;
			i--;
		}
		else
		{
			/* yes.
			   First check that this spike was not found in a previous iteration (to save some time). if so, stop here */

			if (currentspike == last_end_of_chain) 
				return;

			/* a valid end of chain event was found. break out to search for ther rest ofthe chain */

			chainlength = 1;
			last_end_of_chain = currentspike;
			timestamp_next = Global_Var->Analog_Spikes_Events[currentspike].timestamp;
		}
	} 

	/* move backward through the chain and see if the rest of it occured */
	
	for (n = CHAINLEN - 2; n >= 0; --n)
	{
		while (i > 0)
		{
			/* is the time interval still within limits? if not, stop here */

			if ((timestamp_next - (__int64) Global_Var->Analog_Spikes_Events[currentspike].timestamp) > MaxInterval)
				return;

			/* was the spike recorded from the current electrode in the chain? */

			if (Global_Var->Analog_Spikes_Events[currentspike].channel != ElectrodeChain[n])
			{
				/* no - continue the search */

				currentspike--;
				if (currentspike < 0)
					currentspike = Global_Var->Size_Of_Events_In_Memory - 1;
				i--;
			}
			else
			{
				/* yes: the next event in the chain was found. store and move on */

				timestamp_next = Global_Var->Analog_Spikes_Events[currentspike].timestamp;
				chainlength++;
				break;
			}
		} 
	}

	/* if the full chain was detetected, trigger an external device */

	if (chainlength == CHAINLEN)
	{
		/* trigger an external device using line 0 of the DIO port 
		   note that an inter-trigger refractory period is imposed */
		
		if (!trigger_in_progress && (timestamp_now - trigger_start_sample) > RefractoryPeriod)
		{
			*D_Out = 0x0001;
			trigger_in_progress = 1;
			trigger_start_sample = timestamp_now;
		}
	}
}

EXPORT void SlowPeriodic_CL(double* A_out, float *U_out, unsigned short *D_Out, char* str)
{
	__int64				currentspike, lastADsample, i;
	unsigned __int64	timestamp_now, first_timestamp;
	int					n;
	float				spk_per_min;

	i = Global_Var->Spike_count;
	if (i == 0)
		return;

	/* get the most recent AD sample timestamp. Data is stored in round robin fashion, so wraparound is possible */

	lastADsample = (Global_Var->Location_of_Last_Value_of_RAW_Analog_Input - 1 + Global_Var->Samples_per_Channel) % Global_Var->Samples_per_Channel;
	timestamp_now = Global_Var->Time_Stamp_Counter_Array[lastADsample];
	if (timestamp_now < OneMinuteInSamples)
		first_timestamp = 0;
	else
		first_timestamp = timestamp_now - OneMinuteInSamples;

	/* get the last entry in the Time-Electrode table. Here too, data is stored in round robin fashion, so wraparound is possible */

	currentspike = Global_Var->Location_of_Last_Event_Analog_Spike - 1;
	if (currentspike < 0)
		currentspike = Global_Var->Size_Of_Events_In_Memory - 1;

	/* count spikes during last minute */

	n = 1;
	while (i > 0)
	{
		if (Global_Var->Analog_Spikes_Events[currentspike].timestamp <= first_timestamp)
			break;
		n++;
		currentspike--;
		if (currentspike < 0)
			currentspike = Global_Var->Size_Of_Events_In_Memory - 1;
		i--;
	}
	
	spk_per_min = (float) n / (float) FULLRANGE;
	if (spk_per_min > 1)
		spk_per_min = 1.0;

	/* use full range of display */

	U_out[0] = 2 * spk_per_min - 1;
}

