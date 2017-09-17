/*
Closed Loop Experiment Manager

GlobalVar.h

Variables and functions common to GUI, Core function and Plugins

*/

#define	MAXTEXTMESSAGELEN	200

typedef struct
{
	unsigned __int64	timestamp;
	unsigned short		value;
} DIGITALEVENT;

typedef struct
{
	unsigned __int64	timestamp;
	unsigned short		channel; 
	float				*value;
} ANALOGEVENT;


typedef struct
{
	unsigned __int64	timestamp;
	float				*value;
} USEREVENT;


typedef struct
{	// Events
	ANALOGEVENT			*Analog_Spikes_Events;
	ANALOGEVENT			*Analog_Output_Events;
	USEREVENT			*User_Analog_Events;
	DIGITALEVENT		*Digital_I_Events, *Digital_O_Events; 
	unsigned __int64	Size_Of_Events_In_Memory;
	unsigned __int64	Location_of_Last_Event_Analog_Spike;
	unsigned __int64	Location_of_Last_Event_Analog_Output;
	unsigned __int64	Location_of_Last_Event_Digital_I, Location_of_Last_Event_Digital_O;
	unsigned __int64	Location_of_Last_Event_User_Analog;
	
	// RAW DATA
	float				*RAW_Input_Volts, *RAW_Output_Volts;
	unsigned __int64	*Pointer_To_Electrode_In_RAW_Input_Volts;
	unsigned __int64	*Pointer_To_Electrode_In_RAW_Output_Volts;
	unsigned __int64	Location_of_Last_Value_of_RAW_Analog_Input, Location_of_Last_Value_of_RAW_Analog_Output;
	unsigned __int64	*Time_Stamp_Counter_Array;
	unsigned __int64	Samples_per_Channel;
	
	// Threshold 
	unsigned int		Pre_Trigger_Samples, Post_Trigger_Samples;
	double				Spike_refractory_in_MicroSecond;
	float* 				threshold_Per_Electrod;
	unsigned __int32 	Calculating_Threshold;
	double 				RMS_Factor;
	int					LastChannelToInclude;

	//DAQ card Parameters 
	int					buffer_Input_Per_Channel;
	int					buffer_Output_Per_Channel;
	int					Input_bufferSize_for_all_channels;
	int					Analog_Input_Channels;
	int					Analog_Output_Channels;
	int					Digital_IO_Channels;
	double				Input_Hz, Output_Hz;
	char				BoardModelString[50];

	// Misc. Variables

	unsigned int		Number_of_User_Data_Channels;
	unsigned __int64	Time_Stamp_Counter;
	char				Spike_Extention_File[10];
	char				Volt_IN_Extention_File[10];
	char				Spike_Time_file[10];
	char				Volt_OUT_Extention_File[10];
	char				Digital_Extention_File[10];
	char				UserDataFile_Extention_File[10];
	unsigned __int64	Start_Sample_Time;
	unsigned __int64	Spike_count;  // for internal use only
	double				MAX_V, MIN_V;
	char* 				Status_Text;
	float 				DLL_Slow_Process_Running_Freq; 
	BOOL				Convertor_Start, Force_Convertor_to_Stop;

} CoreFunc_Global_var;

extern CoreFunc_Global_var corefunc_global_var;

extern	BOOL WriteMessage(char *messagetxt, BOOL erasealltext);
extern void EndOfPlayback(void);

