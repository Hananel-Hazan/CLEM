#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "GlobalVar.h"
#ifdef __cplusplus
}
#endif


#ifndef CoreFunction_H
#define CoreFunction_H
#ifdef __cplusplus

#include <iostream>
#include <fstream>
#include <tchar.h>
#include <future>
#include <ratio> 
#include <thread> 
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sstream>
#include <iomanip>



const double OneBitIsEqualTo = 0.000244140625; // +10...-10 => 1 / 4096 = 0.000244140625 , 4096 valuse have in 12 bit

// UEI Defenitions "C:\Program Files (x86)\UEI\Framework\CPP\include\"
#include "C:\Program Files (x86)\UEI\Framework\CPP\include\UeiDaq.h"
#include "C:\Program Files (x86)\UEI\Framework\CPP\include\UeiStructs.h"
#include "C:\Program Files (x86)\UEI\Framework\CPP\include\UeiDevice.h"

class CoreFunction {
private:
	//public:
	static double *calculate_offset_per_electrod, *offset_per_electrod, *backup_offset_per_electrod;
	static double **avg_per_electrod;
	static int* elements_Counter_for_RMS;
	//static double* downgrade_avg;
	//static int* downgrade_counter;
	static unsigned __int64* Threshold_Max_Counter;
	static BOOL* ReCalculate_Threshold_AT_Electrode;
	//static double RMS_Factor;

	static HINSTANCE hInstLibrary;
	static std::chrono::high_resolution_clock::time_point Paused, Continue;
	static std::thread *WorkThread_Analog_Input, *WorkThread_Analog_Output, *WorkThread_Digital_Input, *WorkThread_Digital_Output, *WorkThread_DLL_On_Demond, *WorkThread_DLL_RealTime, *WorkThread_File_Convertor;
	static std::mutex DLL_mutex_RealTime, analog_output_mutex, digital_output_mutex, digital_input_mutex;
	static std::condition_variable DLL_cv_RealTime, analog_output_cv, digital_output_cv, digital_input_cv;

	// DLL Plugin - Function pointers that will be used for the DLL functions.
	typedef void(*ClientIOfun)(double*, float*, unsigned short*, char*);
	static ClientIOfun __ClientIOfun_RealTime_CL, __ClientIOfun_SlowPeriodic_CL;
	typedef void(*iniDLL)(CoreFunc_Global_var*, char*);
	static iniDLL __initDLL;
	typedef void(*ClientIOfun_Engage)(int, char**, char*);
	typedef void(*ClientIOfun_Disengage)(char*);
	static ClientIOfun_Engage __ClientIOfun_DLL_Engage;
	static ClientIOfun_Disengage __ClientIOfun_DLL_Disengage;

	static double *Local_Last_Analog_Data_Out, *Local_Analog_Data_Out;
	static float *Local_Analog_User_Data;
	static unsigned short Local_Digital_Data_Out;


	// DAQ Var
	static UeiDaq::CUeiDevice *DAQ_Device_info;
	static UeiDaq::CUeiSession Analog_InputSession, Analog_OutputSession, Digital_InputSession, Digital_OutputSession;
	static UeiDaq::CUeiAnalogScaledReader *Analog_Reader;
	static UeiDaq::CUeiAnalogScaledWriter *Analog_Writer;
	static UeiDaq::CUeiDigitalReader *Digital_Reader;
	static UeiDaq::CUeiDigitalWriter *Digital_Writer;
	static UeiDaq::CUeiDataStream* pDataOutputStream;
	static unsigned short Digital_Data_In, Digital_Data_Out;
	static double *Data_In_Pointer, *Data_In_A, *Data_In_B;
	static double *Data_Out_Pointer, *Data_Out_A, *Data_Out_B;// , *Data_Out_old_Pointer;//,Data_Out_Deafult;
	static float *Global_Spikes_volt_around_spike, *Global_analog_volt_around_event, *Global_User_Event, *backup_threshold_Per_Electrod;

