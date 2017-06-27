/*
Closed Loop Experiment Manager

CLEM.h

Main header file of application

*/

/* Shell related definitions */

#define		IDM_LOGFILESETUP		1000
#define		IDM_PLAYBACKFILESETUP	1001
#define 	IDM_LOADTIMEDEXP        1002
#define 	IDM_SAVETIMEDEXP        1003
#define 	IDM_SAVEMESSAGES		1004
#define 	IDM_QUIT                1009

#define 	IDM_ADDTEXTENTRY		1100
#define		IDM_CLEARMESSAGELOG		1101

#define		IDM_START				1200
#define		IDM_STOP				1201
#define		IDM_STARTLOGGING		1202
#define		IDM_STOPLOGGING			1203
#define		IDM_STARTTIMEDEXP		1204
#define		IDM_STOPTIMEDEXP		1205

#define		IDM_FREEZE				1206
#define		IDM_CLEARSPIKES			1207
#define		IDM_DISP_CONTINUOUS		1208
#define		IDM_DISP_LEVEL			1209
#define		IDM_LAYOUTLASTBIN		1211
#define		IDM_LAYOUTALLBINS		1212

#define		IDM_STARTPLAYBACK		1301
#define		IDM_PAUSEPLAYBACK		1302

#define		IDM_ENGAGE				1400
#define		IDM_DISENGAGE			1401
#define		IDM_LOCATEPLUGIN		1402
#define		IDM_LOADDLL				1403
#define		IDM_UNLOADDLL			1404

#define		IDM_CONVERTDIGFILES		1500

#define		IDM_SETUPTIMINGDLG		1600
#define		IDM_SETUPIODLG			1601
#define		IDM_LOADELECSETUP		1602
#define		IDM_SETUPDIODISPLAY		1603
#define		IDM_SETUPTHRESHOLDS		1604
#define		IDM_RECALC_THRESHOLDS	1605

#define 	IDM_ABOUT			 	1700

#define		IDM_ZOOMUP				1800
#define		IDM_ZOOMDOWN			1801
#define		IDM_APPLY_THRESH_TO_ALL	1802

#define 	ID_STATUSBAR			10
#define		STATUSBAR_PARTSNUMBER	7
#define		STATUSBAR_RECORD		0
#define		STATUSBAR_LOG			1
#define		STATUSBAR_EXECUTE		2
#define		STATUSBAR_PLAYBACK		3
#define		STATUSBAR_CONVERT		4
#define		STATUSBAR_TIMED			6

#define		WM_USER_REFRESH			(WM_USER + 1)
#define		WM_USER_NEWTEXTMESSAGE	(WM_USER + 2)

#define		UM_INITDISPLAY			1

#define		MAINWINDOW_X			1304
#define		MAINWINDOW_Y			813 

#define 	TRACEGRAPH_X 			400
#define 	TRACEGRAPH_Y 			550

#define 	TRACESINGLE_X 			400
#define 	TRACESINGLE_Y 			172

#define 	SPIKERASTER_X 			876
#define 	SPIKERASTER_Y 			172

#define 	SPIKEHISTOGRAM_X		876
#define 	SPIKEHISTOGRAM_Y		60

#define		DIOGRAPH_X				876
#define		DIOGRAPH_Y				83

#define 	USERGRAPH_X 			876
#define 	USERGRAPH_Y 			100

#define		MEALAYOUT_X				240
#define		MEALAYOUT_Y				295

#define		N_AD_CHANNELS			64

#define		DATA_POINTS_PER_TRACE	20000	/* best if this is integer multiple of TRACEGRAPH_X */
#define		DISPLAY_SAMPLS_PER_SPIKE 1000
#define		MAX_DISPLAY_SPIKES		10
#define		ROWS_LEVEL				16
#define		COLS_LEVEL				4

#define		DISPLAY_REFRESH_INTERVAL 100  
#define		TIMEDEXPERIMENT_INTERVAL 200

#define		DISPLAY_TIMER_ID		1
#define		TIMEDEXP_TIMER_ID		2
#define		CONVERT_EVENT_ID		3

#define		MAXRASTERUNITS			600

#define		MAX_AD_FREQ				46870
#define		MIN_AD_FREQ				8000

#define		MIN_DS					1
#define		MAX_DS					10

#define		MIN_VOLT_HISTORY		1
#define		MAX_VOLT_HISTORY		10

#define		MIN_SPIKE_HISTORY		10000
#define		MAX_SPIKE_HISTORY		500000

#define		MIN_SPIKE_TRACE			3
#define		MAX_SPIKE_TRACE			200

#define		MINTRACEDURATION		0.1
#define		MAXTRACEDURATION		10.0

#define		MINRASTERDURATION		1
#define		MAXRASTERDURATION		120

#define		MINHISTORANGE			5
#define		MAXHISTORANGE			2000

#define		MINDISPLAYVOLT			0.01
#define		MAXDISPLAYVOLT			10.0

#define		MINREFRACTORYMICROSEC	1000

#define		DM_CONTINUOUS			1
#define		DM_LEVEL				0

#define		PBFILETYPESPIKES		0				
#define		PBFILETYPEVOLT			1
#define		PBFILETYPESPIKESTIMES	2				

