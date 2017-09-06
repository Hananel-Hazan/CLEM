/*
Closed Loop Experiment Manager

CoreFunction.cpp

Core Function code
*/

#include "CoreFunction.hpp"

#ifdef __cplusplus
extern "C" {
#endif
	// initialize Global Var
	CoreFunc_Global_var corefunc_global_var;
#ifdef __cplusplus
}
#endif

std::string CoreFunction::DAQpath;
std::string CoreFunction::Analog_InputChannels;
std::string CoreFunction::Analog_OutputChannels;

UeiDaq::CUeiDevice *CoreFunction::DAQ_Device_info;
UeiDaq::CUeiAnalogScaledReader *CoreFunction::Analog_Reader = NULL;
UeiDaq::CUeiAnalogScaledWriter *CoreFunction::Analog_Writer = NULL;
UeiDaq::CUeiDigitalReader *CoreFunction::Digital_Reader = NULL;
UeiDaq::CUeiDigitalWriter *CoreFunction::Digital_Writer = NULL;
UeiDaq::CUeiSession CoreFunction::Digital_InputSession;
UeiDaq::CUeiSession	CoreFunction::Digital_OutputSession;
UeiDaq::CUeiSession CoreFunction::Analog_InputSession;
UeiDaq::CUeiSession	CoreFunction::Analog_OutputSession;

CoreFunction::ClientIOfun CoreFunction::__ClientIOfun_RealTime_CL;
CoreFunction::ClientIOfun_Engage CoreFunction::__ClientIOfun_DLL_Engage;
CoreFunction::ClientIOfun CoreFunction::__ClientIOfun_SlowPeriodic_CL;
CoreFunction::ClientIOfun_Disengage CoreFunction::__ClientIOfun_DLL_Disengage;
CoreFunction::iniDLL CoreFunction::__initDLL;
HINSTANCE CoreFunction::hInstLibrary;
BOOL CoreFunction::loop_DLL_RealTime = FALSE;
BOOL CoreFunction::loop_DLL_On_Demond = FALSE;
double *CoreFunction::Local_Last_Analog_Data_Out, *CoreFunction::Local_Analog_Data_Out;
float *CoreFunction::Local_Analog_User_Data;
unsigned short CoreFunction::Local_Digital_Data_Out;

BOOL CoreFunction::loop_analog_input = FALSE;
BOOL CoreFunction::loop_analog_output = FALSE;
BOOL CoreFunction::loop_digital_input = FALSE;
BOOL CoreFunction::loop_digital_output = FALSE;
BOOL CoreFunction::Input_Ready_To_Process = FALSE;
BOOL CoreFunction::Input_Flag = FALSE;
BOOL CoreFunction::Analog_Output_Flag = FALSE;
BOOL CoreFunction::Analog_Output_ready = FALSE;
BOOL CoreFunction::Digital_Output_ready = FALSE;
BOOL CoreFunction::Digital_Input_ready = FALSE;
BOOL CoreFunction::init_Analog_Output = TRUE;
double* CoreFunction::Data_In_Pointer = NULL;
double* CoreFunction::Data_In_A = NULL;
double* CoreFunction::Data_In_B = NULL;
double* CoreFunction::Data_Out_A = NULL;
double* CoreFunction::Data_Out_B = NULL;
double* CoreFunction::Data_Out_Pointer = NULL;
unsigned short CoreFunction::Digital_Data_In = 0;
unsigned short CoreFunction::Digital_Data_Out = 0;
float* CoreFunction::Global_Spikes_volt_around_spike;
float* CoreFunction::Global_analog_volt_around_event;
float* CoreFunction::Global_User_Event;

int CoreFunction::Minutes_to_Remember_Volt_Trace = 0;
unsigned __int64 CoreFunction::Spike_VoltTrace_size = 0;
std::list<CoreFunction::Spike_Ditected> *CoreFunction::Envelope_Counter = NULL;
unsigned __int64 CoreFunction::Spike_reArm_in_iteration = 0;
unsigned __int64 *CoreFunction::Spike_reArm_Counter = NULL;

FILE* CoreFunction::Playback_Input_File;
FILE* CoreFunction::Volt_Recording_File;
FILE* CoreFunction::Spike_Recording_File;
FILE* CoreFunction::Spike_Time_Recording_File;
std::ofstream* CoreFunction::Digital_Recording_File;
std::ofstream* CoreFunction::UserData_Recording_File;
std::ofstream* CoreFunction::Output_Volt_Recording_File;

BOOL CoreFunction::PlaybackStarted = FALSE;
BOOL CoreFunction::Playback_pause = FALSE;
BOOL CoreFunction::Spike_RecordingStarted = FALSE;
BOOL CoreFunction::Spike_Time_RecordingStarted = FALSE;
BOOL CoreFunction::Volt_Input_RecordingStarted = FALSE;
BOOL CoreFunction::Volt_Output_RecordingStarted = FALSE;
BOOL CoreFunction::Digital_RecordingStarted = FALSE;
BOOL CoreFunction::UserData_RecordingStarted = FALSE;
std::string CoreFunction::Playback_Current_Filename;
std::string CoreFunction::Playback_Filename_First;
std::string CoreFunction::Playback_Path_First;
std::string CoreFunction::Playback_Current_Path;
unsigned __int64 CoreFunction::Playback_Counter_First = 0;
unsigned __int64 CoreFunction::Playback_Current_Counter = 0;
unsigned __int64 CoreFunction::Playback_Current_Until_Counter = 0;
int CoreFunction::Playback_Current_File_Type = 0;
int CoreFunction::Playback_File_Type_First = 0;
double* CoreFunction::offset_per_electrod = NULL;
double **CoreFunction::avg_per_electrod = NULL;
double* CoreFunction::calculate_offset_per_electrod = NULL;
unsigned __int64* CoreFunction::Threshold_Max_Counter = NULL;
BOOL* CoreFunction::ReCalculate_Threshold_AT_Electrode = NULL;
float* CoreFunction::backup_threshold_Per_Electrod = NULL;
double* CoreFunction::backup_offset_per_electrod = NULL;
int* CoreFunction::elements_Counter_for_RMS = NULL;
BOOL CoreFunction::Acquisition_paused = FALSE;
BOOL CoreFunction::Memory_Initialized = FALSE;
BOOL CoreFunction::DAQ_Initialized = FALSE;
std::chrono::high_resolution_clock::time_point CoreFunction::Paused;
std::chrono::high_resolution_clock::time_point CoreFunction::Continue;
std::thread *CoreFunction::WorkThread_Analog_Input = NULL;
std::thread *CoreFunction::WorkThread_Analog_Output = NULL;
std::thread *CoreFunction::WorkThread_Digital_Input = NULL;
std::thread *CoreFunction::WorkThread_Digital_Output = NULL;
std::thread *CoreFunction::WorkThread_DLL_RealTime = NULL;
std::thread *CoreFunction::WorkThread_DLL_On_Demond = NULL;
std::thread *CoreFunction::WorkThread_File_Convertor = NULL;
std::mutex CoreFunction::DLL_mutex_RealTime;
std::mutex CoreFunction::analog_output_mutex;
std::mutex CoreFunction::digital_output_mutex;
std::mutex CoreFunction::digital_input_mutex;
std::condition_variable CoreFunction::DLL_cv_RealTime;
std::condition_variable CoreFunction::analog_output_cv;
std::condition_variable CoreFunction::digital_output_cv;
std::condition_variable CoreFunction::digital_input_cv;

const double Synthetic_Spike[] = { -0.0148926, -0.010498, -0.00585938, 0.00170898, 0.0078125, 0.0090332, 0.00366211, 0.00146484, -0.00317383, -0.00195313, 0.00317383, -0.0012207, -0.00488281, -0.00610352, -0.0090332, -0.0146484, -0.012207, -0.00244141, -0.000732422, -0.00268555, -0.000488281, 0.00341797, 0.00463867, 0.00268555, 0.00341797, 0.00537109, 0.0078125, 0.00805664, 0.000732422, -0.00170898, -0.00244141, 0, 0.00683594, 0.00805664, 0.00610352, 0.00390625, 0.00195313, 0.00512695, 0.00366211, 0.00415039, 0.00610352, 0.00585938, 0.0090332, 0.00683594, -0.0148926, 0.03125, 0.0524902, 0.095459, 0.143066, 0.144043, 0.10498, 0.0463867, 0.0114746, 0.00268555, -0.017334, -0.0637207, -0.121094, -0.168213, -0.187256, -0.173828, -0.145508, -0.112305, -0.0898438, -0.0810547, -0.0690918, -0.059082, -0.0466309, -0.0341797, -0.0202637, -0.00830078, -0.00878906, -0.0148926, -0.0144043, -0.00634766, -0.000488281, 0.00488281, 0.0124512, 0.0229492, 0.0256348, 0.0205078, 0.0112305, 0.00390625, -0.0065918, -0.015625, -0.0148926, -0.010498, -0.00585938, 0.00170898, 0.0078125, 0.0090332, 0.00366211, 0.00146484, -0.00317383, -0.00195313, 0.00317383, -0.0012207, -0.00488281, -0.00610352, -0.0090332, -0.0146484, -0.012207, -0.00244141, -0.000732422, -0.00268555, -0.000488281, 0.00341797, 0.00463867, 0.00268555, 0.00341797, 0.00537109, 0.0078125, 0.00805664, 0.000732422, -0.00170898, -0.00244141, 0, 0.00683594, 0.00805664, 0.00610352, 0.00390625, 0.00195313, 0.00512695, 0.00366211, 0.00415039, 0.00610352, 0.00585938, 0.0090332, 0.00683594, 0.00512695, 0.00219727, 0.00268555, 0.00415039, 0.00952148, 0.00830078, 0.00952148, 0.0126953, 0.0119629, 0.017334, 0.0195313, 0.0187988, 0.0168457, 0.0146484, 0.015625, 0.0136719, 0.0112305, 0.0117188, 0.0134277, 0.0117188, 0.0107422, 0.0109863, 0.0078125, 0.00512695, 0.00488281, 0.00439453, 0.00512695, 0.00610352, 0.00634766, 0.00219727, -0.000732422, -0.0012207 };


// LOCAL Variables
time_t Start_time;
unsigned __int64	Head_Pointer_Spikes_before_complete_trace;
std::list<unsigned __int64> *playback_TimeStamp_of_spike;
float *Data_From_Last_Iteration;
bool True_forever = true;
unsigned __int64 Digital_Data_Out_Time_stamp = 0;

CoreFunction::CoreFunction(){

	// initialize default Global Variables values
	corefunc_global_var.Status_Text = "";
	corefunc_global_var.Time_Stamp_Counter_Array = NULL;
	corefunc_global_var.RAW_Input_Volts = NULL;
	corefunc_global_var.RAW_Output_Volts = NULL;
	corefunc_global_var.Analog_Spikes_Events = NULL;
	corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts = NULL;
	corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts = NULL;
	corefunc_global_var.Location_of_Last_Event_Analog_Spike = 0;
	corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input = 0;
	corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output = 0;
	corefunc_global_var.Start_Sample_Time = 0;

	corefunc_global_var.DLL_Slow_Process_Running_Freq = 1.0f; // init to 1 Hz

	// threshold 
	corefunc_global_var.threshold_Per_Electrod = NULL;
	corefunc_global_var.Calculating_Threshold = 0;

	// misc. Var
	corefunc_global_var.Time_Stamp_Counter = 0;
	Acquisition_paused = FALSE;
	corefunc_global_var.Convertor_Start = FALSE;
	corefunc_global_var.Force_Convertor_to_Stop = FALSE;
}

CoreFunction::~CoreFunction(){
	//	delete SelfPtr;
	if (backup_threshold_Per_Electrod != NULL){
		delete[] backup_threshold_Per_Electrod;
		delete[] backup_offset_per_electrod;
	}
}

void CoreFunction::CAnalogInEvent::OnEvent(UeiDaq::tUeiEvent event, void *param)
{
	if (event == UeiDaq::UeiEventFrameDone)
	{
		try
		{
			if (CoreFunction::loop_analog_input){

				// Read Digital Input
				CoreFunction::digital_input_cv.notify_one();

				if (CoreFunction::Input_Flag){
					CoreFunction::Analog_Reader->ReadMultipleScansAsync(corefunc_global_var.buffer_Input_Per_Channel, CoreFunction::Data_In_B);
					CoreFunction::Input_Flag = FALSE;
					CoreFunction::Data_In_Pointer = CoreFunction::Data_In_A;
				}
				else{
					CoreFunction::Analog_Reader->ReadMultipleScansAsync(corefunc_global_var.buffer_Input_Per_Channel, CoreFunction::Data_In_A);
					CoreFunction::Input_Flag = TRUE;
					CoreFunction::Data_In_Pointer = CoreFunction::Data_In_B;
				}

				CoreFunction::CallBackWork();
			}
		}
		catch (UeiDaq::CUeiException e)
		{
			std::cout << "INPUT Error: " << e.GetErrorMessage() << std::endl;
		}
	}
	else if (event == UeiDaq::UeiEventError)
	{
		struct tm now;
		localtime_s(&now, &Start_time);
		std::cout << std::endl << " Start Time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << (now.tm_sec) << ':'
			<< ' ' << (now.tm_year + 1900) << '-'
			<< (now.tm_mon + 1) << '-'
			<< now.tm_mday
			<< std::endl;
		Start_time = time(0);   // get time now
		localtime_s(&now, &Start_time);
		std::cout << " End Time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << (now.tm_sec) << ':'
			<< ' ' << (now.tm_year + 1900) << '-'
			<< (now.tm_mon + 1) << '-'
			<< now.tm_mday
			<< std::endl;
		UeiDaq::tUeiError error = (UeiDaq::tUeiError)(uintptr_t)param;
		std::cout << "INPUT Error: " << UeiDaq::CUeiException::TranslateError(error) << std::endl;
	}
}
CoreFunction::CAnalogInEvent eventINListener;

void CoreFunction::CAnalogOutEvent::OnEvent(UeiDaq::tUeiEvent event, void *param)
{
	if (event == UeiDaq::UeiEventFrameDone)
	{
		try
		{
			if (CoreFunction::loop_analog_output){
				if (CoreFunction::Analog_Output_ready){

					if (CoreFunction::Analog_Output_Flag){
						CoreFunction::Analog_Writer->WriteMultipleScansAsync(corefunc_global_var.buffer_Output_Per_Channel, CoreFunction::Data_Out_A);
						CoreFunction::Analog_Output_Flag = FALSE;
						CoreFunction::Data_Out_Pointer = CoreFunction::Data_Out_B;

					}
					else{
						CoreFunction::Analog_Writer->WriteMultipleScansAsync(corefunc_global_var.buffer_Output_Per_Channel, CoreFunction::Data_Out_B);
						CoreFunction::Analog_Output_Flag = TRUE;
						CoreFunction::Data_Out_Pointer = CoreFunction::Data_Out_A;
					}
					CoreFunction::Analog_Output_ready = FALSE;
				}
			}
		}
		catch (UeiDaq::CUeiException e)
		{
			std::cout << "Output Error: " << e.GetErrorMessage() << std::endl;
		}
	}
	else if (event == UeiDaq::UeiEventError)
	{
		struct tm now;
		localtime_s(&now, &Start_time);
		std::cout << std::endl << " Start Time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << (now.tm_sec) << ':'
			<< ' ' << (now.tm_year + 1900) << '-'
			<< (now.tm_mon + 1) << '-'
			<< now.tm_mday
			<< std::endl;
		Start_time = time(0);   // get time now
		localtime_s(&now, &Start_time);
		std::cout << " End Time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << (now.tm_sec) << ':'
			<< ' ' << (now.tm_year + 1900) << '-'
			<< (now.tm_mon + 1) << '-'
			<< now.tm_mday
			<< std::endl;
		UeiDaq::tUeiError error = (UeiDaq::tUeiError)(uintptr_t)param;
		std::cout << "Output Error: " << UeiDaq::CUeiException::TranslateError(error) << std::endl;
	}
}
CoreFunction::CAnalogOutEvent eventOUTListener;