	// status
	static BOOL loop_analog_input, loop_DLL_RealTime, loop_DLL_On_Demond, loop_analog_output, loop_digital_output, loop_digital_input, Input_Ready_To_Process, DAQ_Initialized;
	static BOOL Input_Flag, Analog_Output_ready, Analog_Output_Flag, init_Analog_Output, Digital_Output_ready, Digital_Input_ready;

	// Recording and Playback
	static FILE *Playback_Input_File, *Spike_Recording_File, *Spike_Time_Recording_File, *Volt_Recording_File;// , *Output_Volt_Recording_File, *Digital_Recording_File;
	static std::ofstream *Output_Volt_Recording_File, *Digital_Recording_File, *UserData_Recording_File;
	static std::string Playback_Path_First, Playback_Filename_First, Playback_Current_Filename, Playback_Current_Path;
	static unsigned __int64 Playback_Counter_First, Playback_Current_Counter, Playback_Current_Until_Counter;
	static int Playback_Current_File_Type;
	static int Playback_File_Type_First;

	// Memory management
	static BOOL  Memory_Initialized;
	static unsigned __int64 Spike_VoltTrace_size;
	static float *Global_analog_volt_OUTPUT_1, *Global_analog_volt_OUTPUT_2;
	typedef struct
	{
		unsigned __int64	Spike_Header_to_continue;
		unsigned __int64	End_of_Envelope;
	} Spike_Ditected;
	static std::list<Spike_Ditected> *Envelope_Counter;
	static unsigned __int64 *Spike_reArm_Counter;
	static unsigned __int64 Spike_reArm_in_iteration;


public:
	// DAQ Var
	static std::string DAQpath, Analog_InputChannels, Analog_OutputChannels;

	// Memory management
	static BOOL Acquisition_paused, PlaybackStarted, Playback_pause, Spike_RecordingStarted, 
		Spike_Time_RecordingStarted, Volt_Input_RecordingStarted, Volt_Output_RecordingStarted, 
		Digital_RecordingStarted, UserData_RecordingStarted;
	static int Minutes_to_Remember_Volt_Trace;

	// IN
	static class CAnalogInEvent : public UeiDaq::IUeiEventListener
	{
		void OnEvent(UeiDaq::tUeiEvent, void*);
	};
	//static CAnalogInEvent eventINListener;

	// OUT
	static class CAnalogOutEvent : public UeiDaq::IUeiEventListener
	{
		void OnEvent(UeiDaq::tUeiEvent, void*);
	};
	//static CAnalogOutEvent eventOUTListener;

	//static int Load_INI(std::string file_name_and_path);
	static void Init_Memory();
	static void Reset_Head_Pointers();
	static int Init_DAQ();
	static void StartDAQ();
	static void StartDAQ_Analog_Input_Thread();
	static void StartDAQ_Analog_Output_Thread();
	static void StartDAQ_Digital_Input_Thread();
	static void StartDAQ_Digital_Output_Thread();
	static void StopDAQ();
	static void StopDAQ_Thread();
	static void CallBackWork();
	static int Init_Playback(char*, char*, unsigned __int64, unsigned __int64, int, BOOL);
	static void Start_Restart_Playback();
	static size_t Playback_From_File(double*, unsigned __int64*, __int64*);
	static void Write_Header_File(FILE *, int);
	static void Write_Header_File_In_Text(std::ofstream *);
	static size_t Write_Input_Spikes(unsigned __int64*, unsigned __int64*, bool);
	static size_t Write_Input_Volts(unsigned __int64*);
	static size_t Write_Output_Volts_Events(unsigned __int64*, unsigned __int64*);
	static size_t Write_Digital(unsigned __int64*, unsigned __int64*);
	static size_t Write_UserData(unsigned __int64*);
	static void Start_Analog_In_Recording(int, char*, char*, __int64, int);
	static void Start_Analog_Out_Recording(char*, char*, __int64, int);
	static void Start_Digital_Recording(char*, char*, __int64, int);
	static void Start_UserData_Recording(char*, char*, __int64, int);
	static void Convert_Binary_Log_to_Text_Log(char*, char*, BOOL, unsigned __int64, unsigned __int64, char*, char*);
	static void Memory_CleanUP();
	static void DAQ_CleanUP();
	static void Recalculate_Threshold_For(int, int, double);
	static void Recalculate_Threshold_For_All_Electrodes(int, double);
	static int Load_DLL(char*);
	static int ReleaseDLL();
	static int Run_DLL(char*);
	static int Stop_DLL();
	CoreFunction();
	~CoreFunction();

};

