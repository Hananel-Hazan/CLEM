/*
Closed Loop Experiment Manager

Main file of application.
Contains Global variables declaration, entry point of program with message loop,
and window procedure of main window

*/

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma hdrstop

#include <process.h>
#include "CLEM.h"
#include "GraphWindow.h"
#include "DialogBoxes.h"
#include "Files.h"

/* DAQ core function */

#include "CoreFunction/CoreFunction.hpp"


/* function prototypes */

LRESULT CALLBACK 	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		InterfaceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
TIMERPROC			RunTimedExperimentEntry(HWND hWnd, INT nMsg, UINT nIDEvent, DWORD dwTime);
void				RegisterApplicationWindowClasses(HINSTANCE hInstance);
LRESULT CALLBACK	KeyboardHookProc(int  code, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK		MessageLogSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND				InitStatusBar(HWND hwndParent);
void				DefineStatusBarParts(int xclient);
BOOL				SetStatusBarText(int part, char *str);
void				InitCoreFunctionDataStructures(CoreFunction *cf);
void				SynctoDAQDataStructures(void);

/* Global variables of program */

HWND		   hwndMainWindow;
HWND		   hwndTraceGraphWindow;
HWND		   hwndTraceSingleWindow;
HWND		   hwndSpikeRasterWindow;
HWND		   hwndSpikeHistogramWindow;
HWND		   hwndDIOGraphWindow;
HWND		   hwndUserGraphWindow;
HWND		   hwndMEALayoutWindow;
HWND		   hwndControlDialogBox;
HWND		   hwndTimedExperimentDialogBox;
HWND 		   hwndStatusBar;
POINT		   StoredCursorPt;
char		   *szAppName = "CLEM";
HMENU          hMenu;
HMENU		   hPopupMenu;
HINSTANCE	   hAppInstance;
UINT_PTR	   DisplayTimerID;
UINT_PTR	   TimedExpTimerID;;
char		   AppDirectory[512];
LONG_PTR	   lpfnOriginalEditProc;

BOOL		   Recording = FALSE;
BOOL		   Logging = FALSE;
BOOL		   Executing = FALSE;
BOOL		   PlayingBack = FALSE;
BOOL		   PlaybackPaused = FALSE;
BOOL		   TimedExperiment = FALSE;
BOOL		   DLL_Loaded = FALSE;
BOOL		   PlaybackFileLoaded = FALSE;
BOOL		   FreezeDisplay = FALSE;
BOOL		   Converting = FALSE;
int			   SelectedTraceNum = 0;
int			   DisplayMode = DM_CONTINUOUS;
int			   LayoutMode = LM_LASTBIN;
BOOL		   LogVolts = FALSE;
BOOL		   LogSpikes = TRUE;
BOOL		   LogDigIO = FALSE;
BOOL		   LogAnalogOut = FALSE;
BOOL		   LogSpikeTimes = FALSE;
BOOL		   LogUserGeneratedData = FALSE;
BOOL		   LogMessages = TRUE;
char		   LogFilePath[MAX_PATH] = "C:\\";
char		   LogFilePrefix[100] = "default_log";
char		   TextLogFileName[MAX_PATH];
int			   FirstLogFileNumber = 0;
int			   MaxLogFileSize = 50;
char		   PlaybackFileDir[MAX_PATH];
char		   PlayBackFileNamePrefix[_MAX_FNAME];
int			   FirstPlaybackFile = 0;
int			   LastPlaybackFile = 0;
int			   PlaybackFileType = DM_LEVEL;
char		   ConvertInFileDir[MAX_PATH];
char		   ConvertOutFileDir[MAX_PATH];
char		   ConvertInFileNamePrefix[_MAX_FNAME];
int			   FirstConvertFile = 0;
int			   LastConvertFile = 0;
int			   ConvertFileType = DM_LEVEL;
int			   ActionEntries = 0;
int			   CurrActionEntry;
unsigned short DigInputDisplayMask = 0x1;
unsigned short DigOutputDisplayMask = 0x1;

RECORDINGPARMS CurrentRecParms;
RECORDINGPARMS GlobalRecParms;
BOOL		   RecParamsChanged = FALSE;
ELECTRODEINFO  ElectrodesInfo[N_AD_CHANNELS] = {
	{ 1, "[2,1]", 2, 1, TRUE }, { 2, "[3,1]", 3, 1, TRUE }, { 3, "[4,1]", 4, 1, TRUE }, { 4, "[5,1]", 5, 1, TRUE }, { 5, "[6,1]", 6, 1, TRUE }, { 6, "[7,1]", 7, 1, TRUE },
	{ 7, "[1,2]", 1, 2, TRUE }, { 8, "[2,2]", 2, 2, TRUE }, { 9, "[3,2]", 3, 2, TRUE }, { 10, "[4,2]", 4, 2, TRUE }, { 11, "[5,2]", 5, 2, TRUE }, { 12, "[6,2]", 6, 2, TRUE }, { 13, "[7,2]", 7, 2, TRUE }, { 14, "[8,2]", 8, 2, TRUE },
	{ 15, "[1,3]", 1, 3, TRUE }, { 16, "[2,3]", 2, 3, TRUE }, { 17, "[3,3]", 3, 3, TRUE }, { 18, "[4,3]", 4, 3, TRUE }, { 19, "[5,3]", 5, 3, TRUE }, { 20, "[6,3]", 6, 3, TRUE }, { 21, "[7,3]", 7, 3, TRUE }, { 22, "[8,3]", 8, 3, TRUE },
	{ 23, "[1,4]", 1, 4, TRUE }, { 24, "[2,4]", 2, 4, TRUE }, { 25, "[3,4]", 3, 4, TRUE }, { 26, "[4,4]", 4, 4, TRUE }, { 27, "[5,4]", 5, 4, TRUE }, { 28, "[6,4]", 6, 4, TRUE }, { 29, "[7,4]", 7, 4, TRUE }, { 30, "[8,4]", 8, 4, TRUE },
	{ 31, "[1,5]", 1, 5, TRUE }, { 32, "[2,5]", 2, 5, TRUE }, { 33, "[3,5]", 3, 5, TRUE }, { 34, "[4,5]", 4, 5, TRUE }, { 35, "[5,5]", 5, 5, TRUE }, { 36, "[6,5]", 6, 5, TRUE }, { 37, "[7,5]", 7, 5, TRUE }, { 38, "[8,5]", 8, 5, TRUE },
	{ 39, "[1,6]", 1, 6, TRUE }, { 40, "[2,6]", 2, 6, TRUE }, { 41, "[3,6]", 3, 6, TRUE }, { 42, "[4,6]", 4, 6, TRUE }, { 43, "[5,6]", 5, 6, TRUE }, { 44, "[6,6]", 6, 6, TRUE }, { 45, "[7,6]", 7, 6, TRUE }, { 46, "[8,6]", 8, 6, TRUE },
	{ 47, "[1,7]", 1, 7, TRUE }, { 48, "[2,7]", 2, 7, TRUE }, { 49, "[3,7]", 3, 7, TRUE }, { 50, "[4,7]", 4, 7, TRUE }, { 51, "[5,7]", 5, 7, TRUE }, { 52, "[6,7]", 6, 7, TRUE }, { 53, "[7,7]", 7, 7, TRUE }, { 54, "[8,7]", 8, 7, TRUE },
	{ 55, "[2,8]", 2, 8, TRUE }, { 56, "[3,8]", 3, 8, TRUE }, { 57, "[4,8]", 4, 8, TRUE }, { 58, "[5,8]", 5, 8, TRUE }, { 59, "[6,8]", 6, 8, TRUE }, { 60, "[7,8]", 7, 8, TRUE },
	{ 61, "Aux1", 1, 10, TRUE }, { 62, "Aux2", 2, 10, TRUE }, { 63, "Aux3", 3, 10, TRUE }, { 64, "Aux4", 4, 10, TRUE }
};

ACTIONENTRY		*ActionTable;

char		   PluginDLLFileName[512];
char		   PluginDLLArgString[256];

CoreFunction   *CoreObj;
BOOL			Init_Core;


/* static module variables */

static HHOOK		hHook;
static PSTR			CommandLine;
static __int64		LastTick;
static int			CountdownLast;
static BOOL			TimedActionInProgress;


/* programs entry point (like main() in regular C programs) */

#pragma warning ( disable : 4100)

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpszCmdLine,
	_In_ int nCmdShow)
{
	MSG         	msg;
	LPVOID			hHeap;

	/* Store the applications instance handle in a global variable */

	hAppInstance = hInstance;

	/* Get the applications current directory */

	GetCurrentDirectory(512, AppDirectory);

	/* load default aquisition, recording and display parameters from the default .ini file
	   or another file if specified */

	if (!LoadParametersFromIniFile(NULL))
		return 1;

	/* Copy these to the current parameter set */

	memcpy_s((void *)&CurrentRecParms, sizeof(RECORDINGPARMS), (void *)&GlobalRecParms, sizeof(RECORDINGPARMS));

	/* allocate memory for the timed execution data structure */

	hHeap = GetProcessHeap();
	ActionTable = (ACTIONENTRY *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAXACTIONS * sizeof(ACTIONENTRY));
	if (ActionTable == NULL)
	{
		MessageBox(NULL, "Failed to allocate memory for Timed Execution entry table", "Error", MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	/* initialize the display data structures */

	if (InitializeCLEMGraphics() == FALSE)
		return 1;

	/* create a default name for the Text Log file - unless changed, will be created and stored on desktop */

	CreateDefaultTextMessageFileName(TextLogFileName);

	/* Initialize the DAQ application core object */

	CoreObj = DAQ_ReturnRef();
	InitCoreFunctionDataStructures(CoreObj);

	Init_Core = FALSE;

	/* register all appplication window classes
	   (a lot of messy code, relegated to a function below */

	RegisterApplicationWindowClasses(hInstance);

	/* initialize Microsoft's common control dynamic-link library */

	InitCommonControls();

	/* store a pointer to the command line string - will be used during initialization */

	CommandLine = lpszCmdLine;

	/* create the applications main window */

	hwndMainWindow = CreateWindow(szAppName, "Closed Loop Experiment Manager",
		WS_OVERLAPPEDWINDOW,
		0, 0, MAINWINDOW_X, MAINWINDOW_Y,
		NULL, NULL, hInstance, NULL);

	/* create the applications Trace Graph window */

	hwndTraceGraphWindow = CreateWindow("CLEMGraphWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		884, 4, TRACEGRAPH_X, TRACEGRAPH_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* create the applications Single Trace Graph window */

	hwndTraceSingleWindow = CreateWindow("CLEMSingleWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		884, 558, TRACESINGLE_X, TRACESINGLE_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* create the applications Spike Raster window */

	hwndSpikeRasterWindow = CreateWindow("CLEMRasterWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		4, 558, SPIKERASTER_X, SPIKERASTER_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* create the applications Spike Histogram window */

	hwndSpikeHistogramWindow = CreateWindow("CLEMHistoWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		4, 494, SPIKEHISTOGRAM_X, SPIKEHISTOGRAM_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* Create the Digital IO graph window */

	hwndDIOGraphWindow = CreateWindow("CLEMDIOGraphWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		4, 407, DIOGRAPH_X, DIOGRAPH_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* Create the user graph window */

	hwndUserGraphWindow = CreateWindow("CLEMUserGraphWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		4, 303, USERGRAPH_X, USERGRAPH_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* Create the MEA layout graph window */

	hwndMEALayoutWindow = CreateWindow("CLEMMEALayoutWindow", "",
		WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
		640, 4, MEALAYOUT_X, MEALAYOUT_Y,
		hwndMainWindow, NULL, hInstance, NULL);

	/* Show all the windows */

	ShowWindow(hwndMainWindow, nCmdShow);
	UpdateWindow(hwndMainWindow);

	ShowWindow(hwndTraceGraphWindow, nCmdShow);
	UpdateWindow(hwndTraceGraphWindow);

	ShowWindow(hwndTraceSingleWindow, nCmdShow);
	UpdateWindow(hwndTraceSingleWindow);

	ShowWindow(hwndSpikeRasterWindow, nCmdShow);
	UpdateWindow(hwndTraceGraphWindow);

	ShowWindow(hwndSpikeHistogramWindow, nCmdShow);
	UpdateWindow(hwndTraceGraphWindow);

	ShowWindow(hwndDIOGraphWindow, nCmdShow);
	UpdateWindow(hwndDIOGraphWindow);

	ShowWindow(hwndUserGraphWindow, nCmdShow);
	UpdateWindow(hwndUserGraphWindow);

	ShowWindow(hwndMEALayoutWindow, nCmdShow);
	UpdateWindow(hwndMEALayoutWindow);

	/* the applications message loop - retrieve messages posted to the application */

	while (GetMessage(&msg, (HWND)NULL, 0, 0))
	{
		/* check if the message belongs to one of the modeless dialog boxes */

		if (IsWindow(hwndControlDialogBox) && IsDialogMessage(hwndControlDialogBox, &msg))
			continue;

		if (IsWindow(hwndTimedExperimentDialogBox) && IsDialogMessage(hwndTimedExperimentDialogBox, &msg))
			continue;

		/* if it doesn't, handle the message */

		TranslateMessage(&msg);

		/* Call WndProc() to handle message */

		DispatchMessage(&msg);
	}

	/* clean up and release memory allocations */

	CleanupCLEMGraphics();

	HeapFree(hHeap, 0, (LPVOID)ActionTable);

	return (int)msg.wParam;
}


/* applications main window callback procedure */

LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int			cyToolBar, cyStatus;
	static char			FileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
	static char			DialogBoxTitle[80];
	static int			LastLoadedImageFileType = 0;
	UINT_PTR			ConversionTimer = 0;
	PAINTSTRUCT         ps;
	int                 i;
	MINMAXINFO       	*lpmmi;
	HCURSOR				hCursor;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* Load applications menu from resource table */

		hMenu = GetMenu(hwnd);

		/*  set up a message hook */

		hHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardHookProc, NULL, GetCurrentThreadId());

		/* initialize common controls (status bar) */

		hwndStatusBar = InitStatusBar(hwnd);

		/* initialize display */

		PostMessage(hwnd, WM_USER, UM_INITDISPLAY, 0);

		return 0;

	case WM_SIZE:								/* windows size has changed */

		if (wParam == SIZE_MINIMIZED)
		{
			/* do somthing here if needed. At present does nothing*/
			return 0;
		}

		/* resize the different sections of the status bar at the bottom of the window */

		DefineStatusBarParts(LOWORD(lParam));

		return 0;

	case WM_GETMINMAXINFO:						/* window size is about to change */

		/* confine the allowed range of the windows size by assigning values
		   to variables in a MINMAXINFO structure. a pointer to this structure
		   is provided in lParam */

		lpmmi = (MINMAXINFO *)lParam;

		lpmmi->ptMinTrackSize.x = MAINWINDOW_X / 2;		/* arbitrary values;  change if needed */
		lpmmi->ptMinTrackSize.y = MAINWINDOW_Y / 2;
		lpmmi->ptMaxTrackSize.x = MAINWINDOW_X;
		lpmmi->ptMaxTrackSize.y = MAINWINDOW_Y;

		return 0;

	case WM_PAINT:								/* redraw the window (in practice, all child windows redraw themself) */

		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		return 0;

	case WM_USER: 								/* private application message */

		switch (LOWORD(wParam))
		{
		case UM_INITDISPLAY:					/* intialize the applications modeless dialog boxes */

			/* Create the modeless interface dialog box */

			hwndControlDialogBox = CreateDialog(hAppInstance, "Dlg_CLEMinterface", hwnd, (DLGPROC)InterfaceDlgProc);
			SetWindowPos(hwndControlDialogBox, HWND_TOP, 4, 4, 640, 300, SWP_NOSIZE | SWP_NOZORDER);
			ShowWindow(hwndControlDialogBox, SW_SHOW);

			/* initialize the states of the various controls ...*/

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			/* create the timed experiment sequence modeless dialog box */

			hwndTimedExperimentDialogBox = CreateDialog(hAppInstance, "Dlg_TimingSetup", hwnd, (DLGPROC)TimingSetup);

			WriteMessage("CLEM initialization complete", FALSE);

			break;
		}

		return 0;

	case WM_USER_REFRESH: 						/* refresh the display - called from multiple places in the application */

		InvalidateRect(hwndTraceGraphWindow, NULL, TRUE);
		UpdateWindow(hwndTraceGraphWindow);

		InvalidateRect(hwndTraceSingleWindow, NULL, TRUE);
		UpdateWindow(hwndTraceSingleWindow);

		InvalidateRect(hwndSpikeRasterWindow, NULL, TRUE);
		UpdateWindow(hwndSpikeRasterWindow);

		InvalidateRect(hwndSpikeHistogramWindow, NULL, TRUE);
		UpdateWindow(hwndSpikeHistogramWindow);

		InvalidateRect(hwndDIOGraphWindow, NULL, TRUE);
		UpdateWindow(hwndDIOGraphWindow);

		InvalidateRect(hwndUserGraphWindow, NULL, TRUE);
		UpdateWindow(hwndUserGraphWindow);

		InvalidateRect(hwndMEALayoutWindow, NULL, TRUE);
		UpdateWindow(hwndMEALayoutWindow);

		return 0;

	case WM_COMMAND:							/* Messages from Menus, Buttons, and other controls */

		switch (LOWORD(wParam))
		{
		case IDM_LOGFILESETUP:					/* Menu item: File | Data Log File Setup */

			if (Logging)
				return 0;

			/* Open the data log file setup dialog box */

			i = (int)DialogBox(hAppInstance, "Dlg_DataLogFileSetup", hwnd, (DLGPROC)DataLogFileSetup);

			/* if the user pressed the OK button, flag this for the next time a recording is to start.*/

			if (i == TRUE)
				RecParamsChanged = TRUE;

			return 0;

		case IDM_PLAYBACKFILESETUP:				/* Menu item: File | Playback File Setup */

			if (Recording || Logging || Executing || (PlayingBack && !PlaybackPaused))
				return 0;

			/* Open the playback setup dialog box.
			   The dialog box, if user pressed OK, will set the  global variables
			   PlaybackFileDir, PlayBackFileNamePrefix, FirstPlaybackFile, LastPlaybackFile, PlaybackFileType */

			i = (int)DialogBox(hAppInstance, "Dlg_PlayBackFileSetup", hwnd, (DLGPROC)PlaybackFileSetup);

			if (i != TRUE)
				return 0;

			/* notify the user that the system in preparing itself for playback */

			WriteMessage("Initializing internal data structures. Please wait...", FALSE);

			/* capture the mouse, set to "wait" icon */

			hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			ShowCursor(TRUE);

			/* call the Core function */

			i = DAQ_Init_Playback(CoreObj, PlaybackFileDir, PlayBackFileNamePrefix, FirstPlaybackFile, LastPlaybackFile, PlaybackFileType);

			/* release the mouse */

			ShowCursor(FALSE);
			SetCursor(hCursor);

			WriteMessage("Done", FALSE);

			/* update some internal variables */

			SynctoDAQDataStructures();
			RecParamsChanged = TRUE;
			PlaybackFileLoaded = TRUE;

			/* refresh the display */

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_LOADTIMEDEXP:					/* Menu item: File | Load Experiment Sequence */

			/* get the name of the file to load */

			strcpy_s(DialogBoxTitle, 80, "Load Experiment Sequence");
			FileName[0] = 0;

			if (FileOpenDlg(hwnd, FT_TIMEDEXP, FileName, DialogBoxTitle) == FALSE)
				return 0;

			/* load the experiment sequence file */

			ActionEntries = 0;

			i = LoadActionsFromFile(FileName);
			if (i == 0)
				MessageBox(hwnd, "An error occured while attempting to load the experiment sequence to file", "Error", MB_ICONERROR | MB_OK);
			else
				ActionEntries = i;

			/* refresh appropriate display items */

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwndTimedExperimentDialogBox, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_SAVETIMEDEXP:					/* Menu item: File | Save Experiment Sequence */

			/* get sequence file name to use */

			strcpy_s(DialogBoxTitle, 80, "Save Experiment Sequence");
			FileName[0] = 0;

			if (FileSaveDlg(hwnd, FT_TIMEDEXP, FileName, DialogBoxTitle) == FALSE)
				return 0;

			/* save the current experimenatl sequence to file */

			if (SaveActionsToFile(FileName) == FALSE)
				MessageBox(hwnd, "An error occured while attempting to save the experiment sequence to file", "Error", MB_ICONERROR | MB_OK);

			return 0;

		case IDM_SAVEMESSAGES:					/* Menu item: File | Save Message Log As */

			/* get a file name to use */

			strcpy_s(DialogBoxTitle, 80, "File to which Text Messages Will Be Saved To");
			FileName[0] = 0;

			if (FileSaveDlg(hwnd, FT_TEXTLOG, FileName, DialogBoxTitle) == FALSE)
				return 0;

			strncpy_s(TextLogFileName, MAX_PATH, FileName, MAX_PATH - 1);

			return 0;

		case IDM_ADDTEXTENTRY:					/* Menu item: Edit | Add Manual Entry to Message Log; 
												   Also called when the Message Log window is double clicked (see MessageLogSubclassProc() below */

			DialogBox(hAppInstance, "Dlg_AddTextEntry", hwnd, (DLGPROC)AddTextEntry);

			return 0;

		case IDM_CLEARMESSAGELOG:				/* Menu item: Edit | Clear All Messages from Message Log */

			/* verify */

			i = MessageBox(hwnd, "Clear all Messages from Message Log: Are you sure?", "Clear log", MB_ICONEXCLAMATION | MB_YESNO);

			/* clear by writing a blank message with 'erasealltext' set to TRUE */

			if (i == IDYES)
				WriteMessage("", TRUE);

			return 0;

		case IDM_START:							/* Menu item: Recording | Start; Also called when the user presses the "Start" button */
			
			if (Recording || Logging || Executing || PlayingBack)
				return 0;

			/* if changes were made to the recording parameters (for example, when a
			   file is played back or the user changed the parameters) reinitilaize	everything */

			if (RecParamsChanged)
			{
				/* notify user by adding an entry to the message log */

				WriteMessage("Initializing internal data structures. Please wait...", FALSE);

				/* capture the mouse, set to "wait" icon */

				hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				ShowCursor(TRUE);

				/* Copy the global parameter set to the current parameter set */

				memcpy_s((void *)&CurrentRecParms, sizeof(RECORDINGPARMS), (void *)&GlobalRecParms, sizeof(RECORDINGPARMS));

				/* unload the plugin DLL if loaded */

				if (DLL_Loaded)
				{
					SetWindowText(GetDlgItem(hwndControlDialogBox, 304), "");
					SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");
					DAQ_UNLoad_DLL(CoreObj);
				}

				/* clear resources and initialize the core object */

				InitCoreFunctionDataStructures(CoreObj);

				/* reload the DLL if necessary */

				if (DLL_Loaded)
				{
					i = DAQ_Load_DLL(CoreObj, PluginDLLFileName);
					if (i != 0)
					{
						MessageBox(hwnd, "Failed to load plugin DLL file", "Error", MB_ICONERROR | MB_OK);
						SetWindowText(GetDlgItem(hwndControlDialogBox, 304), "");
						SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");
						DLL_Loaded = FALSE;
						return 0;
					}

					SetWindowText(GetDlgItem(hwndControlDialogBox, 304), PluginDLLFileName);
					SetWindowText(GetDlgItem(hwndControlDialogBox, 306), " Loaded");
				}

				/* release the mouse */

				ShowCursor(FALSE);
				SetCursor(hCursor);

				/* update internal varibles */

				RecParamsChanged = FALSE;
				PlaybackFileLoaded = FALSE;

				WriteMessage("Done", FALSE);
			}

			/* Clean and refresh all display windows, update internal variables  */

			ResetGraphicsData();

			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			Recording = TRUE;
			PlaybackPaused = FALSE;

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			/* start a Windows Timer that will drive the periodic display of incoming data
			   by periodically calling RenderTimerProc ()*/

			if (!DisplayTimerID)
				DisplayTimerID = SetTimer(hwnd, DISPLAY_TIMER_ID, DISPLAY_REFRESH_INTERVAL, (TIMERPROC)RenderTimerProc);

			/* Instruct the Core Function to start aquisition */

			DAQ_Start(CoreObj);

			/* update the status bar */

			SetStatusBarText(STATUSBAR_RECORD, "Acquiring Data");

			return 0;

		case IDM_STOP:							/* Menu item: Recording | Stop; Also called when the user presses the "Stop" button */
			
			if (!Recording || PlayingBack)
				return 0;

			/* first stop closed loop execution and data logging */

			if (Executing)
				SendMessage(hwnd, WM_COMMAND, IDM_DISENGAGE, 0);

			if (Logging)
				SendMessage(hwnd, WM_COMMAND, IDM_STOPLOGGING, 0);

			/* Instruct the Core Function to stop aquisition */

			DAQ_Stop(CoreObj);

			/* kill the Windows Timer that drives periodic rendering of incoming data */

			if (DisplayTimerID)
				KillTimer(hwnd, DisplayTimerID);
			DisplayTimerID = 0;

			/* update the display and internal variables */

			Recording = FALSE;

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			SetStatusBarText(STATUSBAR_RECORD, "");

			return 0;

		case IDM_STARTLOGGING:					/* Menu item: Recording | Start Logging; Also called when the user presses the "Start" button */

			if (!Recording || Logging)
				return 0;

			/* instruct the Core Functiom to begin logging of selected data */

			if (LogVolts)  
				DAQ_Start_Analog_In_Recording(CoreObj, 1, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			if (LogSpikes)  
				DAQ_Start_Analog_In_Recording(CoreObj, 0, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			if (LogSpikeTimes) 
				DAQ_Start_Analog_In_Recording(CoreObj, 2, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			if (LogUserGeneratedData)
				DAQ_Start_UserData_Recording(CoreObj, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			if (LogAnalogOut) 
				DAQ_Start_Analog_Out_Recording(CoreObj, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			if (LogDigIO) 
				DAQ_Start_Digital_Recording(CoreObj, LogFilePath, LogFilePrefix, FirstLogFileNumber, MaxLogFileSize);

			Logging = TRUE;

			/* add an entry into the message log and refresh display */

			WriteMessage("Data Logging Initiated", FALSE);

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			SetStatusBarText(STATUSBAR_LOG, "Logging Data");

			return 0;

		case IDM_STOPLOGGING:					/* Menu item: Recording | Stop Logging; Also called when the user presses the "Stop" button */

			if (!Logging)
				return 0;

			/* instruct the core function to stop data logging */

			DAQ_Stop_ALL_Recording(CoreObj);

			Logging = FALSE;

			/* add an entry into the message log and refresh display */

			WriteMessage("Data Logging Stopped", FALSE);

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			SetStatusBarText(STATUSBAR_LOG, "");

			return 0;

		case IDM_STARTTIMEDEXP:					/* Menu item: Recording | Begin Timed Experiment"; Also called when the user presses the "Begin" button */
			
			if (Logging || Executing || PlayingBack || TimedExperiment)
				return 0;

			/* and an entry to the message log file and update the display and internal variables */

			WriteMessage("Beginning Timed Experiment", FALSE);

			CurrActionEntry = 0;
			TimedExperiment = TRUE;

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwndTimedExperimentDialogBox, WM_USER_REFRESH, 0, 0);

			/* start a Windows Timer that will trigger execution of experiment sequence steps */

			if (!TimedExpTimerID)
				TimedExpTimerID = SetTimer(hwnd, TIMEDEXP_TIMER_ID, TIMEDEXPERIMENT_INTERVAL, (TIMERPROC)RunTimedExperimentEntry);

			return 0;

		case IDM_STOPTIMEDEXP:					/* Menu item: Recording | Terminate Timed Experiment"; Also called when the user presses the "Terminate" button */

			if (!TimedExperiment)
				return 0;

			/* Kill the Windows Timer that triggers experiment sequence steps */

			if (TimedExpTimerID)
				KillTimer(hwnd, TimedExpTimerID);
			TimedExpTimerID = 0;

			/* and an entry to the message log and update the display and internal variables */

			TimedExperiment = FALSE;
			TimedActionInProgress = FALSE;

			if (CurrActionEntry < ActionEntries)
				WriteMessage("Timed Experiment Aborted", FALSE);

			SetStatusBarText(STATUSBAR_TIMED, "");

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwndTimedExperimentDialogBox, WM_USER_REFRESH, 0, 0);
			InvalidateRect(hwndTimedExperimentDialogBox, NULL, TRUE);

			return 0;

		case IDM_STARTPLAYBACK:					/* Menu item: Playback | (Re)Start Playback"; Also called when the user presses the "(Re)Start" button */
			
			if (Recording)
				return 0;

			/* if playback is active or paused momentary, clean up first */

			if (PlayingBack || PlaybackPaused)
			{
				if (Executing)
					SendMessage(hwnd, WM_COMMAND, IDM_DISENGAGE, 0);

				if (Logging)
					DAQ_Stop_ALL_Recording(CoreObj);

				if (DisplayTimerID)
					KillTimer(hwnd, DisplayTimerID);
				DisplayTimerID = 0;

				SetStatusBarText(STATUSBAR_LOG, "");
			}

			/* Clean and refresh all display windows  */

			ResetGraphicsData();

			PlayingBack = TRUE;
			PlaybackPaused = FALSE;

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			/* start a Windows Timer that will drive the periodic display of playedback data
			   by periodically calling RenderTimerProc ()*/

			if (!DisplayTimerID)
				DisplayTimerID = SetTimer(hwnd, DISPLAY_TIMER_ID, DISPLAY_REFRESH_INTERVAL, (TIMERPROC)RenderTimerProc);

			/* Instruct the Core Function to start playing back data (see IDM_PLAYBACKFILESETUP above for playback settings */

			DAQ_Start_Playback(CoreObj);

			/* add an entry to the message log file and update the display */

			WriteMessage("Playback started", FALSE);

			SetStatusBarText(STATUSBAR_PLAYBACK, "Playing Back");

			return 0;

		case IDM_PAUSEPLAYBACK:					/* Menu item: Playback | Pause Playback"; Also called when the user presses the "Pause" button */

			if (Recording)
				return 0;

			/* toggle playback between Pause and Resume */

			if (PlaybackPaused)			/* paused: resume playback */
			{
				PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

				/* start a Windows Timer that will drive the periodic display of playedback data
				   by periodically calling RenderTimerProc () */

				if (!DisplayTimerID)
					DisplayTimerID = SetTimer(hwnd, DISPLAY_TIMER_ID, DISPLAY_REFRESH_INTERVAL, (TIMERPROC)RenderTimerProc);

				PlayingBack = TRUE;
				PlaybackPaused = FALSE;

				SetStatusBarText(STATUSBAR_PLAYBACK, "Playing Back");
			}
			else						/* playing back - pause playback */
			{
				if (Executing)
					SendMessage(hwnd, WM_COMMAND, IDM_DISENGAGE, 0);

				/* instruct Core Object to stop all data logging  */

				if (Logging)
					DAQ_Stop_ALL_Recording(CoreObj);

				/* kill data rendering timer */

				if (DisplayTimerID)
					KillTimer(hwnd, DisplayTimerID);
				DisplayTimerID = 0;

				PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

				PlayingBack = FALSE;
				PlaybackPaused = TRUE;
				SetStatusBarText(STATUSBAR_PLAYBACK, "");
			}

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			/* instruct Core Object to toggle playback  */

			DAQ_Pause_Resume_Playback(CoreObj);

			return 0;

		case IDM_ENGAGE:						/* Menu item: Plugins | Engage"; Also called when the user presses the "Engage" button */
			
			if (Executing)
				return 0;
			
			/* get user defined argument string */

			GetWindowText(GetDlgItem(hwndControlDialogBox, 305), PluginDLLArgString, 256);

			DAQ_Run_DLL(CoreObj, PluginDLLArgString);

			/* add entry to message log and update display */

			WriteMessage("Closed loop process engaged", FALSE);

			Executing = TRUE;

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			SetStatusBarText(STATUSBAR_EXECUTE, "Closed Loop Engaged");

			return 0;

		case IDM_DISENGAGE:						/* Menu item: Plugins | Disengage"; Also called when the user presses the "Engage" button */

			if (!Executing)
				return 0;

			DAQ_Stop_DLL(CoreObj);

			Executing = FALSE;

			WriteMessage("Closed loop process disengaged", FALSE);
			SetStatusBarText(STATUSBAR_EXECUTE, "");

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_LOCATEPLUGIN:

			if (Executing)
				return 0;

			/* open a standard Windows file open dialog box */

			strcpy_s(DialogBoxTitle, 80, "Load a Plugin File");
			FileName[0] = 0;

			if (FileOpenDlg(hwnd, FT_PLUGIN, FileName, DialogBoxTitle) == FALSE)
				return 0;

			/* unload the prior DLL if exists */

			if (DLL_Loaded)
			{
				i = DAQ_UNLoad_DLL(CoreObj);
				if (i != 0)
				{
					MessageBox(hwnd, "Failed to unload prior plugin DLL file", "Error", MB_ICONERROR | MB_OK);
					return 0;
				}
				
				DLL_Loaded = FALSE;
				SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");
			}

			/* store the new file name */

			strcpy_s(PluginDLLFileName, 512, FileName);
			SetWindowText(GetDlgItem(hwndControlDialogBox, 304), PluginDLLFileName);

			return 0;

		case IDM_LOADDLL:

			if (Executing)
				return 0;

			if (DLL_Loaded)
			{
				i = DAQ_UNLoad_DLL(CoreObj);
				if (i != 0)
				{
					MessageBox(hwnd, "Failed to unload prior plugin DLL file", "Error", MB_ICONERROR | MB_OK);
					return 0;
				}
				DLL_Loaded = FALSE;
				SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");
			}

			/* get plugin file name */

			GetWindowText(GetDlgItem(hwndControlDialogBox, 304), PluginDLLFileName, 512);
			GetWindowText(GetDlgItem(hwndControlDialogBox, 305), PluginDLLArgString, 256);

			i = DAQ_Load_DLL(CoreObj, PluginDLLFileName);  

			if (i != 0)
			{
				MessageBox(hwnd, "Failed to load plugin DLL file", "Error", MB_ICONERROR | MB_OK);
				SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");
				return 0;
			}

			SetWindowText(GetDlgItem(hwndControlDialogBox, 306), " Loaded");

			DLL_Loaded = TRUE;
			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_UNLOADDLL:

			if (!DLL_Loaded || Executing)
				return 0;

			i = DAQ_UNLoad_DLL(CoreObj);

			if (i != 0)
			{
				MessageBox(hwnd, "Failed to unload plugin DLL file", "Error", MB_ICONERROR | MB_OK);
				return 0;
			}

			SetWindowText(GetDlgItem(hwndControlDialogBox, 306), "");

			DLL_Loaded = FALSE;
			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_DISP_CONTINUOUS:		/* Display | Continuous */
		case IDM_DISP_LEVEL:			/* Display | Level */

			DisplayMode = (wParam == IDM_DISP_CONTINUOUS ? DM_CONTINUOUS : DM_LEVEL);

			PostMessage(GetDlgItem(hwndControlDialogBox, IDM_DISP_CONTINUOUS), BM_SETCHECK, (DisplayMode == DM_CONTINUOUS ? BST_CHECKED : BST_UNCHECKED), 0);
			PostMessage(GetDlgItem(hwndControlDialogBox, IDM_DISP_LEVEL), BM_SETCHECK, (DisplayMode == DM_LEVEL ? BST_CHECKED : BST_UNCHECKED), 0);

			InvalidateRect(hwndTraceGraphWindow, NULL, TRUE);
			UpdateWindow(hwndTraceGraphWindow);

			InvalidateRect(hwndTraceSingleWindow, NULL, TRUE);
			UpdateWindow(hwndTraceSingleWindow);

			return 0;

		case IDM_LAYOUTLASTBIN:			/* Display | Layout: Last bin */
		case IDM_LAYOUTALLBINS:			/* Display | Layout: All bins */

			LayoutMode = (wParam == IDM_LAYOUTLASTBIN ? LM_LASTBIN : LM_ALLBINS);

			PostMessage(GetDlgItem(hwndControlDialogBox, IDM_LAYOUTLASTBIN), BM_SETCHECK, (LayoutMode == LM_LASTBIN ? BST_CHECKED : BST_UNCHECKED), 0);
			PostMessage(GetDlgItem(hwndControlDialogBox, IDM_LAYOUTALLBINS), BM_SETCHECK, (LayoutMode == LM_ALLBINS ? BST_CHECKED : BST_UNCHECKED), 0);

			InvalidateRect(hwndMEALayoutWindow, NULL, TRUE);
			UpdateWindow(hwndMEALayoutWindow);

			return 0;

		case IDM_FREEZE:

			/* FreezeDisplay value updated by the UI dialog box windproc */

			EnableWindow(GetDlgItem(hwndControlDialogBox, 402), !FreezeDisplay);
			EnableWindow(GetDlgItem(hwndControlDialogBox, 403), !FreezeDisplay);
			EnableWindow(GetDlgItem(hwndControlDialogBox, 404), !FreezeDisplay);
			EnableWindow(GetDlgItem(hwndControlDialogBox, IDM_LAYOUTLASTBIN), !FreezeDisplay);
			EnableWindow(GetDlgItem(hwndControlDialogBox, IDM_LAYOUTALLBINS), !FreezeDisplay);

			/* handled by the UI dialog box windproc */

			return 0;

		case IDM_CLEARSPIKES:

			/* Clean and refresh all display windows  */

			ClearDisplayedSpikes();

			if (DisplayMode == DM_LEVEL)
			{
				InvalidateRect(hwndTraceGraphWindow, NULL, TRUE);
				UpdateWindow(hwndTraceGraphWindow);

				InvalidateRect(hwndTraceSingleWindow, NULL, TRUE);
				UpdateWindow(hwndTraceSingleWindow);
			}

			return 0;

		case IDM_CONVERTDIGFILES:

			if (Logging)
				return 0;

			/* Open the data log file setup dialog box */

			i = (int)DialogBox(hAppInstance, "Dlg_ConvertFileSetup", hwnd, (DLGPROC)ConvertFileSetup);

			if (i == TRUE)
			{
				/* start conversion in seperate thread */

				Convert_Binary_Log_to_Text_Log(CoreObj, ConvertInFileDir, ConvertInFileNamePrefix, (BOOL)ConvertFileType,
					FirstConvertFile, LastConvertFile, ConvertOutFileDir, "test");

				if (corefunc_global_var.Convertor_Start)
				{
					Converting = TRUE;

					ConversionTimer = SetTimer(hwnd, CONVERT_EVENT_ID, 1000, NULL);

					SetStatusBarText(STATUSBAR_CONVERT, "Converting...");
				}
			}

			if (i == 3)

				/* Abort */

				if (corefunc_global_var.Convertor_Start)
					corefunc_global_var.Force_Convertor_to_Stop = TRUE;

			return 0;

		case IDM_SETUPTIMINGDLG:

			ShowWindow(hwndTimedExperimentDialogBox, SW_SHOW);

			return 0;

		case IDM_SETUPIODLG:

			if (Recording)
				return 0;

			/* Open the aquisition parameters dialog box */

			i = (int)DialogBox(hAppInstance, "Dlg_IOParameters", hwnd, (DLGPROC)IOParametersSetup);

			/* if user pressed OK, flag this for the next time a recording is to start.*/

			if (i == TRUE)
				RecParamsChanged = TRUE;

			return 0;

		case IDM_LOADELECSETUP:

			strcpy_s(DialogBoxTitle, 80, "Load an Electrode Layout File");
			FileName[0] = 0;

			if (FileOpenDlg(hwnd, FT_ELECTRODELAYOUT, FileName, DialogBoxTitle) == FALSE)
				return 0;

			i = LoadMEALayoutFile(FileName, ElectrodesInfo, 64);

			if (i > 0)
			{
				PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
				PostMessage(hwnd, WM_USER_REFRESH, 0, 0);
			}
			return 0;

		case IDM_SETUPDIODISPLAY:

			/* Open the spike detection threshold dialog box */

			DialogBox(hAppInstance, "Dlg_DIODisplay", hwnd, (DLGPROC)DigitalIODisplaySetup);

			return 0;

		case IDM_SETUPTHRESHOLDS:

			/* Open the spike detection threshold dialog box */

			DialogBox(hAppInstance, "Dlg_ThresholdSetup", hwnd, (DLGPROC)ThresholdSetup);

			return 0;

		case IDM_RECALC_THRESHOLDS:

			/* if a threshold calculation is in progress, do nothing */

			if ((!Recording && !PlayingBack) || (corefunc_global_var.Calculating_Threshold > 0))
				return TRUE;

			WriteMessage("Recalculating thresholds for all electrodes", FALSE);

			Recalculate_Threshold_For_All_Electrodes(CoreObj, (int)(CurrentRecParms.ThresholdRMSMeasureDuration * 1000), CurrentRecParms.ThresholdRMSMultFactor);

			return 0;

		case IDM_QUIT:                  /* File | Quit */

			PostMessage(hwnd, WM_CLOSE, 0, 0L);

			return 0;

		case IDM_ABOUT:                 /* Help | About... */

			/* Open the "About" dialog box */

			DialogBox(hAppInstance, "Dlg_AboutBox", hwnd, (DLGPROC)AboutDlgProc);

			return 0;

		case IDM_ZOOMUP:				/* Context sensitive menus (right click) */

			CurrentRecParms.DisplayVoltageFullScale = CurrentRecParms.DisplayVoltageFullScale / 2;
			if (CurrentRecParms.DisplayVoltageFullScale < MINDISPLAYVOLT)
				CurrentRecParms.DisplayVoltageFullScale = MINDISPLAYVOLT;

			/* update displays */

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_ZOOMDOWN:

			CurrentRecParms.DisplayVoltageFullScale = CurrentRecParms.DisplayVoltageFullScale * 2;
			if (CurrentRecParms.DisplayVoltageFullScale > MAXDISPLAYVOLT)
				CurrentRecParms.DisplayVoltageFullScale = MAXDISPLAYVOLT;

			/* update displays */

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			return 0;

		case IDM_APPLY_THRESH_TO_ALL:

			for (i = 0; i < N_AD_CHANNELS; ++i)
				if (i < 60)  // Do not apply the threshold to AUX channels 
					corefunc_global_var.threshold_Per_Electrod[i] = corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum];

			/* update displays */

			PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
			PostMessage(hwnd, WM_USER_REFRESH, 0, 0);

			return 0;
		}

		break;

	case WM_TIMER:

		/* verify that the call is from the appropriate timer (an error here should never happen) */

		if (wParam != CONVERT_EVENT_ID)
			return TRUE;

		/* still converting ? continue to wait...*/

		if (corefunc_global_var.Convertor_Start)
			return TRUE;

		/* if not */

		Converting = FALSE;

		if (ConversionTimer)
			KillTimer(hwnd, ConversionTimer);
		ConversionTimer = 0;

		SetStatusBarText(STATUSBAR_CONVERT, "");

		return TRUE;

	case WM_INITMENUPOPUP:                  /* a Popup menu is being initialized */

		/* check and uncheck the appropriate items */

		switch (LOWORD(lParam))
		{

		case 0:				/* File  top level menu item  */

			EnableMenuItem(hMenu, IDM_LOGFILESETUP, (!Logging ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_PLAYBACKFILESETUP, (!PlayingBack || PlaybackPaused ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_LOADTIMEDEXP, (!TimedExperiment ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

			return 0;

		case 2:				/* Recording top level menu item  */

			EnableMenuItem(hMenu, IDM_START, (!Recording || !PlayingBack ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_STOP, (Recording && !PlayingBack ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_STARTLOGGING, (Recording && !Logging && !PlayingBack ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_STOPLOGGING, (Recording && Logging && !PlayingBack ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

			return 0;

		case 3:				/* Playback  top level menu item  */

			EnableMenuItem(hMenu, IDM_STARTPLAYBACK, (PlaybackFileLoaded && !Recording ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_PAUSEPLAYBACK, (PlaybackFileLoaded && (PlayingBack || PlaybackPaused) && !Recording ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_PLAYBACKFILESETUP, (!PlayingBack && !Recording ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

			return 0;

		case 4:				/* Plugins  top level menu item  */

			EnableMenuItem(hMenu, IDM_ENGAGE, (Recording && !Executing && DLL_Loaded ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_DISENGAGE, (Recording && Executing && DLL_Loaded ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_LOCATEPLUGIN, (!Executing ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_LOADDLL, (!Executing ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_UNLOADDLL, (!Executing && DLL_Loaded ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

			return 0;

		case 5:				/* Display top level menu item  */

			if (DisplayMode == DM_LEVEL)
			{
				CheckMenuItem(hMenu, IDM_DISP_CONTINUOUS, MF_UNCHECKED);
				CheckMenuItem(hMenu, IDM_DISP_LEVEL, MF_CHECKED);
			}
			else
			{
				CheckMenuItem(hMenu, IDM_DISP_CONTINUOUS, MF_CHECKED);
				CheckMenuItem(hMenu, IDM_DISP_LEVEL, MF_UNCHECKED);
			}

			if (LayoutMode == LM_LASTBIN)
			{
				CheckMenuItem(hMenu, IDM_LAYOUTALLBINS, MF_UNCHECKED);
				CheckMenuItem(hMenu, IDM_LAYOUTLASTBIN, MF_CHECKED);
			}
			else
			{
				CheckMenuItem(hMenu, IDM_LAYOUTALLBINS, MF_CHECKED);
				CheckMenuItem(hMenu, IDM_LAYOUTLASTBIN, MF_UNCHECKED);
			}

			return 0;

		case 7:				/* Setup top level menu item  */

			EnableMenuItem(hMenu, IDM_SETUPIODLG, ((!Recording && !PlayingBack) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_LOADELECSETUP, ((!Recording && !PlayingBack) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, IDM_RECALC_THRESHOLDS, (Recording || (PlayingBack && !PlaybackPaused) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

			return 0;
		}

		return 0;

	case WM_CLOSE:		           	/* Shut down application notice */

		/* Verify that the user actually wants to shut down the application, */

		i = MessageBox(hwnd, "Terminate Session: Are You Sure?",
			szAppName, MB_YESNO | MB_ICONINFORMATION);
		if (i != IDYES)
			return 0;

		/* close down application  */

		DestroyWindow(hwnd);

		return 0;

	case WM_DESTROY:                        /* shutting down */

		/* Unhook the message hook */

		if (hHook != NULL)
			UnhookWindowsHookEx(hHook);

		DAQ_CleanUp(CoreObj);

		PostQuitMessage(0);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


#pragma warning ( disable : 4100)

/* The window procedure of the interface window (which contains the buttons, etc.).
   The interface window is actually a modeless dialog box */

BOOL CALLBACK
InterfaceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int			val;
	char		msgstring[100];
	double		double_val;

	switch (message)
	{
	case WM_INITDIALOG:                     /* initialize */

		SetDlgItemText(hDlg, 304, PluginDLLFileName);
		SetDlgItemInt(hDlg, 402, (int)CurrentRecParms.TraceGraphTimeSlice, FALSE);
		SetDlgItemInt(hDlg, 403, (int)CurrentRecParms.SpikeRasterTimeSlice, FALSE);
		SetDlgItemInt(hDlg, 404, (int)CurrentRecParms.SpikeHistoMaxVal, FALSE);
		sprintf_s(msgstring, 100, "%.2f", CurrentRecParms.DisplayVoltageFullScale);
		SetDlgItemText(hDlg, 405, msgstring);
		CheckRadioButton(hDlg, IDM_DISP_CONTINUOUS, IDM_DISP_LEVEL,
			(DisplayMode == DM_CONTINUOUS ? IDM_DISP_CONTINUOUS : IDM_DISP_LEVEL));
		CheckRadioButton(hDlg, IDM_LAYOUTLASTBIN, IDM_LAYOUTALLBINS,
			(LayoutMode == LM_LASTBIN ? IDM_LAYOUTLASTBIN : IDM_LAYOUTALLBINS));

		/* limit the length of the Message window text */

		SendMessage(GetDlgItem(hDlg, 501), EM_SETLIMITTEXT, MAXMESSAGEDISPLAYBUF, 0);

		/* subclass the message log edit control in order to obtain double click messages */

		lpfnOriginalEditProc = SetWindowLongPtr(GetDlgItem(hDlg, 501), GWLP_WNDPROC, (LONG_PTR)MessageLogSubclassProc);

		return TRUE;

	case WM_USER_REFRESH:					/* enable or disable buttons according to current state */

		EnableWindow(GetDlgItem(hDlg, IDOK), !Recording && !PlayingBack);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), Recording && !PlayingBack);
		EnableWindow(GetDlgItem(hDlg, IDM_STARTLOGGING), Recording && !Logging && !PlayingBack && !TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_STOPLOGGING), Recording && Logging && !PlayingBack &&!TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_STARTPLAYBACK), !Recording && !Logging && PlaybackFileLoaded && !TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_PAUSEPLAYBACK), !Recording && !Logging && (PlayingBack || PlaybackPaused) && PlaybackFileLoaded &&!TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_ENGAGE), (Recording||PlayingBack) && !Executing && DLL_Loaded &&!TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_DISENGAGE), (Recording || PlayingBack) && Executing && DLL_Loaded &&!TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, IDM_LOADDLL), !Executing && !TimedExperiment && SendMessage(GetDlgItem(hDlg, 304), WM_GETTEXTLENGTH, 0, 0));
		EnableWindow(GetDlgItem(hDlg, IDM_UNLOADDLL), !Executing && !TimedExperiment && DLL_Loaded);
		EnableWindow(GetDlgItem(hDlg, IDM_STARTTIMEDEXP), !Logging && !Executing && !PlayingBack && !TimedExperiment && ActionEntries > 0);
		EnableWindow(GetDlgItem(hDlg, IDM_STOPTIMEDEXP), TimedExperiment);
		CheckDlgButton(hDlg, IDM_PAUSEPLAYBACK, PlaybackPaused);
		if (PlaybackPaused)
			SetDlgItemText(hDlg, IDM_PAUSEPLAYBACK, "Resume");
		else
			SetDlgItemText(hDlg, IDM_PAUSEPLAYBACK, "Pause");
		SetDlgItemInt(hDlg, 402, (int)CurrentRecParms.TraceGraphTimeSlice, FALSE);
		SetDlgItemInt(hDlg, 403, (int)CurrentRecParms.SpikeRasterTimeSlice, FALSE);
		SetDlgItemInt(hDlg, 404, (int)CurrentRecParms.SpikeHistoMaxVal, FALSE);
		sprintf_s(msgstring, 100, "%.2f", CurrentRecParms.DisplayVoltageFullScale);
		SetDlgItemText(hDlg, 405, msgstring);

		return 0;

	case WM_USER_NEWTEXTMESSAGE:

		RenderNewTextMessages(GetDlgItem(hDlg, 501));

		return 0;

	case WM_COMMAND:						/* messages from controls */
											/* In most cases these are simply relayed to the  main window */
		switch (LOWORD(wParam))
		{
		case IDM_STARTLOGGING:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STARTLOGGING, 0);
			return TRUE;

		case IDM_STOPLOGGING:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOPLOGGING, 0);
			return TRUE;

		case IDM_STARTTIMEDEXP:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STARTTIMEDEXP, 0);
			return TRUE;

		case IDM_STOPTIMEDEXP:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOPTIMEDEXP, 0);
			return TRUE;

		case IDM_ENGAGE:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_ENGAGE, 0);
			return TRUE;

		case IDM_DISENGAGE:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISENGAGE, 0);
			return TRUE;

		case IDM_LOCATEPLUGIN:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_LOCATEPLUGIN, 0);
			return TRUE;

		case IDM_LOADDLL:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_LOADDLL, 0);
			return TRUE;

		case IDM_UNLOADDLL:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_UNLOADDLL, 0);
			return TRUE;

		case IDM_DISP_CONTINUOUS:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISP_CONTINUOUS, 0);
			return TRUE;

		case IDM_DISP_LEVEL:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISP_LEVEL, 0);
			return TRUE;

		case IDM_LAYOUTLASTBIN:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_LAYOUTLASTBIN, 0);
			return TRUE;

		case IDM_LAYOUTALLBINS:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_LAYOUTALLBINS, 0);
			return TRUE;

		case IDM_STARTPLAYBACK:
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STARTPLAYBACK, 0);
			return TRUE;

		case IDM_PAUSEPLAYBACK:
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_PAUSEPLAYBACK, 0);
			return TRUE;

		case IDM_FREEZE:

			FreezeDisplay = IsDlgButtonChecked(hDlg, IDM_FREEZE);
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_FREEZE, 0);

			return TRUE;

		case IDM_CLEARSPIKES:
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_CLEARSPIKES, 0);
			return TRUE;

		case 304:												/* plugin file name edit control */
			if (HIWORD(wParam) == EN_CHANGE)
			{
				val = (int) SendMessage(GetDlgItem(hDlg, 304), WM_GETTEXTLENGTH, 0, 0);
				EnableWindow(GetDlgItem(hDlg, IDM_LOADDLL), !Executing && !TimedExperiment && val);
			}
			return TRUE;

		case 402:												/* trace graph duration */
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				GetDlgItemText(hDlg, 402, msgstring, 100);
				double_val = atof(msgstring);
				if (double_val > MAXTRACEDURATION || double_val < MINTRACEDURATION)
				{
					sprintf_s(msgstring, 100, "Please enter a value between %.2f and %.1f", MINTRACEDURATION, MAXTRACEDURATION);
					MessageBox(hDlg, msgstring, "Error", MB_ICONEXCLAMATION | MB_OK);
				}
				else
				{
					CurrentRecParms.TraceGraphTimeSlice = double_val;
					PostMessage(hwndMainWindow, WM_USER_REFRESH, 0, 0);
				}
			}
			return TRUE;

		case 403:												/* spike raster duration */
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				val = GetDlgItemInt(hDlg, 403, NULL, FALSE);
				if (val > MAXRASTERDURATION || val < MINRASTERDURATION)
				{
					sprintf_s(msgstring, 100, "Please enter a value between %d and %d", MINRASTERDURATION, MAXRASTERDURATION);
					MessageBox(hDlg, msgstring, "Error", MB_ICONEXCLAMATION | MB_OK);
				}
				else
				{
					CurrentRecParms.SpikeRasterTimeSlice = val;
					PostMessage(hwndMainWindow, WM_USER_REFRESH, 0, 0);
				}
			}
			return TRUE;

		case 404:												/* spike histo max count */
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				val = GetDlgItemInt(hDlg, 404, NULL, FALSE);
				if (val > MAXHISTORANGE || val < MINHISTORANGE)
				{
					sprintf_s(msgstring, 100, "Please enter a value between %d and %d", MINHISTORANGE, MAXHISTORANGE);
					MessageBox(hDlg, msgstring, "Error", MB_ICONEXCLAMATION | MB_OK);
				}
				else
				{
					CurrentRecParms.SpikeHistoMaxVal = val;
					PostMessage(hwndMainWindow, WM_USER_REFRESH, 0, 0);
				}
			}
			return TRUE;

		case 405:												/* Y scale  */
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				GetDlgItemText(hDlg, 405, msgstring, 100);
				double_val = atof(msgstring);
				if (double_val > MAXDISPLAYVOLT || double_val < MINDISPLAYVOLT)
				{
					sprintf_s(msgstring, 100, "Please enter a value between %.1f and %.1f", MINDISPLAYVOLT, MAXDISPLAYVOLT);
					MessageBox(hDlg, msgstring, "Error", MB_ICONEXCLAMATION | MB_OK);
				}
				else
				{
					CurrentRecParms.DisplayVoltageFullScale = double_val;
					PostMessage(hwndMainWindow, WM_USER_REFRESH, 0, 0);
				}
			}
			return TRUE;

		case IDOK:
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_START, 0);
			return TRUE;

		case IDCANCEL:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOP, 0);
			return TRUE;
		}

		break;

	case WM_DESTROY:

		/* un-subclass the message log edit control */

		SetWindowLongPtr(GetDlgItem(hDlg, 501), GWLP_WNDPROC, lpfnOriginalEditProc);

		return TRUE;
	}
	return FALSE;
}