inline void CoreFunction::CallBackWork(){

	// Sample Digital Input
	if ((DAQ_Initialized) && (!Digital_Input_ready)){
		{
			std::lock_guard<std::mutex> lk(digital_input_mutex);
			Digital_Input_ready = TRUE;
		}
		digital_input_cv.notify_one();
	}

	unsigned short electrod_counter = 0;
	for (int buff = 0; buff < corefunc_global_var.Input_bufferSize_for_all_channels; buff++, electrod_counter++){

		if (electrod_counter == corefunc_global_var.Analog_Input_Channels){
			electrod_counter = 0;
			(corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input < corefunc_global_var.Samples_per_Channel) ?
				corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input++ : corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input = 0;
			corefunc_global_var.Time_Stamp_Counter_Array[corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input] = corefunc_global_var.Time_Stamp_Counter;
			corefunc_global_var.Time_Stamp_Counter++;
		}

		double Data = Data_In_Pointer[buff] - offset_per_electrod[electrod_counter];

		// Volt Archiving in Memory
		unsigned __int64 p = corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[electrod_counter] + corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input;
		corefunc_global_var.RAW_Input_Volts[p] = (float)Data;

		// check if threshold needs to be recomputed
		if (ReCalculate_Threshold_AT_Electrode[electrod_counter]){
			// Calculating the threshold 
			avg_per_electrod[electrod_counter][elements_Counter_for_RMS[electrod_counter]] = Data;
			elements_Counter_for_RMS[electrod_counter]++;
			calculate_offset_per_electrod[electrod_counter] += Data;			

			if (elements_Counter_for_RMS[electrod_counter] == Threshold_Max_Counter[electrod_counter]){
				ReCalculate_Threshold_AT_Electrode[electrod_counter] = FALSE;
				
				offset_per_electrod[electrod_counter] = calculate_offset_per_electrod[electrod_counter] / (double)elements_Counter_for_RMS[electrod_counter];
				
				double st = 0;
				for (size_t i = 0; i < elements_Counter_for_RMS[electrod_counter]; i++)
					st += std::pow((avg_per_electrod[electrod_counter][i] - offset_per_electrod[electrod_counter]), 2);
				st /= elements_Counter_for_RMS[electrod_counter];

				double t = corefunc_global_var.RMS_Factor * sqrt(st);
		
				corefunc_global_var.threshold_Per_Electrod[electrod_counter] = (float)((electrod_counter <= corefunc_global_var.LastChannelToInclude ) ? -t : t);

				if (corefunc_global_var.Calculating_Threshold > 0)
					corefunc_global_var.Calculating_Threshold--;
				
				delete[] avg_per_electrod[electrod_counter];
				calculate_offset_per_electrod[electrod_counter] = 0;
				elements_Counter_for_RMS[electrod_counter] = 0;
			}
		}

		// check if enough time passed from the last threshold crossing
		if (Spike_reArm_Counter[electrod_counter] == 0){

			// if playbacking and its not volt playbacking, then set ignore threshold searching
			BOOL playback_spike = false;
			if (PlaybackStarted && Playback_Current_File_Type != 1 &&
				playback_TimeStamp_of_spike[electrod_counter].size() > 0 && playback_TimeStamp_of_spike[electrod_counter].front() == corefunc_global_var.Time_Stamp_Counter){
				playback_spike = true;
				playback_TimeStamp_of_spike[electrod_counter].pop_front();
			}


			// check if the current place passed threshold and MARK the place.
			// if yes, store the information of the spike to the list but dont increase the global spike counter. instead increase the temporary spike counter
			if (
				// boolian of playback timeStamp of a spike
				playback_spike ||
				// search if the current electrode passes the threshold
				((electrod_counter <= corefunc_global_var.LastChannelToInclude &&
				Data_From_Last_Iteration[electrod_counter] >= corefunc_global_var.threshold_Per_Electrod[electrod_counter] &&
				Data <= corefunc_global_var.threshold_Per_Electrod[electrod_counter]) ||
				// check thresholding on aux electrodes 
				(electrod_counter > corefunc_global_var.LastChannelToInclude &&
				Data_From_Last_Iteration[electrod_counter] <= corefunc_global_var.threshold_Per_Electrod[electrod_counter] &&
				Data >= corefunc_global_var.threshold_Per_Electrod[electrod_counter])
				)

				)
			{
				// if spike as been detected
				Spike_reArm_Counter[electrod_counter] = Spike_reArm_in_iteration;
				Spike_Ditected temp;
				temp.End_of_Envelope = corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input + corefunc_global_var.Post_Trigger_Samples;
				if (temp.End_of_Envelope >= corefunc_global_var.Samples_per_Channel)
					temp.End_of_Envelope -= corefunc_global_var.Samples_per_Channel;

				temp.Spike_Header_to_continue = Head_Pointer_Spikes_before_complete_trace;
				corefunc_global_var.Analog_Spikes_Events[Head_Pointer_Spikes_before_complete_trace].channel = electrod_counter;
				corefunc_global_var.Analog_Spikes_Events[Head_Pointer_Spikes_before_complete_trace].timestamp = corefunc_global_var.Time_Stamp_Counter;

				Head_Pointer_Spikes_before_complete_trace++;
				if (Head_Pointer_Spikes_before_complete_trace == corefunc_global_var.Size_Of_Events_In_Memory)	Head_Pointer_Spikes_before_complete_trace = 0;

				Envelope_Counter[electrod_counter].push_back(temp);
			}
		}
		else
			Spike_reArm_Counter[electrod_counter]--;

		if ((Envelope_Counter[electrod_counter].size() > 0) &&  // there is spikes in the que wating for complition
			(Envelope_Counter[electrod_counter].front().End_of_Envelope == corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input) // check the global time counter for complite the spike envelop 
			){

			unsigned __int64 i = (corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input >= Spike_VoltTrace_size) ?
				corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input - Spike_VoltTrace_size :
				(corefunc_global_var.Samples_per_Channel - Spike_VoltTrace_size) + corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input;

			unsigned __int64 pc = Envelope_Counter[electrod_counter].front().Spike_Header_to_continue,
				pp = corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[electrod_counter];
			for (unsigned __int64 t = 0; t < Spike_VoltTrace_size; t++, i++)
			{
				if (i == corefunc_global_var.Samples_per_Channel)	i = 0;
				corefunc_global_var.Analog_Spikes_Events[pc].value[t] = corefunc_global_var.RAW_Input_Volts[pp + i];
			}

			Envelope_Counter[electrod_counter].pop_front();

			if (corefunc_global_var.Spike_count < corefunc_global_var.Size_Of_Events_In_Memory)
				corefunc_global_var.Spike_count++;


			corefunc_global_var.Location_of_Last_Event_Analog_Spike++;
			if (corefunc_global_var.Location_of_Last_Event_Analog_Spike == corefunc_global_var.Size_Of_Events_In_Memory)
				corefunc_global_var.Location_of_Last_Event_Analog_Spike = 0;

		}

		Data_From_Last_Iteration[electrod_counter] = (float)Data;
	}

	(corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input < corefunc_global_var.Samples_per_Channel) ?
		corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input++ : corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input = 0;


	// Notify the Real Time DLL that its time to run
	if (!CoreFunction::Input_Ready_To_Process){
		// send data to the worker_DLL thread
		{
			std::lock_guard<std::mutex> lk(CoreFunction::DLL_mutex_RealTime);
			CoreFunction::Input_Ready_To_Process = TRUE;
		}
		CoreFunction::DLL_cv_RealTime.notify_one();
	}

}

size_t CoreFunction::Write_Input_Spikes(unsigned __int64 *From_Header_p, unsigned __int64 *StartPoint, bool envelope){
	unsigned __int64 Until_Header = corefunc_global_var.Location_of_Last_Event_Analog_Spike;
	unsigned __int64 Size = 0, From_Header = *From_Header_p;
	size_t Number_of_writen_bytes = 0;
	FILE *file_ptr = (envelope) ? Spike_Recording_File : Spike_Time_Recording_File;
	if (file_ptr == NULL) return 0;

	Size = (Until_Header >= From_Header) ?
		(Until_Header - From_Header) :
		((corefunc_global_var.Size_Of_Events_In_Memory - From_Header) + Until_Header);

	if (Size == 0) return 0;

	for (unsigned __int64 t = 0; t < Size; t++){
		unsigned __int64 time = corefunc_global_var.Analog_Spikes_Events[From_Header].timestamp - *StartPoint;
		// time 
		Number_of_writen_bytes += sizeof(unsigned __int64) * fwrite(&time, sizeof(unsigned __int64), 1, file_ptr);
		// channel
		Number_of_writen_bytes += sizeof(unsigned short) * fwrite(&corefunc_global_var.Analog_Spikes_Events[From_Header].channel, sizeof(unsigned short), 1, file_ptr);

		if (envelope){
			// short saving		
			short *temp = new short[Spike_VoltTrace_size];
			for (int e = 0; e < Spike_VoltTrace_size; e++)
				temp[e] = (short)ceil(corefunc_global_var.Analog_Spikes_Events[From_Header].value[e] / OneBitIsEqualTo);
			Number_of_writen_bytes += sizeof(short) * fwrite(temp, sizeof(short), Spike_VoltTrace_size, file_ptr);
			delete[] temp;
		}

		From_Header++;
		if (From_Header == corefunc_global_var.Size_Of_Events_In_Memory) From_Header = 0;
	}
	*From_Header_p = Until_Header;
	return  Number_of_writen_bytes;
}

size_t CoreFunction::Write_Input_Volts(unsigned __int64 *From_Header_p){

	unsigned __int64 Until_Header = corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input;
	unsigned __int64 Size = 0, From_Header = *From_Header_p;
	size_t Number_of_writen_bytes = 0;

	Size = (Until_Header >= From_Header) ?
		(Until_Header - From_Header) :
		((corefunc_global_var.Samples_per_Channel - From_Header) + Until_Header);

	// short saving
	short *temp = new short[corefunc_global_var.Analog_Input_Channels];
	for (unsigned __int64 i = 0; i < Size; i++){

		for (int e = 0; e < corefunc_global_var.Analog_Input_Channels; e++)
			temp[e] = (short)ceil(corefunc_global_var.RAW_Input_Volts[corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[e] + From_Header] / OneBitIsEqualTo);

		From_Header++;
		if (From_Header == corefunc_global_var.Samples_per_Channel) From_Header = 0;

		Number_of_writen_bytes += sizeof(short) * fwrite(temp, sizeof(short), corefunc_global_var.Analog_Input_Channels, Volt_Recording_File);

	}
	delete[] temp;


	*From_Header_p = Until_Header;
	return Number_of_writen_bytes;
}

size_t CoreFunction::Write_Output_Volts_Events(unsigned __int64 *From_Header_p, unsigned __int64 *StartPoint){

	unsigned __int64 Until_Header = corefunc_global_var.Location_of_Last_Event_Analog_Output;
	unsigned __int64 Size = 0, From_Header = *From_Header_p;
	size_t Number_of_writen_bytes = 0;

	Size = (Until_Header >= From_Header) ?
		(Until_Header - From_Header) :
		(((corefunc_global_var.Size_Of_Events_In_Memory) - From_Header) + Until_Header);

	if (Size == 0) return 0;

	for (unsigned __int64 t = 0; t < Size; t++){
		unsigned __int64 time = corefunc_global_var.Analog_Output_Events[From_Header].timestamp - *StartPoint;

		*Output_Volt_Recording_File << time << "," << corefunc_global_var.Analog_Output_Events[From_Header].channel;
		for (int e = 0; e < corefunc_global_var.buffer_Output_Per_Channel; e++)
			*Output_Volt_Recording_File << "," << corefunc_global_var.Analog_Output_Events[From_Header].value[e];
		*Output_Volt_Recording_File << std::endl;

		From_Header++;
		if (From_Header == corefunc_global_var.Size_Of_Events_In_Memory) From_Header = 0;
	}
	*From_Header_p = Until_Header;
	return  Number_of_writen_bytes;
}

size_t CoreFunction::Write_Digital(unsigned __int64 *From_Header_I, unsigned __int64 *From_Header_O){

	unsigned __int64 Until_Header_I = 0, Until_Header_O = 0, Size = 0, from_Header_I = 0, from_Header_O = 0;
	size_t Number_of_writen_bytes = 0;

	from_Header_I = *From_Header_I;
	Until_Header_I = corefunc_global_var.Location_of_Last_Event_Digital_I;

	from_Header_O = *From_Header_O;
	Until_Header_O = corefunc_global_var.Location_of_Last_Event_Digital_O;


	Size = (Until_Header_I >= from_Header_I) ?
		(Until_Header_I - from_Header_I) :
		((corefunc_global_var.Size_Of_Events_In_Memory - from_Header_I) + Until_Header_I);
	if (Size > 0){
		for (unsigned __int64 i = 0; i < Size; i++){
			*Digital_Recording_File << "I," << corefunc_global_var.Digital_I_Events[from_Header_I].timestamp << "," << corefunc_global_var.Digital_I_Events[from_Header_I].value << std::endl;
			from_Header_I = (from_Header_I < corefunc_global_var.Size_Of_Events_In_Memory) ? from_Header_I + 1 : 0;
			Number_of_writen_bytes++;
		}
		*From_Header_I = Until_Header_I;
	}

	Size = (Until_Header_O >= from_Header_O) ?
		(Until_Header_O - from_Header_O) :
		((corefunc_global_var.Size_Of_Events_In_Memory - from_Header_O) + Until_Header_O);
	if (Size > 0){
		for (unsigned __int64 i = 0; i < Size; i++){
			*Digital_Recording_File << "O," << corefunc_global_var.Digital_O_Events[from_Header_O].timestamp << "," << corefunc_global_var.Digital_O_Events[from_Header_O].value << std::endl;
			from_Header_O = (from_Header_O < corefunc_global_var.Size_Of_Events_In_Memory) ? from_Header_O + 1 : 0;
			Number_of_writen_bytes++;
		}
		*From_Header_O = Until_Header_O;
	}

	return Number_of_writen_bytes;
}