#else
typedef
struct CoreFunction
	CoreFunction;
#endif

#ifdef __cplusplus
extern "C" {
#endif
	//#include "GlobalVar.h"
	////////////////////////////// initialization ///////////////////////////////////////
	CoreFunction* DAQ_ReturnRef(void);
	void DAQ_DistroyRef(CoreFunction*);
	////////////////////////////// parameters ///////////////////////////////////////
	void DAQ_Set_Minutes_to_Remember_Volt_Trace(CoreFunction*, int);
	int DAQ_Get_Minutes_to_Remember_Volt_Trace(CoreFunction*);
	void DAQ_Set_Size_of_Spike_Train_Memory(CoreFunction*, unsigned __int64);
	unsigned __int64 DAQ_Get_DAQ_Size_of_Spike_Train_Memory(CoreFunction*);
	void DAQ_Set_DAQpath(CoreFunction*, char*);
	char* DAQ_Get_DAQpath(CoreFunction*);
	void DAQ_Set_Analog_OutputChannels(CoreFunction*, char*);
	char* DAQ_Get_OutputChannels(CoreFunction*);
	void DAQ_Set_Analog_InputChannels(CoreFunction*, char*);
	char* DAQ_Get_InputChannels(CoreFunction*);
	////////////////////////////// Functions ///////////////////////////////////////
	int Core_Init(CoreFunction*);
	int DAQ_Init(CoreFunction*);
	void DAQ_Start(CoreFunction*);
	void DAQ_Stop(CoreFunction*);
	void DAQ_CleanUp(CoreFunction*);

	void DAQ_Start_Analog_In_Recording(CoreFunction*, int, char*, char*, unsigned __int64, int);
	void DAQ_Start_Analog_Out_Recording(CoreFunction*, char*, char*, unsigned __int64, int);
	void DAQ_Start_Digital_Recording(CoreFunction*, char*, char*, unsigned __int64, int);
	void DAQ_Start_UserData_Recording(CoreFunction*, char*, char*, unsigned __int64, int);
	void DAQ_Stop_ALL_Recording(CoreFunction*);
	void DAQ_Stop_Spike_Recording(CoreFunction*);
	void DAQ_Stop_Input_Volt_Recording(CoreFunction*);
	void DAQ_Stop_Output_Volt_Recording(CoreFunction*);
	void DAQ_Stop_Digital_Recording(CoreFunction*);
	void DAQ_Stop_UserData_Recording(CoreFunction*);

	int DAQ_Init_Playback(CoreFunction*, char*, char*, unsigned __int64, unsigned __int64, int);
	void DAQ_Start_Playback(CoreFunction*);
	void DAQ_Pause_Resume_Playback(CoreFunction*);

	int DAQ_Load_DLL(CoreFunction*, char*);
	int DAQ_Run_DLL(CoreFunction*, char*);
	int DAQ_Stop_DLL(CoreFunction*);
	int DAQ_UNLoad_DLL(CoreFunction*);
	void Recalculate_Threshold_For_All_Electrodes(CoreFunction*, int, double);
	void Recalculate_Threshold_For(CoreFunction*, int, int, double);
	void Convert_Binary_Log_to_Text_Log(CoreFunction*, char*, char*, BOOL, unsigned __int64, unsigned __int64, char*, char*);

#ifdef __cplusplus
}
#endif

#endif /*CoreFunction_H*/