/* The callback of the timed experiment sequence timer. Parses actions, dispatches and terminates them */

TIMERPROC
RunTimedExperimentEntry(HWND hWnd, INT nMsg, UINT nIDEvent, DWORD dwTime)
{
	__int64			ticknow;
	char			str[100];
	int				s;

	if (TimedExperiment == FALSE || ActionEntries == 0 || CurrActionEntry >= ActionEntries)
		return 0;

	/* start the execution of an action */

	if (TimedActionInProgress == FALSE)
	{
		if (ActionTable[CurrActionEntry].active == FALSE)	/* an inactive action */
		{
			CurrActionEntry++;
		}
		else
		{
			ticknow = GetTickCount64();
			LastTick = ticknow + (_int64)(ActionTable[CurrActionEntry].duration * 60000.0);

			switch (ActionTable[CurrActionEntry].action)
			{
			case ACTION_PRINT:
				WriteMessage(ActionTable[CurrActionEntry].argstring, FALSE);
				break;

			case ACTION_LOG:
				if (!Recording)
					PostMessage(hwndMainWindow, WM_COMMAND, IDM_START, 0);
				PostMessage(hwndMainWindow, WM_COMMAND, IDM_STARTLOGGING, 0);
				break;

			case ACTION_LOADDLL:

				if (strlen(ActionTable[CurrActionEntry].argstring) > 0)
					SetWindowText(GetDlgItem(hwndControlDialogBox, 304), ActionTable[CurrActionEntry].argstring);

				PostMessage(hwndMainWindow, WM_COMMAND, IDM_LOADDLL, 0);

				break;

			case ACTION_LOGANDEXEC:
				if (!Recording)
					PostMessage(hwndMainWindow, WM_COMMAND, IDM_START, 0);

				PostMessage(hwndMainWindow, WM_COMMAND, IDM_STARTLOGGING, 0);

				if (strlen(ActionTable[CurrActionEntry].argstring) > 0)
					SetWindowText(GetDlgItem(hwndControlDialogBox, 305), ActionTable[CurrActionEntry].argstring);

				PostMessage(hwndMainWindow, WM_COMMAND, IDM_ENGAGE, 0);

				break;

			case ACTION_PAUSE:

				break;
			}
			TimedActionInProgress = TRUE;
			InvalidateRect(hwndTimedExperimentDialogBox, NULL, TRUE);
		}
	}

	/* stop the execution of an action if its duration has elapsed*/

	if (TimedActionInProgress == TRUE)
	{
		ticknow = GetTickCount64();
		if (ticknow <= LastTick)		/* the duration defined for the action has still not passed */
		{
			s = (int)((LastTick - ticknow) / 1000);
			if (s != CountdownLast)
			{
				/* update the countdown in the status bar */

				sprintf_s(str, 100, "Experiment in Progress: %d Seconds to Next Action", CountdownLast);
				SetStatusBarText(STATUSBAR_TIMED, str);
				CountdownLast = s;
			}
			return 0;
		}

		/* the duration for the action has passsed */

		switch (ActionTable[CurrActionEntry].action)
		{
		case ACTION_PRINT:
			break;

		case ACTION_LOG:
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOPLOGGING, 0);
			break;

		case ACTION_LOADDLL:
			break;

		case ACTION_LOGANDEXEC:

			PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISENGAGE, 0);
			PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOPLOGGING, 0);

			break;

		case ACTION_PAUSE:
			break;
		}

		/* progress to next action */

		CurrActionEntry++;
		TimedActionInProgress = FALSE;
		InvalidateRect(hwndTimedExperimentDialogBox, NULL, TRUE);
	}

	/* no more actions to run? quit */

	if (CurrActionEntry >= ActionEntries)
	{
		WriteMessage("Timed Experiment Completed", FALSE);

		/* post a message to stop the experiment  */

		PostMessage(hwndMainWindow, WM_COMMAND, IDM_STOPTIMEDEXP, 0);
	}

	return 0;
}