size_t CoreFunction::Write_UserData(unsigned __int64 *From_Header){

	unsigned __int64 Until_Header = 0, Size = 0, from_Header = 0;
	size_t Number_of_writen_bytes = 0;

	from_Header = *From_Header;
	Until_Header = corefunc_global_var.Location_of_Last_Event_User_Analog;

	Size = (Until_Header >= from_Header) ?
		(Until_Header - from_Header) :
		((corefunc_global_var.Size_Of_Events_In_Memory - from_Header) + Until_Header);
	if (Size > 0){
		for (unsigned __int64 i = 0; i < Size; i++){
			*UserData_Recording_File << corefunc_global_var.User_Analog_Events[from_Header].timestamp;

			for (size_t t = 0; t < corefunc_global_var.Number_of_User_Data_Channels; t++)
				*UserData_Recording_File << "," << corefunc_global_var.User_Analog_Events[from_Header].value[t];

			*UserData_Recording_File << std::endl;

			from_Header = (from_Header < corefunc_global_var.Size_Of_Events_In_Memory) ? from_Header + 1 : 0;
			Number_of_writen_bytes++;
		}
		*From_Header = Until_Header;
	}

	return Number_of_writen_bytes;
}

void CoreFunction::Init_Memory(){

	if (Memory_Initialized)
		return;

	// Global Var init
	corefunc_global_var.Start_Sample_Time = 0;
	corefunc_global_var.DLL_Slow_Process_Running_Freq = 1.0f; // init to 1 Hz

	// misc Var

	corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input = 0;
	corefunc_global_var.Location_of_Last_Event_Analog_Spike = 0;
	corefunc_global_var.Samples_per_Channel = (UINT64)(corefunc_global_var.Input_Hz * 60 * Minutes_to_Remember_Volt_Trace);

	if (corefunc_global_var.Input_Hz == 0){
		Memory_Initialized = FALSE;
		return;
	}

	// Volts
	Data_From_Last_Iteration = new float[corefunc_global_var.Analog_Input_Channels];
	corefunc_global_var.RAW_Input_Volts = new float[corefunc_global_var.Analog_Input_Channels * (corefunc_global_var.Samples_per_Channel + 1)];
	corefunc_global_var.RAW_Output_Volts = new float[corefunc_global_var.Analog_Output_Channels * (corefunc_global_var.Samples_per_Channel + 1)];

	// Electrode pointer in Memory
	corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts = new unsigned __int64[corefunc_global_var.Analog_Input_Channels];
	corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[0] = 0;
	for (int i = 1; i < corefunc_global_var.Analog_Input_Channels; i++)
		corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[i] = corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[i - 1] + (corefunc_global_var.Samples_per_Channel + 1);

	corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts = new unsigned __int64[corefunc_global_var.Analog_Output_Channels];
	corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts[0] = 0;
	for (int i = 1; i < corefunc_global_var.Analog_Output_Channels; i++)
		corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts[i] = corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts[i - 1] + (corefunc_global_var.Samples_per_Channel + 1);

	// TimeStamp
	corefunc_global_var.Time_Stamp_Counter_Array = new unsigned __int64[(corefunc_global_var.Samples_per_Channel + 1)];

	// Digital Input / Output
	corefunc_global_var.Digital_I_Events = new DIGITALEVENT[corefunc_global_var.Size_Of_Events_In_Memory];
	corefunc_global_var.Digital_O_Events = new DIGITALEVENT[corefunc_global_var.Size_Of_Events_In_Memory];

	// Spikes
	Spike_VoltTrace_size = corefunc_global_var.Pre_Trigger_Samples + corefunc_global_var.Post_Trigger_Samples;
	corefunc_global_var.Analog_Spikes_Events = new ANALOGEVENT[corefunc_global_var.Size_Of_Events_In_Memory];
	Global_Spikes_volt_around_spike = new float[Spike_VoltTrace_size * corefunc_global_var.Size_Of_Events_In_Memory];
	for (UINT64 i = 0, p_counter = 0; i < corefunc_global_var.Size_Of_Events_In_Memory; i++, p_counter += Spike_VoltTrace_size){
		// Spikes Analog Input
		corefunc_global_var.Analog_Spikes_Events[i].value = &Global_Spikes_volt_around_spike[p_counter];
	}

	// Analog Output Events
	corefunc_global_var.Analog_Output_Events = new ANALOGEVENT[corefunc_global_var.Size_Of_Events_In_Memory];
	Global_analog_volt_around_event = new float[corefunc_global_var.buffer_Output_Per_Channel * corefunc_global_var.Size_Of_Events_In_Memory];
	for (UINT64 i = 0, p_counter = 0, end = corefunc_global_var.Size_Of_Events_In_Memory;
		i < end;
		i++, p_counter += corefunc_global_var.buffer_Output_Per_Channel){
		//Analog Event Output		
		corefunc_global_var.Analog_Output_Events[i].value = &Global_analog_volt_around_event[p_counter];
	}

	// User Event for GUI
	corefunc_global_var.Number_of_User_Data_Channels = 2;
	corefunc_global_var.User_Analog_Events = new USEREVENT[corefunc_global_var.Size_Of_Events_In_Memory];
	Global_User_Event = new float[corefunc_global_var.Number_of_User_Data_Channels * corefunc_global_var.Size_Of_Events_In_Memory];
	for (UINT64 i = 0, p_counter = 0, end = corefunc_global_var.Size_Of_Events_In_Memory;
		i < end;
		i++, p_counter += 2){
		corefunc_global_var.User_Analog_Events[i].value = &Global_User_Event[p_counter];
	}

	// threshold 
	corefunc_global_var.threshold_Per_Electrod = new float[corefunc_global_var.Analog_Input_Channels];
	ReCalculate_Threshold_AT_Electrode = new BOOL[corefunc_global_var.Analog_Input_Channels];
	Threshold_Max_Counter = new unsigned __int64[corefunc_global_var.Analog_Input_Channels];
	playback_TimeStamp_of_spike = new std::list<unsigned __int64>[corefunc_global_var.Analog_Input_Channels];

	avg_per_electrod = new double*[corefunc_global_var.Analog_Input_Channels];
	offset_per_electrod = new double[corefunc_global_var.Analog_Input_Channels];
	calculate_offset_per_electrod = new double[corefunc_global_var.Analog_Input_Channels];
	elements_Counter_for_RMS = new int[corefunc_global_var.Analog_Input_Channels];
	Envelope_Counter = new std::list<CoreFunction::Spike_Ditected>[corefunc_global_var.Analog_Input_Channels];
	Spike_reArm_Counter = new unsigned __int64[corefunc_global_var.Analog_Input_Channels];

	Memory_Initialized = TRUE;

	Reset_Head_Pointers();
}

void CoreFunction::Reset_Head_Pointers(){

	if (!Memory_Initialized)
		return;

	corefunc_global_var.Start_Sample_Time = 0;
	corefunc_global_var.Location_of_Last_Event_Analog_Spike = 0;
	Head_Pointer_Spikes_before_complete_trace = 0;
	corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input = 0;
	corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output = 0;
	corefunc_global_var.Location_of_Last_Event_Digital_I = 0;
	corefunc_global_var.Location_of_Last_Event_Digital_O = 0;
	corefunc_global_var.Location_of_Last_Event_Analog_Output = 0;
	corefunc_global_var.Spike_count = 0;

	std::uninitialized_fill(corefunc_global_var.Time_Stamp_Counter_Array, corefunc_global_var.Time_Stamp_Counter_Array + corefunc_global_var.Samples_per_Channel, 0);
	std::uninitialized_fill(corefunc_global_var.RAW_Input_Volts, corefunc_global_var.RAW_Input_Volts + corefunc_global_var.Size_Of_Events_In_Memory, (float)0.0);
	std::uninitialized_fill(corefunc_global_var.RAW_Output_Volts, corefunc_global_var.RAW_Output_Volts + corefunc_global_var.Samples_per_Channel, (float)0.0);

	// Spikes in Analog Input Spike Events initialized
	std::uninitialized_fill(Global_Spikes_volt_around_spike, Global_Spikes_volt_around_spike + (Spike_VoltTrace_size * corefunc_global_var.Size_Of_Events_In_Memory), (float)0.0);
	// Analog Output events
	std::uninitialized_fill(Global_analog_volt_around_event, Global_analog_volt_around_event + (corefunc_global_var.buffer_Output_Per_Channel * corefunc_global_var.Size_Of_Events_In_Memory), (float)0.0);
	// User Output events
	std::uninitialized_fill(Global_User_Event, Global_User_Event + (corefunc_global_var.Number_of_User_Data_Channels * corefunc_global_var.Size_Of_Events_In_Memory), (float)0.0);

	// calculate the refractory time trigger for threshold detection
	Spike_reArm_in_iteration = (unsigned __int64)(corefunc_global_var.Spike_refractory_in_MicroSecond / ((1 / corefunc_global_var.Input_Hz) * 1000000));


	for (size_t i = 0; i < corefunc_global_var.Size_Of_Events_In_Memory; i++){
		// Spikes in Analog Input Spike Events initialized
		corefunc_global_var.Analog_Spikes_Events[i].timestamp = 0;
		corefunc_global_var.Analog_Spikes_Events[i].channel = 0;

		// Analog Output events		
		corefunc_global_var.Analog_Output_Events[i].timestamp = 0;
		corefunc_global_var.Analog_Output_Events[i].channel = 0;

		// User events
		corefunc_global_var.User_Analog_Events[i].timestamp = 0;

		// Digital I/O
		corefunc_global_var.Digital_I_Events[i].timestamp = 0;
		corefunc_global_var.Digital_I_Events[i].value = 0;
		corefunc_global_var.Digital_O_Events[i].timestamp = 0;
		corefunc_global_var.Digital_O_Events[i].value = 0;

	}

	for (int i = 0; i < corefunc_global_var.Analog_Input_Channels; i++){
		Threshold_Max_Counter[i] = 0;
		playback_TimeStamp_of_spike[i].clear();
		Data_From_Last_Iteration[i] = 0;
		if (Acquisition_paused){
			corefunc_global_var.threshold_Per_Electrod[i] = backup_threshold_Per_Electrod[i];
			offset_per_electrod[i] = backup_offset_per_electrod[i];
		}
		else{
			corefunc_global_var.threshold_Per_Electrod[i] = (PlaybackStarted) ? (float)corefunc_global_var.MAX_V : (float)-0.1;
			offset_per_electrod[i] = 0.0;
		}
		calculate_offset_per_electrod[i] = 0;

		elements_Counter_for_RMS[i] = 0;

		ReCalculate_Threshold_AT_Electrode[i] = FALSE;
		Envelope_Counter[i].clear();
		Spike_reArm_Counter[i] = 0;
	}
	corefunc_global_var.Calculating_Threshold = 0;

}

int CoreFunction::Init_DAQ(){

	if (DAQ_Initialized)
		return 1; // already initilizied

	std::string s;

	try{
		UeiDaq::CUeiDeviceEnumerator devEnum("pwrdaq://");
		DAQ_Device_info = devEnum.GetDevice(0);

		s = DAQ_Device_info->GetDeviceName();

		if (s.compare(corefunc_global_var.BoardModelString) != 0)
		{
			// Throw error: card does not match
			std::string str = "ERROR: The DAQ card in the computer doesn’t match to the one that state in the configuration file!";
			char *msg = &str[0u];
			WriteMessage(msg, FALSE);
			return -2;
		}
		else{

			corefunc_global_var.MIN_V = -corefunc_global_var.MAX_V;

			if (s.compare("PD2-MF-64-3M/12L") == 0)
				if (!(
					((corefunc_global_var.MAX_V == 10.0) && (corefunc_global_var.MIN_V == -10.0)) ||
					((corefunc_global_var.MAX_V == 5.0) && (corefunc_global_var.MIN_V == -5.0)) ||
					((corefunc_global_var.MAX_V == 1.0) && (corefunc_global_var.MIN_V == -1.0)) ||
					((corefunc_global_var.MAX_V == 0.5) && (corefunc_global_var.MIN_V == -0.5)) ||
					((corefunc_global_var.MAX_V == 0.1) && (corefunc_global_var.MIN_V == -0.1)) ||
					((corefunc_global_var.MAX_V == 0.05) && (corefunc_global_var.MIN_V == -0.05)) ||
					((corefunc_global_var.MAX_V == 0.01) && (corefunc_global_var.MIN_V == -0.05))
					)){
					// Throw error: voltage does not match
					std::string str = "MAX_V and / or MIN_V values are incorrect!!!";
					char *msg = &str[0u];
					WriteMessage(msg, FALSE);
					return -2;
				}
			if (s.compare("PD2-MF-64-3M/12H") == 0)
				if (!(
					((corefunc_global_var.MAX_V == 10.0) && (corefunc_global_var.MIN_V == -10.0)) ||
					((corefunc_global_var.MAX_V == 5.0) && (corefunc_global_var.MIN_V == -5.0)) ||
					((corefunc_global_var.MAX_V == 2.0) && (corefunc_global_var.MIN_V == -2.0))
					)){
					// Throw error: voltage does not match
					std::string str = "MAX_V and / or MIN_V values are incorrect!!!";
					char *msg = &str[0u];
					WriteMessage(msg, FALSE);
					return -2;
				}

		}

		try
		{

			s = "DAQ model: " + DAQ_Device_info->GetDeviceName();
			char *m = &s[0u];
			WriteMessage(m, FALSE);

			///////////////////
			//// ANALOG INPUT 
			// Create Analog input channels on a powerdaq board
			Analog_InputSession.CreateAIChannel(DAQpath + "/ai" + Analog_InputChannels, corefunc_global_var.MIN_V, corefunc_global_var.MAX_V, UeiDaq::UeiAIChannelInputModeSingleEnded);

			// Configure the session to acquire
			Analog_InputSession.ConfigureTimingForBufferedIO(corefunc_global_var.buffer_Input_Per_Channel, UeiDaq::UeiTimingClockSourceContinuous, corefunc_global_var.Input_Hz, UeiDaq::UeiDigitalEdgeRising, UeiDaq::UeiTimingDurationContinuous);
			Analog_InputSession.GetDataStream()->SetOverUnderRun(1);

			corefunc_global_var.Analog_Input_Channels = Analog_InputSession.GetNumberOfChannels();
			corefunc_global_var.Input_bufferSize_for_all_channels = corefunc_global_var.buffer_Input_Per_Channel * corefunc_global_var.Analog_Input_Channels;

			//-------- change that suggested by UEI Suport
			// Program CV clock to 10000Hz to induce 100us delay between channels.
			//Analog_InputSession.GetTiming()->SetConvertClockSource(UeiDaq::UeiTimingClockSourceInternal);
			//Analog_InputSession.GetTiming()->SetConvertClockRate(corefunc_global_var.Input_Hz * corefunc_global_var.Analog_Input_Channels);
			//------------

			// Allocate a buffer to hold each acquired frame
			Data_In_A = new double[Analog_InputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Input_Per_Channel];
			std::uninitialized_fill(Data_In_A, Data_In_A + Analog_InputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Input_Per_Channel, (float)0.0);
			Data_In_B = new double[Analog_InputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Input_Per_Channel];
			std::uninitialized_fill(Data_In_B, Data_In_B + Analog_InputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Input_Per_Channel, (float)0.0);

			Input_Flag = FALSE;
			Data_In_Pointer = Data_In_A;

			// Create a reader object to read data synchronously.
			Analog_Reader = new UeiDaq::CUeiAnalogScaledReader(Analog_InputSession.GetDataStream());

			///////////////////
			//// ANALOG OUTPUT 
			// Create Analog output channels on a powerdaq board
			Analog_OutputSession.CreateAOChannel(DAQpath + "/ao" + Analog_OutputChannels, -10.0, 10.0);
			Analog_OutputSession.ConfigureTimingForSimpleIO();

			// For Async outputing
			//Analog_OutputSession.ConfigureTimingForBufferedIO(corefunc_global_var.buffer_Output_Per_Channel, UeiDaq::UeiTimingClockSourceInternal, corefunc_global_var.Output_Hz, UeiDaq::UeiDigitalEdgeRising, UeiDaq::UeiTimingDurationSingleShot);
			//Analog_OutputSession.ConfigureTimingForBufferedIO(corefunc_global_var.buffer_Output_Per_Channel, UeiDaq::UeiTimingClockSourceInternal, corefunc_global_var.Output_Hz, UeiDaq::UeiDigitalEdgeRising, UeiDaq::UeiTimingDurationContinuous);
			//Analog_OutputSession.GetDataStream()->SetNumberOfFrames(0);
			//Analog_OutputSession.GetDataStream()->SetScanSize(1);
			//Analog_OutputSession.GetDataStream()->SetOverUnderRun(1);

			corefunc_global_var.Analog_Output_Channels = Analog_OutputSession.GetNumberOfChannels();


			// Allocate a buffer to hold each output frame
			Data_Out_A = new double[Analog_OutputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Output_Per_Channel];
			Data_Out_B = new double[Analog_OutputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Output_Per_Channel];
			std::uninitialized_fill(Data_Out_A, Data_Out_A + Analog_OutputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Output_Per_Channel, (float)0.0);
			std::uninitialized_fill(Data_Out_B, Data_Out_B + Analog_OutputSession.GetNumberOfChannels()*corefunc_global_var.buffer_Output_Per_Channel, (float)0.0);
			Analog_Output_Flag = FALSE;
			Data_Out_Pointer = Data_Out_A;

			// Create a reader object to read data synchronously.
			Analog_Writer = new UeiDaq::CUeiAnalogScaledWriter(Analog_OutputSession.GetDataStream());

			///////////////////
			//// DIGITAL OUTPUT 
			// Create digital input channels on a powerdaq board
			Digital_OutputSession.CreateDOChannel(DAQpath + "/do0");
			Digital_OutputSession.ConfigureTimingForSimpleIO();
			Digital_Writer = new UeiDaq::CUeiDigitalWriter(Digital_OutputSession.GetDataStream());
			Digital_Data_Out = 0;

			///////////////////
			//// DIGITAL INPUT 
			// Create digital input channels on a powerdaq board
			Digital_InputSession.CreateDIChannel(DAQpath + "/di0");
			Digital_InputSession.ConfigureTimingForSimpleIO();
			Digital_Reader = new UeiDaq::CUeiDigitalReader(Digital_InputSession.GetDataStream());
			Digital_Data_In = 0;

			s = "DAQ initialization OK";
			m = &s[0u];
			WriteMessage(m, FALSE);

			DAQ_Initialized = TRUE;

		}
		catch (UeiDaq::CUeiException e)
		{
			//char msg[100];
			std::string str = "DAQ initialization Error: ";
			str.append(e.GetErrorMessage());
			str.append("\n");
			char *msg = &str[0u];
			WriteMessage(msg, FALSE);
			std::cout << str;

			DAQ_Initialized = FALSE;
			Memory_CleanUP();
			DAQ_CleanUP();
			return -1;
		}
	}
	catch (UeiDaq::CUeiException e)
	{
		s = " DAQ is not responsive ";
		s.append(e.GetErrorMessage());
		s.append("\n");
		char *msg = &s[0u];
		WriteMessage(msg, FALSE);
		std::cout << s;

		DAQ_Initialized = FALSE;
		Memory_CleanUP();
		DAQ_CleanUP();
		return -1;
	}

	Init_Memory();

	return 0;
}