#define		LM_LASTBIN				0
#define		LM_ALLBINS				1

#define		ACTION_PRINT			0
#define		ACTION_LOG				1
#define		ACTION_LOADDLL			2
#define		ACTION_LOGANDEXEC		3
#define		ACTION_PAUSE			4

#define		MAXACTIONS				200

#define		MAXMESSAGEDISPLAYBUF	0x6000

typedef struct
{
	char		   BoardModelString[30];
	int			   BoardSampleRate;
	int			   BoardAOFreq;
	int			   SampleRate;
	int			   ContVoltageStorageDuration;	/* in minutes */
	int			   SpikeHistoryLength;
	int			   SpikeTraceDuration;			/* in msec */
	int			   SpikeTracePreTrigger;		/* in msec */
	int			   SpikesToDisplay;
	double		   VoltageFullScale;
	double		   TraceGraphTimeSlice;			/* in sec */
	double		   SpikeRasterTimeSlice;		/* in sec */
	int			   SpikeHistoMaxVal;			/* in spike counts*/
	double		   DisplayVoltageFullScale;
	float		   ThresholdRMSMultFactor;
	float		   ThresholdRMSMeasureDuration;	 /* in sec */
	float		   GlobalThreshold;				 /* in volt */
	int			   LastChannelToInclude;
	int			   InputBufferSizePerChannel;
	int			   OutputBufferSizePerChannel;
	int			   SpikeRefractoryPeriodMicrosecond;  
	char		   DAQCardPath[30];
	char		   DAQAnalogInputChannels[20];
	char		   DAQAnalogOutputChannels[20];
	char		   SpikesFileExt[10];
	char		   SpikeTimesExt[10];
	char		   AnalogInputFileExt[10];
	char		   AnalogOutFileExt[10];
	char		   DigitalIOFileExt[10];
	char		   UserDataFileExt[10];
} RECORDINGPARMS;

typedef struct
{
	int		    channel;
	char	    name[100];
	int		    xpos;
	int		    ypos;
	BOOL		inuse;
} ELECTRODEINFO;

typedef struct
{
	int		    action;
	double		duration;
	char		argstring[256];
	BOOL		active;
} ACTIONENTRY;


/* Global variables */

extern	HWND           hwndMainWindow;
extern	HWND		   hwndTraceGraphWindow;
extern  HWND		   hwndTraceSingleWindow;
extern	HWND		   hwndSpikeRasterWindow;
extern  HWND		   hwndSpikeHistogramWindow;
extern  HWND		   hwndDIOGraphWindow;
extern	HWND		   hwndUserGraphWindow;
extern	HWND		   hwndMEALayoutWindow;
extern  HWND		   hwndControlDialogBox;
extern	HWND		   hwndTimedExperimentDialogBox; 
extern	HWND 		   hwndStatusBar;
extern  POINT		   StoredCursorPt;
extern	char           *szAppName;
extern	HMENU          hMenu;
extern  HMENU		   hPopupMenu;
extern	HINSTANCE	   hAppInstance;
extern	UINT_PTR	   DisplayTimerID;
extern	UINT_PTR	   TimedExpTimerID;
extern	char		   AppDirectory[];

extern	BOOL		   Recording;
extern	BOOL		   Logging;
extern	BOOL		   Executing;
extern  BOOL		   PlayingBack;
extern	BOOL		   PlaybackPaused;
extern	BOOL		   TimedExperiment;
extern	BOOL		   DLL_Loaded;
extern	BOOL		   PlaybackFileLoaded;
extern	BOOL		   FreezeDisplay;
extern	BOOL		   Converting;
extern  int			   SelectedTraceNum;
extern	int			   DisplayMode;
extern	int			   LayoutMode;
extern  BOOL		   LogVolts;
extern  BOOL		   LogSpikes;
extern	BOOL		   LogDigIO;
extern	BOOL		   LogAnalogOut;
extern	BOOL		   LogSpikeTimes;
extern	BOOL		   LogUserGeneratedData;
extern	BOOL		   LogMessages;
extern  char		   LogFilePath[];
extern  char		   LogFilePrefix[];
extern  char		   TextLogFileName[];
extern  int			   FirstLogFileNumber;
extern  int			   MaxLogFileSize;
extern  char		   PlaybackFileDir[];
extern  char		   PlayBackFileNamePrefix[];
extern  int			   FirstPlaybackFile;
extern  int			   LastPlaybackFile;
extern	int			   PlaybackFileType;
extern	char		   ConvertInFileDir[];
extern	char		   ConvertOutFileDir[];
extern	char		   ConvertInFileNamePrefix[];
extern	int			   FirstConvertFile;
extern	int			   LastConvertFile;
extern	int			   ConvertFileType;
extern	int			   ActionEntries;
extern	int			   CurrActionEntry;
extern unsigned short  DigInputDisplayMask;
extern unsigned short  DigOutputDisplayMask;

extern	RECORDINGPARMS	CurrentRecParms;
extern	RECORDINGPARMS	GlobalRecParms;
extern	BOOL		   RecParamsChanged;
extern	ELECTRODEINFO	ElectrodesInfo[];
extern	ACTIONENTRY		*ActionTable;