/* --------------------------------------- Status bar ---------------------------------------*/

/* Create and Initialize the Status bar */

HWND
InitStatusBar(HWND hwndParent)
{
	HWND	hwndSB;

	hwndSB = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_BOTTOM | SBARS_SIZEGRIP,
		"", hwndParent, ID_STATUSBAR);

	return hwndSB;
}


/* divide the status bar into parts, whose dimensions are defined by the current 
   size of the main window and its state (maximized or not) */

void
DefineStatusBarParts(int xclient)
{
	int		aWidths[STATUSBAR_PARTSNUMBER];
	int		margin;

	/* if the window is maximized, leave no right margin.
	if not, leave a margin for the status bar grip bitmap */

	if (IsZoomed(hwndMainWindow))
		margin = 0;
	else
		margin = GetSystemMetrics(SM_CXVSCROLL);

	aWidths[0] = 146;
	aWidths[1] = 293;
	aWidths[2] = 440;
	aWidths[3] = 587;
	aWidths[4] = 733;
	aWidths[5] = 880;
	aWidths[6] = xclient - margin;

	/* set the parts */

	SendMessage(hwndStatusBar, SB_SETPARTS, STATUSBAR_PARTSNUMBER, (LPARAM)aWidths);
}


/* type out some text on a specific part of the status bar */