int CoreFunction::Init_Playback(char* path, char* filename, unsigned __int64 counter, unsigned __int64 untilcounter, int file_type, BOOL first_time){

	int return_value = 0;

	if (PlaybackStarted){
		Playback_pause = false;
		PlaybackStarted = false;
		Sleep(1000);
	}


	// initilization 
	std::string candidate_f = path;
	candidate_f.append(filename);
	candidate_f.append(".");

	std::stringstream ss;
	ss << std::setw(8) << std::setfill('0') << counter;
	candidate_f.append(ss.str());

	switch (file_type)
	{
	case 0:
		candidate_f.append(corefunc_global_var.Spike_Extention_File);
		break;
	case 1:
		candidate_f.append(corefunc_global_var.Volt_IN_Extention_File);
		break;
	case 2:
		candidate_f.append(corefunc_global_var.Spike_Time_file);
		break;
	}
	char *f = &candidate_f[0u];

	fopen_s(&Playback_Input_File, f, "rb");
	if (Playback_Input_File == NULL) { fputs("File error", stderr); return(-1); }

	if (first_time){
		Playback_Path_First = path;
		Playback_Filename_First = filename;
		Playback_Counter_First = counter;
		Playback_File_Type_First = file_type;
	}
	Playback_Current_Filename = filename;
	Playback_Current_Path = path;
	Playback_Current_Counter = counter;
	Playback_Current_Until_Counter = untilcounter;
	Playback_Current_File_Type = file_type;

	int Analog_Input_Channels, Analog_Output_Channels, bufferSize, buffer_Input_Per_Channel, buffer_Output_Per_Channel;// , Downgrade_Signal;
	float DLL_Slow_Process_Running_Freq;
	double Input_Hz, Output_Hz, MAX_V;
	unsigned int Pre_Trigger_Samples, Post_Trigger_Samples;
	int Playback_file_Continues = 0;
	BOOL change = false;

	// read all parameters
	// ----- Global Var
	// bool is converted to char because for VS11+ bool is char therefor more values can be stored!
	fread(&Playback_file_Continues, sizeof(BOOL), 1, Playback_Input_File);

	//Time and Date of the record
	time_t t;   // get time now
	fread(&t, sizeof(time_t), 1, Playback_Input_File);

	//Parameters of DAQ
	fread(&Analog_Input_Channels, sizeof(int), 1, Playback_Input_File);
	fread(&Analog_Output_Channels, sizeof(int), 1, Playback_Input_File);
	fread(&bufferSize, sizeof(int), 1, Playback_Input_File);
	fread(&buffer_Input_Per_Channel, sizeof(int), 1, Playback_Input_File);
	fread(&buffer_Output_Per_Channel, sizeof(int), 1, Playback_Input_File);
	fread(&Input_Hz, sizeof(double), 1, Playback_Input_File);
	fread(&Output_Hz, sizeof(double), 1, Playback_Input_File);

	fread(&DLL_Slow_Process_Running_Freq, sizeof(int), 1, Playback_Input_File);

	fread(&Pre_Trigger_Samples, sizeof(unsigned int), 1, Playback_Input_File);
	fread(&Post_Trigger_Samples, sizeof(unsigned int), 1, Playback_Input_File);
	fread(&MAX_V, sizeof(double), 1, Playback_Input_File);
	fread(&MAX_V, sizeof(double), 1, Playback_Input_File); // backward compatible

	// chack if the parameters in the file are differeent from what is set in  memory
	if ((corefunc_global_var.Analog_Input_Channels != Analog_Input_Channels) ||
		(corefunc_global_var.Analog_Output_Channels != Analog_Output_Channels) ||
		(corefunc_global_var.Input_bufferSize_for_all_channels != bufferSize) ||
		(corefunc_global_var.buffer_Input_Per_Channel != buffer_Input_Per_Channel) ||
		(corefunc_global_var.buffer_Output_Per_Channel != buffer_Output_Per_Channel) ||
		(corefunc_global_var.DLL_Slow_Process_Running_Freq != DLL_Slow_Process_Running_Freq) ||
		//(corefunc_global_var.Downgrade_Signal != Downgrade_Signal) ||
		(corefunc_global_var.MAX_V != MAX_V) ||
		(corefunc_global_var.Input_Hz != Input_Hz) ||
		(corefunc_global_var.Output_Hz != Output_Hz) ||
		(corefunc_global_var.Pre_Trigger_Samples != Pre_Trigger_Samples) ||
		(corefunc_global_var.Post_Trigger_Samples != Post_Trigger_Samples) ||
		(CoreFunction::Playback_Current_File_Type != Playback_file_Continues)
		)
		change = true;

	if (change){
		Memory_CleanUP();
		return_value = 1;

		//Apply the changes
		corefunc_global_var.Analog_Input_Channels = Analog_Input_Channels;
		corefunc_global_var.Analog_Output_Channels = Analog_Output_Channels;
		corefunc_global_var.Input_bufferSize_for_all_channels = bufferSize;
		corefunc_global_var.buffer_Input_Per_Channel = buffer_Input_Per_Channel;
		corefunc_global_var.buffer_Output_Per_Channel = buffer_Output_Per_Channel;
		corefunc_global_var.DLL_Slow_Process_Running_Freq = DLL_Slow_Process_Running_Freq;
		corefunc_global_var.MAX_V = MAX_V;
		corefunc_global_var.MIN_V = -MAX_V;
		corefunc_global_var.Input_Hz = Input_Hz;
		corefunc_global_var.Output_Hz = Output_Hz;
		corefunc_global_var.Pre_Trigger_Samples = Pre_Trigger_Samples;
		corefunc_global_var.Post_Trigger_Samples = Post_Trigger_Samples;
		CoreFunction::Playback_Current_File_Type = Playback_file_Continues;

		Init_Memory();

		corefunc_global_var.Time_Stamp_Counter = 0;

		// important init Data_In_Pointer because init_memory will not allocate it
		if (Data_In_Pointer == NULL)
			Data_In_Pointer = new double[corefunc_global_var.Analog_Input_Channels * corefunc_global_var.buffer_Input_Per_Channel];
	}

	// set the internal time to the first file record
	if (Playback_Current_File_Type == 1){  // Volt 
		//No need
	}
	else{  // Spike		
		// read the first line that contains a time stamp
		fread(&corefunc_global_var.Time_Stamp_Counter, sizeof(unsigned __int64), 1, Playback_Input_File);
		corefunc_global_var.Start_Sample_Time = corefunc_global_var.Time_Stamp_Counter;
		// Need to rewind and return the same position of the buffers
		long tmp = sizeof(unsigned __int64);
		fseek(Playback_Input_File, -tmp, SEEK_CUR);
	}

	return return_value;
}

void CoreFunction::StartDAQ(){

	StopDAQ();
	DAQ_CleanUP();
	Memory_CleanUP();
	Init_DAQ();
	if (DAQ_Initialized == FALSE) return;
	Reset_Head_Pointers();

	Start_time = time(0);   // get time now
	struct tm now;
	localtime_s(&now, &Start_time);
	std::cout << "Start Time: " << (now.tm_hour) << ':' << (now.tm_min) << ':' << (now.tm_sec) << ':'
		<< ' ' << (now.tm_year + 1900) << '-'
		<< (now.tm_mon + 1) << '-'
		<< now.tm_mday
		<< std::endl;


	if (Acquisition_paused){
		Continue = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(Continue - Paused);

		corefunc_global_var.Time_Stamp_Counter += (unsigned __int64)ceil(time_span.count() / 1 / corefunc_global_var.Input_Hz);  // convert the elpased time from seconds to ms and divide in sample per sec
	}
	else{
		Acquisition_paused = TRUE;
		corefunc_global_var.Time_Stamp_Counter = 0;
		Recalculate_Threshold_For_All_Electrodes(2000, 4.5);
	}

	corefunc_global_var.Start_Sample_Time = corefunc_global_var.Time_Stamp_Counter;

	//Run_DLL();
	StartDAQ_Digital_Input_Thread();
	StartDAQ_Digital_Output_Thread();
	StartDAQ_Analog_Input_Thread();
	StartDAQ_Analog_Output_Thread();


	// write the first timestamp in all events
	for (size_t c = 0; c < corefunc_global_var.Analog_Output_Channels; c++)
	{
		corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].channel = (unsigned short)c;
		corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].timestamp = corefunc_global_var.Time_Stamp_Counter;
		for (size_t i = 0; i < corefunc_global_var.buffer_Output_Per_Channel; i++)
			corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].value[i] = 0.0f;

		corefunc_global_var.Location_of_Last_Event_Analog_Output = (corefunc_global_var.Location_of_Last_Event_Analog_Output < corefunc_global_var.Size_Of_Events_In_Memory) ?
			corefunc_global_var.Location_of_Last_Event_Analog_Output++ : 0;
	}

	corefunc_global_var.Digital_I_Events[corefunc_global_var.Location_of_Last_Event_Digital_I].timestamp = corefunc_global_var.Time_Stamp_Counter;
	corefunc_global_var.Digital_I_Events[corefunc_global_var.Location_of_Last_Event_Digital_I].value = Digital_Data_In;
	corefunc_global_var.Location_of_Last_Event_Digital_I = (corefunc_global_var.Location_of_Last_Event_Digital_I < corefunc_global_var.Size_Of_Events_In_Memory) ?
		corefunc_global_var.Location_of_Last_Event_Digital_I++ : 0;

	corefunc_global_var.Digital_O_Events[corefunc_global_var.Location_of_Last_Event_Digital_O].timestamp = corefunc_global_var.Time_Stamp_Counter;
	corefunc_global_var.Digital_O_Events[corefunc_global_var.Location_of_Last_Event_Digital_O].value = Digital_Data_Out;
	corefunc_global_var.Location_of_Last_Event_Digital_O = (corefunc_global_var.Location_of_Last_Event_Digital_O < corefunc_global_var.Size_Of_Events_In_Memory) ?
		corefunc_global_var.Location_of_Last_Event_Digital_O++ : 0;
	//-------------------


}

void CoreFunction::StartDAQ_Analog_Input_Thread(){
	if (!DAQ_Initialized) return;
	if (CoreFunction::WorkThread_Analog_Input == NULL){

		CoreFunction::WorkThread_Analog_Input = new std::thread([]()
		{
			SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

			std::cout << "Start DAQ Analog Input Thread" << std::endl;
			loop_analog_input = TRUE;

			// Start the acquisition, this is optional because the acquisition starts
			// automatically as soon as the reader starts reading data.
			Analog_Reader->AddEventListener(&eventINListener);
			Analog_InputSession.Start();

			// Arm the reader so that it acquire and store samples in the
			// data array and call our handler once the array is full
			Analog_Reader->ReadMultipleScansAsync(corefunc_global_var.buffer_Input_Per_Channel, Data_In_Pointer);

		}
		);
		WorkThread_Analog_Input->detach();
	}
}

void CoreFunction::StartDAQ_Analog_Output_Thread(){
	if (!DAQ_Initialized) return;

	if (CoreFunction::WorkThread_Analog_Output == NULL){
		CoreFunction::WorkThread_Analog_Output = new std::thread([]()
		{
			SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

			std::cout << "Start DAQ Analog Output Thread" << std::endl;
			loop_analog_output = TRUE;
			// Transmit Zero 
			double *zero = new double[corefunc_global_var.Analog_Output_Channels];
			for (size_t i = 0; i < corefunc_global_var.Analog_Output_Channels; i++)
				zero[i] = 0.0;

			Analog_Writer->WriteSingleScan(zero);
			delete[] zero;

			while (True_forever)
			{
				// Wait until main() sends data
				std::unique_lock<std::mutex> lk(analog_output_mutex);
				analog_output_cv.wait(lk, []{return Analog_Output_ready; });

				if (!CoreFunction::loop_analog_output) break;

				if (Analog_Output_ready){
					double *oldptr;
					//Output_Counter++;
					if (Analog_Output_Flag){
						Analog_Output_Flag = FALSE;
						Data_Out_Pointer = Data_Out_A;
						oldptr = Data_Out_B;
					}
					else{
						Analog_Output_Flag = TRUE;
						Data_Out_Pointer = Data_Out_B;
						oldptr = Data_Out_A;
					}
					Analog_Output_ready = FALSE;

					Analog_Writer->WriteSingleScan(oldptr);
					//Analog_Writer->WriteMultipleScans(corefunc_global_var.buffer_Output_Per_Channel, oldptr);
					//for (int i = 0; i < corefunc_global_var.Analog_Output_Channels; i++)
					//	Analog_Writer->WriteSingleScan(&oldptr[i]);

					lk.unlock();
				}
			}
		}
		);
		WorkThread_Analog_Output->detach();
	}

}

void CoreFunction::StartDAQ_Digital_Output_Thread(){
	if (CoreFunction::WorkThread_Digital_Output == NULL){
		CoreFunction::WorkThread_Digital_Output = new std::thread([]()
		{
			SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

			std::cout << "Start DAQ Digital Output Thread" << std::endl;
			loop_digital_output = TRUE;
			// Transmit Zero 
			UINT16 zero = 0;
			if (DAQ_Initialized)
				Digital_Writer->WriteSingleScan(&zero);

			while (True_forever)
			{
				// Wait until main() sends data
				std::unique_lock<std::mutex> lk(digital_output_mutex);
				digital_output_cv.wait(lk, []{return Digital_Output_ready; });

				if (!CoreFunction::loop_digital_output) break;

				//Archive Digital Output in Memory 			DELTA Save
				corefunc_global_var.Location_of_Last_Event_Digital_O = (corefunc_global_var.Location_of_Last_Event_Digital_O < corefunc_global_var.Size_Of_Events_In_Memory) ?
					corefunc_global_var.Location_of_Last_Event_Digital_O + 1 : 0;

				corefunc_global_var.Digital_O_Events[corefunc_global_var.Location_of_Last_Event_Digital_O].value = Digital_Data_Out;
				corefunc_global_var.Digital_O_Events[corefunc_global_var.Location_of_Last_Event_Digital_O].timestamp = Digital_Data_Out_Time_stamp;
				//-----------------

				if (Digital_Output_ready){
					Digital_Output_ready = FALSE;
					if (DAQ_Initialized)
						Digital_Writer->WriteSingleScan(&Digital_Data_Out);
					lk.unlock();
				}
			}
		}
		);
		WorkThread_Digital_Output->detach();
	}

}

void CoreFunction::StartDAQ_Digital_Input_Thread(){
	if (!DAQ_Initialized) return;
	if (CoreFunction::WorkThread_Digital_Input == NULL){
		CoreFunction::WorkThread_Digital_Input = new std::thread([]()
		{
			SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

			std::cout << "Start DAQ Digital Input Thread" << std::endl;
			loop_digital_input = TRUE;

			while (True_forever)
			{
				// Wait until main() sends data
				std::unique_lock<std::mutex> lk(digital_input_mutex);
				digital_input_cv.wait(lk, []{return Digital_Input_ready; });

				if (!CoreFunction::loop_digital_input) break;

				if (Digital_Input_ready){
					Digital_Input_ready = FALSE;
					Digital_Reader->ReadSingleScan(&Digital_Data_In);
					lk.unlock();
				}

				//Archive Digital Input in Memory 			DELTA Save
				if (corefunc_global_var.Digital_I_Events[corefunc_global_var.Location_of_Last_Event_Digital_I].value != Digital_Data_In){

					corefunc_global_var.Location_of_Last_Event_Digital_I = (corefunc_global_var.Location_of_Last_Event_Digital_I < corefunc_global_var.Size_Of_Events_In_Memory) ?
						corefunc_global_var.Location_of_Last_Event_Digital_I + 1 : 0;

					corefunc_global_var.Digital_I_Events[corefunc_global_var.Location_of_Last_Event_Digital_I].value = Digital_Data_In;
					corefunc_global_var.Digital_I_Events[corefunc_global_var.Location_of_Last_Event_Digital_I].timestamp = corefunc_global_var.Time_Stamp_Counter;
				}
				//----------------------

			}
		}
		);
		WorkThread_Digital_Input->detach();
	}

}

void CoreFunction::Write_Header_File(FILE *Recording_File, int continues){
	// ----- Global Var
	fwrite(&continues, sizeof(int), 1, Recording_File);

	//Time and Date of the record
	time_t t = time(0);   // get time now
	fwrite(&t, sizeof(time_t), 1, Recording_File);

	//Parameters of DAQ
	fwrite(&corefunc_global_var.Analog_Input_Channels, sizeof(int), 1, Recording_File);
	fwrite(&corefunc_global_var.Analog_Output_Channels, sizeof(int), 1, Recording_File);
	fwrite(&corefunc_global_var.Input_bufferSize_for_all_channels, sizeof(int), 1, Recording_File);
	fwrite(&corefunc_global_var.buffer_Input_Per_Channel, sizeof(int), 1, Recording_File);
	fwrite(&corefunc_global_var.buffer_Output_Per_Channel, sizeof(int), 1, Recording_File);
	fwrite(&corefunc_global_var.Input_Hz, sizeof(double), 1, Recording_File);
	fwrite(&corefunc_global_var.Output_Hz, sizeof(double), 1, Recording_File);

	fwrite(&corefunc_global_var.DLL_Slow_Process_Running_Freq, sizeof(float), 1, Recording_File);

	fwrite(&corefunc_global_var.Pre_Trigger_Samples, sizeof(unsigned int), 1, Recording_File);
	fwrite(&corefunc_global_var.Post_Trigger_Samples, sizeof(unsigned int), 1, Recording_File);
	fwrite(&corefunc_global_var.MAX_V, sizeof(double), 1, Recording_File);
	fwrite(&corefunc_global_var.MAX_V, sizeof(double), 1, Recording_File); // backward compatible
}

void CoreFunction::Write_Header_File_In_Text(std::ofstream *Output){

	//Time and Date of the record
	time_t t = time(0);   // get time now
	struct tm now;
	localtime_s(&now, &t);
	*Output << "//  Date " << now.tm_mday << '-' << (now.tm_mon + 1) << '-' << (now.tm_year + 1900) << "\n";
	*Output << "//  time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << now.tm_sec << "\n";

	//Parameters of DAQ
	*Output << "//  Analog_Input_Channels = " << corefunc_global_var.Analog_Input_Channels << "\n";
	*Output << "//  Analog_Output_Channels= " << corefunc_global_var.Analog_Output_Channels << "\n";
	*Output << "//  bufferSize = " << corefunc_global_var.Input_bufferSize_for_all_channels << "\n";
	*Output << "//  buffer_Input_Per_Channel = " << corefunc_global_var.buffer_Input_Per_Channel << "\n";
	*Output << "//  buffer_Output_Per_Channel = " << corefunc_global_var.buffer_Output_Per_Channel << "\n";
	*Output << "//  Input_Hz = " << corefunc_global_var.Input_Hz << "\n";
	*Output << "//  Output_Hz = " << corefunc_global_var.Output_Hz << "\n";
	*Output << "//  DLL_Slow_Process_Running_Freq = " << corefunc_global_var.DLL_Slow_Process_Running_Freq << "\n";
	*Output << "//  Pre_Trigger_Samples = " << corefunc_global_var.Pre_Trigger_Samples << "\n";
	*Output << "//  Post_Trigger_Samples = " << corefunc_global_var.Post_Trigger_Samples << "\n";
	*Output << "//  Max_V = " << corefunc_global_var.MAX_V << "," << corefunc_global_var.MIN_V << "\n";
}

void CoreFunction::Start_Analog_In_Recording(int file_type, char* path, char* filename, __int64 file_counter, int File_Size_in_Mega){

	std::string str;
	do
	{
		file_counter++;
		str = path;
		str.append("\\");
		str.append(filename);
		str.append(".");

		std::stringstream ss;
		ss << std::setw(8) << std::setfill('0') << file_counter;
		str.append(ss.str());

		switch (file_type)
		{
		case 1:
			str.append(corefunc_global_var.Volt_IN_Extention_File);
			break;
		case 0:
			str.append(corefunc_global_var.Spike_Extention_File);
			break;
		case 2:
			str.append(corefunc_global_var.Spike_Time_file);
			break;
		}

		std::ifstream infile(str.c_str());
		if (!infile.good()){
			infile.close();
			break;
		}

	} while (True_forever);


	switch (file_type)
	{
	case 1:
		// Start Header Memory Dump 
		fopen_s(&Volt_Recording_File, str.c_str(), "wb");
		Write_Header_File(Volt_Recording_File, file_type);

		if (Volt_Input_RecordingStarted)
			return;// the thread is already running 
		//start logging thread
		Volt_Input_RecordingStarted = true;
		break;
	case 0:
		// Start Header Memory Dump 
		fopen_s(&Spike_Recording_File, str.c_str(), "wb");
		Write_Header_File(Spike_Recording_File, file_type);

		if (Spike_RecordingStarted)
			return; // the thread already running 
		//start logging thread
		Spike_RecordingStarted = true;
		break;
	case 2:
		// Start Header Memory Dump 
		fopen_s(&Spike_Time_Recording_File, str.c_str(), "wb");
		Write_Header_File(Spike_Time_Recording_File, file_type);

		if (Spike_Time_RecordingStarted)
			return; // the thread already running 
		//start logging thread
		Spike_Time_RecordingStarted = true;
		break;
	}

	switch (file_type)
	{
	case 1:// Start VOLT Recording Thread
		std::thread([path, filename, File_Size_in_Mega, file_counter, file_type]() {
			unsigned __int64 Time_Counter = corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input;
			unsigned __int64 file_size = 0;

			while (Volt_Input_RecordingStarted)
			{
				file_size += Write_Input_Volts(&Time_Counter);
				if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
					file_size = 0;

					fclose(Volt_Recording_File);
					Start_Analog_In_Recording(file_type, path, filename, file_counter, File_Size_in_Mega);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}

			fclose(Volt_Recording_File);

		}).detach();
		break;
	case 0: // Start SPIKE Recording Thread
		std::thread([path, filename, File_Size_in_Mega, file_counter, file_type]() {
			unsigned __int64 Time_Counter = corefunc_global_var.Location_of_Last_Event_Analog_Spike,
				Start_Time = corefunc_global_var.Analog_Spikes_Events[corefunc_global_var.Location_of_Last_Event_Analog_Spike].timestamp;
			unsigned __int64 file_size = 0;

			while (Spike_RecordingStarted)
			{
				file_size += Write_Input_Spikes(&Time_Counter, &Start_Time, (file_type == 0));
				if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
					file_size = 0;
					fclose(Spike_Recording_File);
					Start_Analog_In_Recording(file_type, path, filename, file_counter, File_Size_in_Mega);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}

			fclose(Spike_Recording_File);

		}).detach();
		break;
	case 2:// Start SPIKE time only Recording Thread
		std::thread([path, filename, File_Size_in_Mega, file_counter, file_type]() {
			unsigned __int64 Time_Counter = corefunc_global_var.Location_of_Last_Event_Analog_Spike,
				Start_Time = corefunc_global_var.Analog_Spikes_Events[corefunc_global_var.Location_of_Last_Event_Analog_Spike].timestamp;
			unsigned __int64 file_size = 0;

			while (Spike_Time_RecordingStarted)
			{
				file_size += Write_Input_Spikes(&Time_Counter, &Start_Time, (file_type == 0));
				if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
					file_size = 0;
					fclose(Spike_Time_Recording_File);
					Start_Analog_In_Recording(file_type, path, filename, file_counter, File_Size_in_Mega);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}

			fclose(Spike_Time_Recording_File);

		}).detach();
		break;
	}


	if (file_type == 1) {

	}
	else{

	}


}