BOOL
SetStatusBarText(int part, char *str)
{
	if (part > STATUSBAR_PARTSNUMBER)
		return FALSE;

	return (BOOL)(SendMessage(hwndStatusBar, SB_SETTEXT, part, (LPARAM)(LPSTR)str));
}


/* ---------------------------- Core function initialization  -------------------------------*/

/* initialize the Core function object and its data structures */

void
InitCoreFunctionDataStructures(CoreFunction *cf)
{
	/* free old memory if allocated before */

	DAQ_CleanUp(cf);

	/* set up the parameters */

	strcpy_s(corefunc_global_var.BoardModelString, 50, CurrentRecParms.BoardModelString);

	corefunc_global_var.MAX_V = CurrentRecParms.VoltageFullScale;

	corefunc_global_var.Input_Hz = CurrentRecParms.BoardSampleRate;
	corefunc_global_var.Output_Hz = CurrentRecParms.BoardAOFreq;

	corefunc_global_var.buffer_Input_Per_Channel = CurrentRecParms.InputBufferSizePerChannel;
	corefunc_global_var.buffer_Output_Per_Channel = CurrentRecParms.OutputBufferSizePerChannel;

	DAQ_Set_Minutes_to_Remember_Volt_Trace(cf, CurrentRecParms.ContVoltageStorageDuration);
	DAQ_Set_Size_of_Spike_Train_Memory(cf, CurrentRecParms.SpikeHistoryLength);

	corefunc_global_var.Pre_Trigger_Samples = (int)((double)CurrentRecParms.SpikeTracePreTrigger * (double)CurrentRecParms.SampleRate / 1000.0 + 0.5);
	corefunc_global_var.Post_Trigger_Samples = (int)((double)(CurrentRecParms.SpikeTraceDuration - CurrentRecParms.SpikeTracePreTrigger) * (double)CurrentRecParms.SampleRate / 1000.0 + 0.5);

	DAQ_Set_DAQpath(cf, CurrentRecParms.DAQCardPath);
	DAQ_Set_Analog_OutputChannels(cf, CurrentRecParms.DAQAnalogOutputChannels);
	DAQ_Set_Analog_InputChannels(cf, CurrentRecParms.DAQAnalogInputChannels);

	corefunc_global_var.RMS_Factor = CurrentRecParms.ThresholdRMSMultFactor;
	corefunc_global_var.Spike_refractory_in_MicroSecond = CurrentRecParms.SpikeRefractoryPeriodMicrosecond;

	strcpy_s(corefunc_global_var.Spike_Extention_File, 10, CurrentRecParms.SpikesFileExt);
	strcpy_s(corefunc_global_var.Volt_IN_Extention_File, 10, CurrentRecParms.AnalogInputFileExt);
	strcpy_s(corefunc_global_var.Volt_OUT_Extention_File, 10, CurrentRecParms.AnalogOutFileExt);
	strcpy_s(corefunc_global_var.Digital_Extention_File, 10, CurrentRecParms.DigitalIOFileExt);
	strcpy_s(corefunc_global_var.UserDataFile_Extention_File, 10, CurrentRecParms.UserDataFileExt);
	strcpy_s(corefunc_global_var.Spike_Time_file, 10, CurrentRecParms.SpikeTimesExt);

	corefunc_global_var.LastChannelToInclude = CurrentRecParms.LastChannelToInclude;

	/* initialize the Core object  */

	if (Core_Init(cf) < 0)
		return;
}


/* update global variables based on those loaded from file */

void
SynctoDAQDataStructures(void)
{
	/* copy and convert the parameters */

	CurrentRecParms.BoardSampleRate = (int)corefunc_global_var.Input_Hz;
	CurrentRecParms.SampleRate = CurrentRecParms.BoardSampleRate;

	corefunc_global_var.Pre_Trigger_Samples = (int)((double)CurrentRecParms.SpikeTracePreTrigger * (double)CurrentRecParms.SampleRate / 1000.0 + 0.5);
	corefunc_global_var.Post_Trigger_Samples = (int)((double)(CurrentRecParms.SpikeTraceDuration - CurrentRecParms.SpikeTracePreTrigger) * (double)CurrentRecParms.SampleRate / 1000.0 + 0.5);
}


/* --------------------- Keyboard hook and subclassing functions ----------------------------*/

/* a keyboard  message hook for intercepting all keyboard input to main window
   as well as dialog boxes. The keyboard events trapped by this procedure are
   not passed on to the applications message loops */

LRESULT CALLBACK
KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	/* ignore messages delivered when Alt key was depressed and when keys are released */

	if ((code != HC_ACTION) || (lParam & 0xA0000000))
		return CallNextHookEx(hHook, code, wParam, lParam);

	switch (wParam)
	{
	case VK_F9:
		PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISP_CONTINUOUS, 0);
		break;

	case VK_F10:
		PostMessage(hwndMainWindow, WM_COMMAND, IDM_DISP_LEVEL, 0);
		break;

	case VK_ADD:
		PostMessage(hwndMainWindow, WM_COMMAND, IDM_ZOOMUP, 0);
		break;
	
	case VK_SUBTRACT:
		PostMessage(hwndMainWindow, WM_COMMAND, IDM_ZOOMDOWN, 0);
		break;
	};
	return CallNextHookEx(hHook, code, wParam, lParam);
}