void CoreFunction::Start_Analog_Out_Recording(char* path, char* filename, __int64 file_counter, int File_Size_in_Mega){

	std::string str;
	do
	{
		file_counter++;
		str = path;
		str.append("\\");
		str.append(filename);
		str.append(".");

		std::stringstream ss;
		ss << std::setw(8) << std::setfill('0') << file_counter;
		str.append(ss.str());


		//str.append(".A-OUT");
		str.append(corefunc_global_var.Volt_OUT_Extention_File);
		str.append(".txt");
		std::ifstream infile(&str[0u]);// str.c_str());
		if (!infile.good()){
			infile.close();
			break;
		}
	} while (True_forever);


	// Start Header Memory Dump 
	Output_Volt_Recording_File = new std::ofstream();
	Output_Volt_Recording_File->open(&str[0u]);
	Write_Header_File_In_Text(Output_Volt_Recording_File);

	if (Volt_Output_RecordingStarted)
		return;// the thread is already running 

	//start looging thread
	Volt_Output_RecordingStarted = true;

	// Spike Event record
	std::thread([path, filename, File_Size_in_Mega, file_counter]() {
		unsigned __int64 Time_Counter = corefunc_global_var.Location_of_Last_Event_Analog_Output;
		unsigned __int64 Start_Time = corefunc_global_var.Time_Stamp_Counter;
		unsigned __int64  file_size = 0;
		//std::string str;
		//const char* f = filename;

		while (Volt_Output_RecordingStarted)
		{
			file_size += Write_Output_Volts_Events(&Time_Counter, &Start_Time);
			if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
				file_size = 0;

				Output_Volt_Recording_File->close();
				Start_Analog_Out_Recording(path, filename, file_counter, File_Size_in_Mega);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		Output_Volt_Recording_File->close();

	}).detach();

}

void CoreFunction::Start_Digital_Recording(char* path, char* filename, __int64 file_counter, int File_Size_in_Mega){

	std::string str;
	do
	{
		file_counter++;
		str = path;
		str.append("\\");
		str.append(filename);
		str.append(".");

		std::stringstream ss;
		ss << std::setw(8) << std::setfill('0') << file_counter;
		str.append(ss.str());

		str.append(corefunc_global_var.Digital_Extention_File);
		str.append(".txt");
		std::ifstream infile(&str[0u]);// str.c_str());
		if (!infile.good()){
			infile.close();
			break;
		}
	} while (True_forever);


	// Start Header Memory Dump 
	Digital_Recording_File = new std::ofstream();
	Digital_Recording_File->open(&str[0u]);
	Write_Header_File_In_Text(Digital_Recording_File);

	if (Digital_RecordingStarted)
		return; // the thread is already running 
	//start logging thread
	Digital_RecordingStarted = true;

	std::thread([path, filename, File_Size_in_Mega, file_counter]() {
		unsigned __int64 Time_Counter_I, Time_Counter_O;
		Time_Counter_I = corefunc_global_var.Location_of_Last_Event_Digital_I;
		Time_Counter_O = corefunc_global_var.Location_of_Last_Event_Digital_O;
		unsigned __int64  file_size = 0;

		while (Digital_RecordingStarted)
		{
			file_size += Write_Digital(&Time_Counter_I, &Time_Counter_O);
			if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
				file_size = 0;

				Digital_Recording_File->close();
				Start_Digital_Recording(path, filename, file_counter, File_Size_in_Mega);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		Digital_Recording_File->close();

	}).detach();


}

void CoreFunction::Start_UserData_Recording(char* path, char* filename, __int64 file_counter, int File_Size_in_Mega){

	std::string str;
	do
	{
		file_counter++;
		str = path;
		str.append("\\");
		str.append(filename);
		str.append(".");

		std::stringstream ss;
		ss << std::setw(8) << std::setfill('0') << file_counter;
		str.append(ss.str());

		str.append(corefunc_global_var.UserDataFile_Extention_File);
		str.append(".txt");
		std::ifstream infile(&str[0u]);// str.c_str());
		if (!infile.good()){
			infile.close();
			break;
		}
	} while (True_forever);


	// Start Header Memory Dump 
	UserData_Recording_File = new std::ofstream();
	UserData_Recording_File->open(&str[0u]);
	Write_Header_File_In_Text(UserData_Recording_File);

	if (UserData_RecordingStarted)
		return; // the thread is already running 
	//start logging thread
	UserData_RecordingStarted = true;

	std::thread([path, filename, File_Size_in_Mega, file_counter]() {
		unsigned __int64 User_Data_P = corefunc_global_var.Location_of_Last_Event_User_Analog;
		unsigned __int64  file_size = 0;

		while (UserData_RecordingStarted)
		{
			file_size += Write_UserData(&User_Data_P);
			if (file_size / 1000000 >= (unsigned __int64)File_Size_in_Mega){
				file_size = 0;

				UserData_Recording_File->close();
				Start_UserData_Recording(path, filename, file_counter, File_Size_in_Mega);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		UserData_Recording_File->close();

	}).detach();


}

void CoreFunction::Start_Restart_Playback(){

	Playback_pause = false;

	if (PlaybackStarted){
		PlaybackStarted = false;
		Sleep(100);
	}

	Reset_Head_Pointers();

	if (Init_Playback(&Playback_Current_Path[0u], &Playback_Filename_First[0u], Playback_Counter_First, Playback_Current_Until_Counter, Playback_File_Type_First, TRUE) == -1)
		return;

	PlaybackStarted = true;

	// start/stop Digital Output thread 
	StopDAQ();
	StartDAQ_Digital_Output_Thread();


	// run the playback in different thread
	std::thread t = std::thread([]() {
		double* Electrode_Spike_Data = NULL;
		unsigned __int64* Electrod_ptr = NULL;
		__int64* counter_electrode_spike = NULL;

		if (Playback_Current_File_Type != 1){
			Electrode_Spike_Data = new double[Spike_VoltTrace_size * corefunc_global_var.Analog_Input_Channels];
			std::uninitialized_fill(Electrode_Spike_Data, Electrode_Spike_Data + Spike_VoltTrace_size * corefunc_global_var.Analog_Input_Channels, 0.0);
			Electrod_ptr = new unsigned __int64[corefunc_global_var.Analog_Input_Channels];
			counter_electrode_spike = new __int64[corefunc_global_var.Analog_Input_Channels];
			for (size_t i = 0; i < corefunc_global_var.Analog_Input_Channels; i++)
			{
				counter_electrode_spike[i] = -1;
				Electrod_ptr[i] = i * Spike_VoltTrace_size;
			}
		}

		int itereation_in_millisecond = (int)ceil(corefunc_global_var.Input_Hz / corefunc_global_var.buffer_Input_Per_Channel);

		size_t num_of_buf_read = 0;
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		char str[100];
		sprintf_s(str, 100, "Playing back file number %08llu", Playback_Current_Counter);
		WriteMessage(str, FALSE);

		// real data
		while (PlaybackStarted)
		{
			if (!Playback_pause){
				for (size_t i = 0; i < itereation_in_millisecond; i++){
					num_of_buf_read = Playback_From_File(Electrode_Spike_Data, Electrod_ptr, counter_electrode_spike);

					if (num_of_buf_read == 0)
					{ // file ended!

						PlaybackStarted = false;
						if (Playback_Input_File == 0) continue;
						fclose(Playback_Input_File);
						Playback_Input_File = NULL;

						int file_open = 0;
						Playback_Current_Counter++;

						if (Playback_Current_Until_Counter > Playback_Current_Counter){
							// initilize new file 
							file_open = Init_Playback(&Playback_Current_Path[0u], &Playback_Current_Filename[0u], Playback_Current_Counter, Playback_Current_Until_Counter, Playback_Current_File_Type, FALSE);

							char str1[100];
							sprintf_s(str1, 100, "Playing back file number %08llu", Playback_Current_Counter);
							WriteMessage(str1, FALSE);

							if (file_open == -1){
								PlaybackStarted = false;
								break;
							}
							PlaybackStarted = true;
						}
						else{
							char str1[100];
							sprintf_s(str1, 100, "Playing back file reach to end number %08llu", Playback_Current_Counter);
							WriteMessage(str1, FALSE);
							break;
						}
						// if OK then PlaybackStarted will be true and the loop will continue
						// else PlaybackStarted will be false and the loop will stop
					}
					else{
						CallBackWork();
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(itereation_in_millisecond));
		}

		delete[] Electrode_Spike_Data;
		delete[] Electrod_ptr;
		delete[] counter_electrode_spike;

		if (Playback_Input_File != NULL){
			fclose(Playback_Input_File);
			Playback_Input_File = NULL;
		}

	});
	t.detach();
}

size_t CoreFunction::Playback_From_File(double *Electrode_Spike_Data, unsigned __int64 *Electrod_ptr, __int64 *counter_electrode_spike){
	size_t read_counter = 0;
	// Read File	

	if (Playback_Current_File_Type == 1){  // Volt 
		// short reading 
		short *data = new short[corefunc_global_var.Input_bufferSize_for_all_channels];
		read_counter = fread(data, sizeof(short), corefunc_global_var.Input_bufferSize_for_all_channels, Playback_Input_File);
		if (read_counter == 0){
			delete[] data;
			return read_counter;
		}

		for (unsigned __int64 t = 0; t < corefunc_global_var.Input_bufferSize_for_all_channels; t++)
			Data_In_Pointer[t] = (double)data[t] * OneBitIsEqualTo;


		delete[] data;
	}
	else{  // Spike

		unsigned __int64 Next_Time_Step = corefunc_global_var.Time_Stamp_Counter + corefunc_global_var.buffer_Input_Per_Channel;
		// check to see if there are more spikes to put in the buffer
		while (True_forever)
		{
			unsigned __int64 time_stamp;
			// read the first line that contain time stamp
			size_t t = fread(&time_stamp, sizeof(unsigned __int64), 1, Playback_Input_File);
			if (t == 0)
				return 0;

			read_counter += t;

			if (time_stamp < Next_Time_Step){
				unsigned short electrode;
				read_counter += fread(&electrode, sizeof(unsigned short), 1, Playback_Input_File);
				counter_electrode_spike[electrode] = 0;

				playback_TimeStamp_of_spike[electrode].push_back(time_stamp);

				if (Playback_Current_File_Type == 0){
					// Read Envelope
					short *temp = new short[Spike_VoltTrace_size];
					read_counter += fread(temp, sizeof(short), Spike_VoltTrace_size, Playback_Input_File);
					for (size_t i = 0, c = Electrod_ptr[electrode]; i < Spike_VoltTrace_size; c++, i++)
						Electrode_Spike_Data[c] = (double)temp[i] * OneBitIsEqualTo;
					delete[] temp;
				}
				else if (Playback_Current_File_Type == 2){
					// Assign Synthetic spike
					for (size_t i = 0, c = Electrod_ptr[electrode]; i < Spike_VoltTrace_size; c++, i++)
						Electrode_Spike_Data[c] = Synthetic_Spike[i];
				}

			}
			else{
				// Need to rewind and return the same position of the buffers
				long tf = sizeof(unsigned __int64);
				fseek(Playback_Input_File, -tf, SEEK_CUR);
				break;
			}

		}


		// if there is data in buffer, put it in Data_In_Pointer
		for (size_t counter = 0, buff_counter = 0; buff_counter < corefunc_global_var.buffer_Input_Per_Channel; buff_counter++)
		{
			for (size_t e = 0; e < corefunc_global_var.Analog_Input_Channels; e++, counter++){
				if (counter_electrode_spike[e] > -1){

					// put small value to make it not zero (0.0)
					Data_In_Pointer[counter] = Electrode_Spike_Data[counter_electrode_spike[e] + Electrod_ptr[e]];

					counter_electrode_spike[e]++;
					if ((unsigned __int64)counter_electrode_spike[e] == Spike_VoltTrace_size)
						counter_electrode_spike[e] = -1;
				}
				else
					Data_In_Pointer[counter] = 0.0;
			}
		}
	}
	return read_counter;
}

void CoreFunction::StopDAQ(){
	if (CoreFunction::WorkThread_Analog_Input != NULL){
		loop_analog_input = FALSE;
		Sleep(10);
		Analog_Reader->AddEventListener(NULL);
		Analog_InputSession.Stop();
		delete CoreFunction::WorkThread_Analog_Input;
		CoreFunction::WorkThread_Analog_Input = NULL;
	}

	if (CoreFunction::WorkThread_Analog_Output != NULL){
		loop_analog_output = FALSE;
		{
			std::lock_guard<std::mutex> lk(analog_output_mutex);
			Analog_Output_ready = TRUE;
		}
		analog_output_cv.notify_one();
		Sleep(10);
		delete CoreFunction::WorkThread_Analog_Output;
		CoreFunction::WorkThread_Analog_Output = NULL;
	}

	if (CoreFunction::WorkThread_Digital_Output != NULL){
		loop_digital_output = FALSE;
		{
			std::lock_guard<std::mutex> lk(digital_output_mutex);
			Digital_Output_ready = TRUE;
		}
		digital_output_cv.notify_one();
		Sleep(10);
		delete CoreFunction::WorkThread_Digital_Output;
		CoreFunction::WorkThread_Digital_Output = NULL;
	}

	if (CoreFunction::WorkThread_Digital_Input != NULL){
		loop_digital_input = FALSE;
		{
			std::lock_guard<std::mutex> lk(digital_input_mutex);
			Digital_Input_ready = TRUE;
		}
		digital_input_cv.notify_one();
		Sleep(10);
		delete CoreFunction::WorkThread_Digital_Input;
		CoreFunction::WorkThread_Digital_Input = NULL;
	}


	StopDAQ_Thread();

}

void CoreFunction::StopDAQ_Thread(){

	Paused = std::chrono::high_resolution_clock::now();

}

void CoreFunction::Recalculate_Threshold_For(int electrod, int Threshold_Timer_in_Milisecond, double RMS_Factor){
	corefunc_global_var.Calculating_Threshold++;
	unsigned __int64 iteration = (unsigned __int64)ceil(Threshold_Timer_in_Milisecond / (1000 / corefunc_global_var.Input_Hz));
	avg_per_electrod[electrod] = new double[iteration];	
	offset_per_electrod[electrod] = 0;
	calculate_offset_per_electrod[electrod] = 0;
	Threshold_Max_Counter[electrod] = iteration;
	elements_Counter_for_RMS[electrod] = 0;
	corefunc_global_var.RMS_Factor = RMS_Factor;
	ReCalculate_Threshold_AT_Electrode[electrod] = TRUE;
}

void CoreFunction::Recalculate_Threshold_For_All_Electrodes(int Threshold_Timer_in_Milisecond, double RMS_Factor){
	for (int i = 0; i < corefunc_global_var.Analog_Input_Channels; i++)
		Recalculate_Threshold_For(i, Threshold_Timer_in_Milisecond, RMS_Factor);
}

void CoreFunction::DAQ_CleanUP(){

	// client DLL 
	if (loop_DLL_RealTime || loop_DLL_On_Demond)
		Stop_DLL();

	if (PlaybackStarted)
		PlaybackStarted = false;
	// Stop_Playback(); //!!!!!!!!

	if (Volt_Input_RecordingStarted)
		Volt_Input_RecordingStarted = false;

	if (Volt_Output_RecordingStarted)
		Volt_Output_RecordingStarted = false;

	if (Spike_RecordingStarted)
		Spike_RecordingStarted = false;

	if (Spike_Time_RecordingStarted)
		Spike_Time_RecordingStarted = false;

	if (Digital_RecordingStarted)
		Digital_RecordingStarted = false;


	// DAQ
	if (loop_analog_output || loop_analog_input || loop_digital_output || loop_digital_input)
		StopDAQ();

	if (DAQ_Initialized){
		Analog_InputSession.Stop();
		Analog_InputSession.CleanUp();

		Analog_OutputSession.Stop();
		Analog_OutputSession.CleanUp();

		Digital_OutputSession.Stop();
		Digital_OutputSession.CleanUp();

		Digital_InputSession.Stop();
		Digital_InputSession.CleanUp();

		DAQ_Initialized = FALSE;
		//delete DAQ_Device_info;
		delete Analog_Reader;
		delete Analog_Writer;
		delete Digital_Reader;
		delete Digital_Writer;
		delete[] Data_In_A;
		delete[] Data_In_B;
		Data_In_Pointer = NULL;

		delete[] Data_Out_A;
		delete[] Data_Out_B;
		Data_Out_Pointer = NULL;

	}
}

void CoreFunction::Memory_CleanUP(){

	// client DLL 
	if (loop_DLL_RealTime || loop_DLL_On_Demond){
		Stop_DLL();
		ReleaseDLL();
	}

	// Memory
	if (Memory_Initialized){
		delete[] Global_Spikes_volt_around_spike;
		delete[] Global_analog_volt_around_event;
		delete[] Global_User_Event;
		delete[] corefunc_global_var.Analog_Spikes_Events;
		delete[] corefunc_global_var.Analog_Output_Events;
		delete[] corefunc_global_var.User_Analog_Events;

		delete[] corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts;
		delete[] corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts;
		delete[] corefunc_global_var.RAW_Input_Volts;
		delete[] Data_From_Last_Iteration;
		delete[] corefunc_global_var.RAW_Output_Volts;
		delete[] corefunc_global_var.Time_Stamp_Counter_Array;
		delete[] corefunc_global_var.Digital_I_Events;
		delete[] corefunc_global_var.Digital_O_Events;

		delete[] avg_per_electrod;
		delete[] calculate_offset_per_electrod;
		delete[] elements_Counter_for_RMS;

		if (backup_threshold_Per_Electrod != NULL){
			delete[] backup_threshold_Per_Electrod; //Noam: program crashes here - attempt to write beyon heap
			backup_threshold_Per_Electrod = corefunc_global_var.threshold_Per_Electrod;
			delete[] backup_offset_per_electrod;
			backup_offset_per_electrod = offset_per_electrod;
		}
		else{
			backup_threshold_Per_Electrod = corefunc_global_var.threshold_Per_Electrod;
			backup_offset_per_electrod = offset_per_electrod;
		}

		for (int i = 0; i < corefunc_global_var.Analog_Input_Channels; i++)
			Envelope_Counter[i].clear();
		delete[] Envelope_Counter;

		delete[] Spike_reArm_Counter;
		delete[] ReCalculate_Threshold_AT_Electrode;
		delete[] Threshold_Max_Counter;
		delete[] playback_TimeStamp_of_spike;

		Memory_Initialized = FALSE;
	}

}

int CoreFunction::Load_DLL(char* s){
	hInstLibrary = LoadLibrary(_T(s));
	if (hInstLibrary)
	{
		std::string pout;

		// Set up our function pointers.
		__ClientIOfun_RealTime_CL = (ClientIOfun)GetProcAddress(hInstLibrary, "RealTime_CL");
		if (!__ClientIOfun_RealTime_CL){
			pout = "DLL Warning - RealTime (RealTime_CL) Function not found!!";
			WriteMessage(&pout[0u], FALSE);
		}
		__ClientIOfun_SlowPeriodic_CL = (ClientIOfun)GetProcAddress(hInstLibrary, "SlowPeriodic_CL");
		if (!__ClientIOfun_SlowPeriodic_CL){
			pout = "DLL Warning - Slow Process (SlowPeriodic_CL) Function not found!!";
			WriteMessage(&pout[0u], FALSE);
		}
		__ClientIOfun_DLL_Engage = (ClientIOfun_Engage)GetProcAddress(hInstLibrary, "DLL_Engage");
		if (!__ClientIOfun_DLL_Engage){
			pout = "DLL Warning - Engage Function not found!!";
			WriteMessage(&pout[0u], FALSE);
		}
		__ClientIOfun_DLL_Disengage = (ClientIOfun_Disengage)GetProcAddress(hInstLibrary, "DLL_Disengage");
		if (!__ClientIOfun_DLL_Disengage){
			pout = "DLL Warning - Disengage Function not found!!";
			WriteMessage(&pout[0u], FALSE);
		}
		__initDLL = (iniDLL)GetProcAddress(hInstLibrary, "DLL_Init");
		if (__initDLL){
			char str[MAXTEXTMESSAGELEN];
			std::uninitialized_fill(str, str + MAXTEXTMESSAGELEN, '\0');

			// Note: If you want to Debug the INIT DLL put the breaking point in the next line!	
			__initDLL(&corefunc_global_var, str);

			if (str[0] != '\0'){
				WriteMessage(str, FALSE);
			}
		}
		else{
			pout = "DLL Warning - Initialization Function not found!!";
			WriteMessage(&pout[0u], FALSE);
		}

		int Analog_output_channel = corefunc_global_var.Analog_Output_Channels;
		int analog_output_size_buffer = Analog_output_channel * corefunc_global_var.buffer_Output_Per_Channel;
		Local_Last_Analog_Data_Out = new double[analog_output_size_buffer];
		Local_Analog_Data_Out = new double[analog_output_size_buffer];
		Local_Analog_User_Data = new float[corefunc_global_var.Number_of_User_Data_Channels];

		// save output buffer to compare later
		std::uninitialized_fill(Local_Last_Analog_Data_Out, Local_Last_Analog_Data_Out + analog_output_size_buffer, (double)0.0);
		std::uninitialized_fill(Local_Analog_Data_Out, Local_Analog_Data_Out + analog_output_size_buffer, (double)0.0);
		std::uninitialized_fill(Local_Analog_User_Data, Local_Analog_User_Data + corefunc_global_var.Number_of_User_Data_Channels, (float)0.0);
		Local_Digital_Data_Out = 0;

		return 0;
	}
	else
	{
		// Our DLL failed to load!
		std::string pout = "DLL Failed To Load!";
		WriteMessage(&pout[0u], FALSE);
		return 1;
	}
}

int CoreFunction::Run_DLL(char* args_c){
	// This function spawns two threads, one the realtime DLL the other one is the Client DLL
	// The Real time DLL is responsble on archiving and transmitting the output data (Analog Digital and User Data)
	// The User DLL have the same variable as Realtime DLL, so the last function to write to the resource is the winner

	if (!hInstLibrary || loop_DLL_RealTime || loop_DLL_On_Demond || CoreFunction::WorkThread_DLL_RealTime != NULL)
		return 1;

	// Parse the arguments and Engage DLL
	int argc = 0;
	char **argv;

	int		arg_str_length, i, n, p, letterfound = 0;

	arg_str_length = (int) strlen(args_c);
								 
															   

	// get number of arguments;
	for (i = 0; i < arg_str_length ; ++i)
	{
		if (isspace ((unsigned char) args_c[i]))
			letterfound = 0;
		else
		{
			if (letterfound == 0)
				argc++;
			letterfound = 1;
		}
	}

	i = 0;
	if (argc == 0)	// a precaution just in case a plugin function attempts to read a non existant string
	{
		argv = new char*[1];
										  
		argv[0] = args_c;
	}
	else
	{
		argv = new char*[argc];
		for (n = 0; n < argc; ++n)
		{
			while (isspace((unsigned char)args_c[i]))
				i++;
			p = i;
			while (!isspace((unsigned char)args_c[p]))
				p++;
			argv[n] = new char[p - i + 1];
			strncpy_s(argv[n], p - i + 1, args_c + i, p - i);
									

											
			argv[n][p - i] = 0;
			i = p;
	   
						  
		}
	}
	 
					  

	char tout[MAXTEXTMESSAGELEN];
	std::uninitialized_fill(tout, tout + MAXTEXTMESSAGELEN, '\0');
	// Note: If you want to Debug the Engage DLL put the breakpoint in the next line!	
	if (__ClientIOfun_DLL_Engage)
		__ClientIOfun_DLL_Engage(argc, argv, tout);

			  
	for (n = 0; n < argc; ++n)
		delete[] argv[n];
	 
	delete[] argv;

	if (tout[0] != '\0'){
		WriteMessage(tout, FALSE);
	}

	/// Real time DLL
	CoreFunction::WorkThread_DLL_RealTime = new std::thread([]()
	{
		int Analog_output_channel = corefunc_global_var.Analog_Output_Channels;

		loop_DLL_RealTime = TRUE;

		SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);
		char pout[MAXTEXTMESSAGELEN];

		while (True_forever)
		{
			// Wait until main() sends data
			std::unique_lock<std::mutex> lk_DLL(DLL_mutex_RealTime);
			DLL_cv_RealTime.wait(lk_DLL, []{return Input_Ready_To_Process; });
			std::uninitialized_fill(pout, pout + MAXTEXTMESSAGELEN, '\0');
			UINT64 current_timeStamp = corefunc_global_var.Time_Stamp_Counter;
			
			// Run RealTime_CL!
																						  
			if (__ClientIOfun_RealTime_CL)
				// Note: If you want to Debug the Real time DLL put the breakpoint in the next line!			
				__ClientIOfun_RealTime_CL(Local_Analog_Data_Out, Local_Analog_User_Data, &Local_Digital_Data_Out, pout);

			Input_Ready_To_Process = FALSE;
			lk_DLL.unlock();

			if (pout[0] != '\0'){
				WriteMessage(pout, FALSE);
			}

			if (!loop_DLL_RealTime) break;

			//Archive Digital Output in Memory 			DELTA Save
			if (corefunc_global_var.Digital_O_Events[corefunc_global_var.Location_of_Last_Event_Digital_O].value != Local_Digital_Data_Out){
				// send data to the Output thread
				if (!Digital_Output_ready){
					{
						std::lock_guard<std::mutex> lk(digital_output_mutex);
						Digital_Data_Out = Local_Digital_Data_Out;
						Digital_Data_Out_Time_stamp = current_timeStamp;
						Digital_Output_ready = TRUE;
					}
					digital_output_cv.notify_one();
				}
			}


			// Archive RAW Volt Output in Memory 
			for (size_t j = 0, memory_counter = 0; j < corefunc_global_var.buffer_Output_Per_Channel; j++)
			{
				for (size_t i = 0; i < corefunc_global_var.Analog_Output_Channels; memory_counter++, i++)
					corefunc_global_var.RAW_Output_Volts[corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output + corefunc_global_var.Pointer_To_Electrode_In_RAW_Output_Volts[i]] = (float)Local_Analog_Data_Out[memory_counter];

				corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output = (corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output < corefunc_global_var.Samples_per_Channel) ?
					corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Output + 1 : 0;

			}

			// Archive Volt Output Events in Memory 
			for (size_t c = 0; c < Analog_output_channel; c++)
			{
				bool changes = FALSE;
				for (size_t b = 0, index = c * corefunc_global_var.buffer_Output_Per_Channel; b < corefunc_global_var.buffer_Output_Per_Channel; b++, index++)
				{
					// ready the data in the event
					corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].value[b] = (float)Local_Analog_Data_Out[index];

					if (DAQ_Initialized){
						// ready data to send to DAQ
						Data_Out_Pointer[index] = Local_Analog_Data_Out[index];
					}

					// COPY and save event for next time
					if (Local_Last_Analog_Data_Out[index] != Local_Analog_Data_Out[index]){
						// save it to next iteration
						Local_Last_Analog_Data_Out[index] = Local_Analog_Data_Out[index];
						changes = TRUE;
					}
				}
				if (changes){
					corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].channel = (unsigned short)c;
					corefunc_global_var.Analog_Output_Events[corefunc_global_var.Location_of_Last_Event_Analog_Output].timestamp = current_timeStamp;

					corefunc_global_var.Location_of_Last_Event_Analog_Output = (corefunc_global_var.Location_of_Last_Event_Analog_Output < corefunc_global_var.Size_Of_Events_In_Memory) ?
						corefunc_global_var.Location_of_Last_Event_Analog_Output + 1 : 0;

					// send data to the Output thread
					if ((DAQ_Initialized) && (!Analog_Output_ready)){
						{
							std::lock_guard<std::mutex> lk(analog_output_mutex); \
								Analog_Output_ready = TRUE;
						}
						analog_output_cv.notify_one();
					}
				}
			}

			// Archive User Analog Events in Memory 
			bool flag = false;
			for (size_t i = 0; i < corefunc_global_var.Number_of_User_Data_Channels; i++)
				if (corefunc_global_var.User_Analog_Events[corefunc_global_var.Location_of_Last_Event_User_Analog].value[i] != Local_Analog_User_Data[i]) {
					flag = true;
					break;
				}
			if (flag){
				corefunc_global_var.Location_of_Last_Event_User_Analog = (corefunc_global_var.Location_of_Last_Event_User_Analog < corefunc_global_var.Size_Of_Events_In_Memory) ?
					corefunc_global_var.Location_of_Last_Event_User_Analog + 1 : 0;

				corefunc_global_var.User_Analog_Events[corefunc_global_var.Location_of_Last_Event_User_Analog].timestamp = current_timeStamp;
				for (size_t i = 0; i < corefunc_global_var.Number_of_User_Data_Channels; i++)
					corefunc_global_var.User_Analog_Events[corefunc_global_var.Location_of_Last_Event_User_Analog].value[i] = Local_Analog_User_Data[i];
			}

		}
	}
	);
	WorkThread_DLL_RealTime->detach();


	// Periodic DLL
	CoreFunction::WorkThread_DLL_On_Demond = new std::thread([]()
	{
		loop_DLL_On_Demond = TRUE;

		char pout[MAXTEXTMESSAGELEN];

		SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

		unsigned int sleepping_in_ms = (unsigned int)std::round((1 / corefunc_global_var.DLL_Slow_Process_Running_Freq) * 1000.0); ///  Loaded from the init DLL or the engade botton
		UINT64 prev_time_Stamp = corefunc_global_var.Time_Stamp_Counter;
		double One_iteration_in_ms = (1.0 / corefunc_global_var.Input_Hz) * 1000.0;

		while (True_forever)
		{

			// calculate the time spent on running the slow process dll and deduct it from the sleeping time.
			unsigned int delta_time_Stamp_in_ms = (unsigned int)std::round((corefunc_global_var.Time_Stamp_Counter - prev_time_Stamp) * One_iteration_in_ms);
			unsigned int time_left_to_sleep_in_ms = (delta_time_Stamp_in_ms >= sleepping_in_ms) ? 0 : sleepping_in_ms - delta_time_Stamp_in_ms;
			prev_time_Stamp = corefunc_global_var.Time_Stamp_Counter;

			UINT64 sleep_until_Time_Stamp_Counter = (unsigned int)std::round(corefunc_global_var.Time_Stamp_Counter + (time_left_to_sleep_in_ms / One_iteration_in_ms));

			while (True_forever)
			{
				if (!loop_DLL_On_Demond) break;
				if (sleep_until_Time_Stamp_Counter <= corefunc_global_var.Time_Stamp_Counter) break;
				Sleep(250);
			}

			std::uninitialized_fill(pout, pout + MAXTEXTMESSAGELEN, '\0');
			
			// Run SlowPeriodic_CL!
																							 

			if (__ClientIOfun_SlowPeriodic_CL)
				// Note: If you want to Debug the Slow PeriodicDLL put the breakpoint in the next line!			
				__ClientIOfun_SlowPeriodic_CL(Local_Analog_Data_Out, Local_Analog_User_Data, &Digital_Data_Out, pout);

			if (pout[0] != '\0'){
				WriteMessage(pout, FALSE);
			}

		if (!loop_DLL_On_Demond)
				break;								   
		}

	}
	);
	WorkThread_DLL_On_Demond->detach();
	return 0;
}

int CoreFunction::Stop_DLL(){
	if (hInstLibrary){
		//loop_DLL = FALSE;

		if (CoreFunction::WorkThread_DLL_RealTime != NULL){
			{
				loop_DLL_RealTime = false;

				std::lock_guard<std::mutex> lk(DLL_mutex_RealTime);
				Input_Ready_To_Process = TRUE;
			}
			DLL_cv_RealTime.notify_one();

			Sleep(250);

			//CoreFunction::WorkThread_DLL_RealTime->join();
			delete CoreFunction::WorkThread_DLL_RealTime;
			CoreFunction::WorkThread_DLL_RealTime = NULL;
		}

		if (CoreFunction::WorkThread_DLL_On_Demond != NULL){

			loop_DLL_On_Demond = false;
			Sleep(400);

			//CoreFunction::WorkThread_DLL_On_Demond->join();

			delete CoreFunction::WorkThread_DLL_On_Demond;
			CoreFunction::WorkThread_DLL_On_Demond = NULL;
		}


		char pout[MAXTEXTMESSAGELEN];
		std::uninitialized_fill(pout, pout + MAXTEXTMESSAGELEN, '\0');
		
		// Note: If you want to Debug the Disengage DLL put the breakpoint in the next line!	
		if (__ClientIOfun_DLL_Disengage)
			__ClientIOfun_DLL_Disengage(pout);

		if (pout[0] != '\0'){
			WriteMessage(pout, FALSE);
		}

		return 0;
	}
	else
		return 1;
}

int CoreFunction::ReleaseDLL(){

	BOOL	result;

	Stop_DLL();

	if (hInstLibrary)
	{
		__ClientIOfun_RealTime_CL = NULL;
		__ClientIOfun_SlowPeriodic_CL = NULL;
		__ClientIOfun_DLL_Engage = NULL;
		__ClientIOfun_DLL_Disengage = NULL;
		__initDLL = NULL;
		// We're done with the DLL so we need to release it from memory.
		result = FreeLibrary(hInstLibrary);
		// clean up allocated memory 
		delete[] Local_Last_Analog_Data_Out;
		delete[] Local_Analog_Data_Out;
		delete[] Local_Analog_User_Data;
		Local_Last_Analog_Data_Out = NULL;
		Local_Analog_Data_Out = NULL;
		Local_Analog_User_Data = NULL;


		if (result)
		{
			char pout[MAXTEXTMESSAGELEN] = "Plugin DLL Released";
			WriteMessage(pout, FALSE);
		}
		else
		{
			char pout[MAXTEXTMESSAGELEN] = "Plugin DLL NOT released successfully ";
			WriteMessage(pout, FALSE);
		}

		return 0;
	}
	else
		return 1;
}

void CoreFunction::Convert_Binary_Log_to_Text_Log(char* Source_Path, char* Source_Filename, BOOL continues, unsigned __int64 Start_counter, unsigned __int64 End_counter, char* Target_Path, char* Target_Filename){

	std::string S_Path = Source_Path;
	std::string S_Filename = Source_Filename;
	std::string T_Path = Target_Path;
	std::string T_Filename = Target_Filename;

	CoreFunction::WorkThread_File_Convertor = new std::thread([
		S_Path, S_Filename, T_Path, T_Filename, continues, Start_counter, End_counter]()
	{
		corefunc_global_var.Convertor_Start = TRUE;
		corefunc_global_var.Force_Convertor_to_Stop = FALSE;

		for (size_t file_number = Start_counter; file_number < End_counter + 1; file_number++)
		{

			std::string s = "Start Converting file ";
			s.append(S_Filename);
			char *msg = &s[0u];
			WriteMessage(msg, FALSE);

			std::string Bin_Input, Text_Output;

			Bin_Input = S_Path;
			Bin_Input.append(S_Filename);
			Bin_Input.append(".");

			std::stringstream ss;
			ss << std::setw(8) << std::setfill('0') << file_number;
			Bin_Input.append(ss.str());


			if (continues)
				Bin_Input.append(corefunc_global_var.Volt_IN_Extention_File);
			else
				Bin_Input.append(corefunc_global_var.Spike_Extention_File);
			std::ifstream infile(Bin_Input);
			if (!infile.good()){
				infile.close();
				break;
			}

			Text_Output = T_Path;
			Text_Output.append("\\");
			Text_Output.append(T_Filename);
			Text_Output.append(".");

			ss.clear();
			ss << std::setw(8) << std::setfill('0') << file_number;
			Text_Output.append(ss.str());

			Text_Output.append(".txt");

			FILE *Input;
			std::ofstream Output;

			fopen_s(&Input, &Bin_Input[0u], "rb");
			if (Input == NULL) { fputs("File error", stderr); exit(1); }

			Output.open(Text_Output);

			int dont_care_int;
			unsigned int dont_care_uint;
			double dont_care_double;
			double dont_care_double_array[2];

			UINT Continues_Playback;

			// read all parameters
			// ----- Global Var
			fread(&Continues_Playback, sizeof(int), 1, Input);

			//Time and Date of the record
			time_t t = time(0);   // get time now
			fread(&t, sizeof(time_t), 1, Input);
			struct tm now;
			localtime_s(&now, &t);
			Output << "//  Date " << now.tm_mday << '-' << (now.tm_mon + 1) << '-' << (now.tm_year + 1900) << "\n";
			Output << "//  time " << (now.tm_hour) << ':' << (now.tm_min) << ':' << now.tm_sec << "\n";

			//Parameters of DAQ
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  Analog_Input_Channels = " << dont_care_int << "\n";
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  Analog_Output_Channels= " << dont_care_int << "\n";
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  bufferSize = " << dont_care_int << "\n";
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  buffer_Input_Per_Channel = " << dont_care_int << "\n";
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  buffer_Output_Per_Channel = " << dont_care_int << "\n";
			fread(&dont_care_double, sizeof(double), 1, Input);
			Output << "//  Input_Hz = " << dont_care_double << "\n";
			fread(&dont_care_double, sizeof(double), 1, Input);
			Output << "//  Output_Hz = " << dont_care_double << "\n";
			fread(&dont_care_int, sizeof(int), 1, Input);
			Output << "//  DLL_Slow_Process_Running_Freq = " << dont_care_int << "\n";

			unsigned int temp_Spike_VoltTrace_size = 0;
			fread(&dont_care_uint, sizeof(unsigned int), 1, Input);
			Output << "//  Pre_Trigger_Samples = " << dont_care_uint << "\n";
			temp_Spike_VoltTrace_size += dont_care_uint;

			fread(&dont_care_uint, sizeof(unsigned int), 1, Input);
			Output << "//  Post_Trigger_Samples = " << dont_care_uint << "\n";
			temp_Spike_VoltTrace_size += dont_care_uint;

			fread(&dont_care_double_array, sizeof(double), 2, Input);
			Output << "//  Max_V = " << dont_care_double_array[0] << ",  Min_V =" << dont_care_double_array[1] << "\n";


			short data;
			if (continues)// Volt 
				while (fread(&data, sizeof(short), 1, Input) > 0)
				{
					Output << "," << (float)data  * OneBitIsEqualTo;

					if (corefunc_global_var.Force_Convertor_to_Stop)
						break;
				}
			else// Spike
				while (True_forever)
				{
					unsigned __int64 time_stamp;
					if (fread(&time_stamp, sizeof(unsigned __int64), 1, Input) == 0) break;

					unsigned short electrode;
					if (fread(&electrode, sizeof(unsigned short), 1, Input) == 0) break;

					short *volt = new short[temp_Spike_VoltTrace_size];
					size_t wb = 0;
					std::cout << ".";

					wb = fread(volt, sizeof(short), temp_Spike_VoltTrace_size, Input);

					// write volt 
					Output << time_stamp << "," << electrode + 1 << "," << (float)volt[0] * OneBitIsEqualTo;
					for (size_t i = 1; i < temp_Spike_VoltTrace_size; i++)
						Output << "," << (float)volt[i] * OneBitIsEqualTo;

					Output << "\n";

					delete[] volt;
					if (wb == 0)
						break;

					if (corefunc_global_var.Force_Convertor_to_Stop)
						break;
				}

			fclose(Input);
			Output.close();


			if (corefunc_global_var.Force_Convertor_to_Stop)
				break;
		}
		std::string s = "Done Converting";
		char *msg = &s[0u];
		WriteMessage(msg, FALSE);

		corefunc_global_var.Convertor_Start = FALSE;
		corefunc_global_var.Force_Convertor_to_Stop = FALSE;
	});
	CoreFunction::WorkThread_File_Convertor->detach();


}


/*------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------Functions for C Wrapper----------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
////////////////////////////// initialization //////////////////////////////////////*/

////////////////////////////// parameters ///////////////////////////////////////
void DAQ_Set_Minutes_to_Remember_Volt_Trace(CoreFunction *obj, int min){
	obj->Minutes_to_Remember_Volt_Trace = min;
	obj;
}
int DAQ_Get_Minutes_to_Remember_Volt_Trace(CoreFunction* obj)
{
	obj;
	return(obj->Minutes_to_Remember_Volt_Trace);
}
void DAQ_Set_Size_of_Spike_Train_Memory(CoreFunction* obj, unsigned __int64 size){
	obj;
	corefunc_global_var.Size_Of_Events_In_Memory = size;
}
unsigned __int64 DAQ_Get_DAQ_Size_of_Spike_Train_Memory(CoreFunction* obj)
{
	obj;
	return(corefunc_global_var.Size_Of_Events_In_Memory);
}
void DAQ_Set_DAQpath(CoreFunction* obj, char* path){
	obj;
	std::string s(path);
	obj->DAQpath = s;
}
char* DAQ_Get_DAQpath(CoreFunction* obj)
{
	obj;
	char * writable = new char[obj->DAQpath.size()];
	obj->DAQpath.assign(writable, obj->DAQpath.size());
	return(writable);
}
void DAQ_Set_Analog_OutputChannels(CoreFunction* obj, char* Out){
	obj;
	std::string s(Out);
	obj->Analog_OutputChannels = s;
	size_t x = s.find(":");
	std::string s1 = s.substr(x + 1, s.size());
	corefunc_global_var.Analog_Output_Channels = stoi(s1);
}
char* DAQ_Get_OutputChannels(CoreFunction* obj)
{
	obj;
	char * writable = new char[obj->Analog_OutputChannels.size()];
	obj->Analog_OutputChannels.assign(writable, obj->Analog_OutputChannels.size());
	return(writable);
}
void DAQ_Set_Analog_InputChannels(CoreFunction* obj, char* In){
	obj;
	std::string s(In);
	obj->Analog_InputChannels = s;
	size_t x = s.find(":");
	std::string s1 = s.substr(x + 1, s.size());
	corefunc_global_var.Analog_Input_Channels = stoi(s1) + 1;

}
char* DAQ_Get_InputChannels(CoreFunction* obj)
{
	obj;
	char * writable = new char[obj->Analog_InputChannels.size()];
	obj->Analog_InputChannels.assign(writable, obj->Analog_InputChannels.size());
	return(writable);
}
////////////////////////////// Functions ///////////////////////////////////////


CoreFunction* DAQ_ReturnRef(void)
{
	CoreFunction *Core = NULL;
	std::thread *main_thread = new std::thread([&]()
	{
		Core = new CoreFunction();
	}
	);
	main_thread->detach();

	while (Core == NULL)
	{
	}

	return Core;
}

void DAQ_DistroyRef(CoreFunction* obj){
	obj->DAQ_CleanUP();
	obj->Memory_CleanUP();
	delete obj;
}

int Core_Init(CoreFunction* obj){
	obj;
	obj->Init_Memory();
	return 0;
}

int DAQ_Init(CoreFunction* obj){
	obj;
	return obj->Init_DAQ();
}

void DAQ_Start(CoreFunction* obj){
	obj;
	obj->StartDAQ();
}

void DAQ_Stop(CoreFunction* obj){
	obj;
	obj->StopDAQ();
}

void DAQ_Start_Analog_In_Recording(CoreFunction* obj, int continues, char* path, char* filename, unsigned __int64 counter, int File_Size_in_Mega){
	obj;
	obj->Start_Analog_In_Recording(continues, path, filename, counter, File_Size_in_Mega);
}

void DAQ_Start_Analog_Out_Recording(CoreFunction* obj, char* path, char* filename, unsigned __int64 counter, int File_Size_in_Mega){
	obj;
	obj->Start_Analog_Out_Recording(path, filename, counter, File_Size_in_Mega);
}

void DAQ_Start_Digital_Recording(CoreFunction* obj, char* path, char* filename, unsigned __int64 counter, int File_Size_in_Mega){
	obj;
	obj->Start_Digital_Recording(path, filename, counter, File_Size_in_Mega);
}

void DAQ_Start_UserData_Recording(CoreFunction* obj, char* path, char* filename, unsigned __int64 counter, int File_Size_in_Mega){
	obj;
	obj->Start_UserData_Recording(path, filename, counter, File_Size_in_Mega);
}

void DAQ_Stop_ALL_Recording(CoreFunction* obj){
	obj;
	obj->Volt_Input_RecordingStarted = false;
	obj->Volt_Output_RecordingStarted = false;
	obj->Spike_RecordingStarted = false;
	obj->Spike_Time_RecordingStarted = false;
	obj->Digital_RecordingStarted = false;
	obj->UserData_RecordingStarted = false;
}

void DAQ_Stop_Input_Volt_Recording(CoreFunction* obj){
	obj;
	obj->Volt_Input_RecordingStarted = false;
}

void DAQ_Stop_Output_Volt_Recording(CoreFunction* obj){
	obj;
	obj->Volt_Output_RecordingStarted = false;
}

void DAQ_Stop_Spike_Recording(CoreFunction* obj){
	obj;
	obj->Spike_RecordingStarted = false;
	obj->Spike_Time_RecordingStarted = false;
}

void DAQ_Stop_Digital_Recording(CoreFunction* obj){
	obj;
	obj->Digital_RecordingStarted = false;
}

void DAQ_Stop_UserData_Recording(CoreFunction* obj){
	obj;
	obj->UserData_RecordingStarted = false;
}

int DAQ_Init_Playback(CoreFunction* obj, char* path, char* filename, unsigned __int64 counter, unsigned __int64 untilcounter, int continues){
	obj;
	return(obj->Init_Playback(path, filename, counter, untilcounter, continues, TRUE));
}

void DAQ_Start_Playback(CoreFunction* obj){
	obj;
	obj->Start_Restart_Playback();
}

void DAQ_Pause_Resume_Playback(CoreFunction* obj){
	obj;
	if (obj->PlaybackStarted){
		if (obj->Playback_pause)
			obj->Playback_pause = false;
		else
			obj->Playback_pause = true;
	}
}

void DAQ_CleanUp(CoreFunction* obj){
	obj;
	obj->DAQ_CleanUP();
	obj->Memory_CleanUP();
}

int DAQ_Load_DLL(CoreFunction* obj, char* DLL_PATH){
	obj;
	return obj->Load_DLL(DLL_PATH);
}

int DAQ_UNLoad_DLL(CoreFunction* obj){
	obj;
	return obj->ReleaseDLL();
}

int DAQ_Run_DLL(CoreFunction* obj, char* args){
	obj;
	return obj->Run_DLL(args);
}

int DAQ_Stop_DLL(CoreFunction* obj){
	obj;
	return obj->Stop_DLL();
}

void Recalculate_Threshold_For_All_Electrodes(CoreFunction* obj, int timer_in_Milisecond, double RMS_Factor){
	obj;
	obj->Recalculate_Threshold_For_All_Electrodes(timer_in_Milisecond, RMS_Factor);
}

void Recalculate_Threshold_For(CoreFunction* obj, int electrod, int timer_in_Milisecond, double RMS_Factor){
	obj;
	obj->Recalculate_Threshold_For(electrod, timer_in_Milisecond, RMS_Factor);
}

void Convert_Binary_Log_to_Text_Log(CoreFunction* obj, char* Source_Path, char* Source_Filename, BOOL continues, unsigned __int64 Start_counter, unsigned __int64 End_counter, char* Target_Path, char* Target_Filename){
	obj;
	obj->Convert_Binary_Log_to_Text_Log(Source_Path, Source_Filename, continues, Start_counter, End_counter, Target_Path, Target_Filename);
}