/* the subclass window procedure for the  message log edit control;  used to trap double click events */

BOOL CALLBACK
MessageLogSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* trap doubleclick messages*/

	switch (message)
	{
	case WM_LBUTTONDBLCLK:

		PostMessage(hwndMainWindow, WM_COMMAND, IDM_ADDTEXTENTRY, 0);

		return 0;

	default:
		break;

	}

	/* pass the message to the original windows procedure for further processing */

	return (BOOL)CallWindowProc((WNDPROC)lpfnOriginalEditProc, hwnd, message, wParam, lParam);
}


/* ---------------------------- helper functions --------------------------------------------*/


/* Register all application window classes */

void
RegisterApplicationWindowClasses(HINSTANCE hInstance)
{
	WNDCLASSEX    	wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, "IDI_CLEM");
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName = szAppName;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(hInstance, "IDI_CLEM");

	RegisterClassEx(&wndclass);

	/* register trace graph window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = TraceGraphWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMGraphWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register single trace graph window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = TraceSingleWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMSingleWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register spike raster window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = SpikeRasterWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMRasterWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register spike histogram window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = SpikeHistogramWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMHistoWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register Digital IO graph window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = DIOGraphWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMDIOGraphWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register user graph window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = UserGraphWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMUserGraphWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	/* register MEA layout window class */

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = MEALayoutWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "CLEMMEALayoutWindow";
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);
}

void EndOfPlayback(void)
{
	if (Executing)
		SendMessage(hwndMainWindow, WM_COMMAND, IDM_DISENGAGE, 0);

	if (Logging)
		DAQ_Stop_ALL_Recording(CoreObj);

	if (DisplayTimerID)
		KillTimer(hwndMainWindow, DisplayTimerID);
	DisplayTimerID = 0;

	PlayingBack = FALSE;

	PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
	PostMessage(hwndMainWindow, WM_USER_REFRESH, 0, 0);

} 


