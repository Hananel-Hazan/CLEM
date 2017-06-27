/*
Closed Loop Experiment Manager

GraphWindow.c

Window procedures of all data rendering windows
Functions responsible for rendering the display
Note: Most rendering functions draw onto offscreen bitmaps 
	  that are therafter copied to display


Public functions:
----------------
TraceGraphWndProc()
TraceSingleWndProc()
SpikeRasterWndProc()
SpikeHistogramWndProc()
DIOGraphWndProc()
UserGraphWndProc()
MEALayoutWndProc()
WriteMessage()
RenderNewTextMessages ()
InitializeCLEMGraphics ()
CleanupCLEMGraphics()
ClearDisplayedSpikes()
ResetGraphicsData ()


*/


#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdBOOL.h>

#pragma hdrstop

#include "CLEM.h"
#include "GraphWindow.h"
#include "files.h"
#include "CoreFunction/GlobalVar.h"

/* local constants and data structures */

#define	 MAX_TEXT_MSGS			500
#define  TEXTMESSAGELEN			256

typedef struct
{
	int		    y_position;
	int		    pencolor;
	float		*datapoint;
	int			env_x;
	int			env_y;
	int			spike_env_count;
	int			spike_env_last;
} TRACEGRAPHDATA;

/* text message queue structure */

typedef struct
{
	char	str[TEXTMESSAGELEN ];
	int		length;
}   TEXTMESSAGEITEM;

/* local function prototypes */

void		    InitializeGraphicObjects(void);
void		    DestroyGraphicObjects(void);
BOOL		    CreateDisplayBitmaps(void);
void		    DestroyDisplayBitmaps(void);
void		    UpdateTraceGraphData(void);
void		    RenderSpikeRaster(void);
void		    RenderRasterHistogram(void);
void		    RenderUserGraph(void);
void		    RenderMEALayout(BOOL fullupdate);
void		    RenderTraceData(void);
void			RenderSpikeEnvelopes(void);
void			RenderTraceSingle(void);
void			RenderSpikeEnvelopeSingle(void);
void			RenderDigitalIO (void);
int				GetElectrodeFromLayout(int clicked_x, int clicked_y);
BOOL			IsPtNearThreshold(int electrode, int y);
double			ConvertPtToThreshold(int y);
void		    DrawLineBresenham(int x1, int y1, int xn, int yn, LPBITMAPINFO bminfo, PVOID pixels, int color_index);
BOOL			AddTextMessageToQueue(char *str);
int				GetTextMessageFromQueue(char *str);
void			ClearTextMsgQueue(void);

/* global variables private to this file */

static TRACEGRAPHDATA	TraceGraphDataArray[N_AD_CHANNELS];
static float			*TraceGraphDataMemBlock;
static float			*TraceGraphSpikeEnvMemBlock;
static __int64			*TraceGraphSpikeTimesMemBlock;
static int				HistogramData[MAXRASTERUNITS];
static int				LayoutHistogram[N_AD_CHANNELS];
static double			UserGraph1[MAXRASTERUNITS];
static double			UserGraph2[MAXRASTERUNITS];
static int				RasterUnits = 120;
static __int64			LastVisitedVoltSample;
static __int64			LastVisitedVoltSampleTime, PrevLastVisitedVoltSampleTime;
static __int64			LastVisitedSpike;
static __int64			FirstRasterVoltSampleTime;
static __int64			SpikesInBuffer;
static RGBQUAD			spectrum[64];
static RGBQUAD			spectrumLUT[256];
static int				TraceGraphWidth, TraceGraphHeight;
static int				TraceSingleWidth, TraceSingleHeight;
static int				SpikeRasterWidth, SpikeRasterHeight;
static int				SpikeHistogramWidth, SpikeHistogramHeight;
static int				DIOGraphWidth, DIOGraphHeight;
static int				UserGraphWidth, UserGraphHeight;
static int				MEALayoutWidth, MEALayoutHeight;
static int				HistogramOffset;
static PCHAR			MessageWindowText;
static HDC				TraceGraphWindowDC;
static HDC				TraceSingleWindowDC;
static HDC				SpikeRasterWindowDC;
static HDC				SpikeHistogramWindowDC;
static HDC			    DIOGraphWindowDC;
static HDC				UserGraphWindowDC;
static HDC				MEALayoutWindowDC;
static TEXTMESSAGEITEM  *TextMsgQueue;
static int				FirstTextMsg = 0;
static int				LastTextMsg = 0;
static int				nTextMsgs = 0;
static PVOID			pDIB_TracePixels, pDIB_SinglePixels, pDIB_RasterPixels, pDIB_HistoPixels, pDIB_MEALayoutPixels, pDIB_UserGraphPixels, pDIB_DIOGraphPixels;
static LPBITMAPINFO		pDIB_TraceInfo, pDIB_SingleInfo, pDIB_RasterInfo, pDIB_HistoInfo, pDIB_MEALayoutInfo, pDIB_UserGraphInfo, pDIB_DIOGraphInfo;
static RGBQUAD			PenColors[11] = { { 148, 138, 84, 0 }, { 85, 142, 213, 0 }, { 149, 179, 215, 0 }, { 217, 150, 148, 0 },
										  { 195, 214, 155, 0 }, { 179, 162, 199, 0 }, { 147, 205, 221, 0 }, { 250, 192, 144, 0 },
										  { 128, 128, 128, 128 }, { 255, 0, 0, 0 }, { 0, 0, 0, 0 } };

static int AntialieasedCircle[10][10] = {
	{ 0, 0, 0, 104, 190, 199, 112, 8, 0, 0 },
	{ 0, 32, 222, 245, 255, 255, 248, 237, 48, 0 },
	{ 8, 225, 255, 255, 255, 255, 255, 255, 234, 32 },
	{ 154, 251, 255, 255, 255, 255, 255, 255, 255, 181 },
	{ 222, 255, 255, 255, 255, 255, 255, 255, 255, 246 },
	{ 235, 255, 255, 255, 255, 255, 255, 255, 255, 251 },
	{ 181, 255, 255, 255, 255, 255, 255, 255, 255, 207 },
	{ 32, 234, 255, 255, 255, 255, 255, 255, 239, 64 },
	{ 0, 78, 241, 255, 255, 255, 255, 240, 106, 0 },
	{ 0, 0, 24, 181, 246, 251, 190, 48, 0, 0 } };

#pragma warning ( disable : 4100)

/* TraceGraphWindow window callback procedure */

LRESULT CALLBACK
TraceGraphWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC             hdc;
	PAINTSTRUCT     ps;
	POINT			pt;
	int				i, j, dx, dy;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndTraceGraphWindow = hwnd;

		/* create a private device context for this window */

		TraceGraphWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		TraceGraphWidth = LOWORD(lParam);
		TraceGraphHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the graph */

		if (DisplayMode == DM_CONTINUOUS)
			RenderTraceData();

		if (DisplayMode == DM_LEVEL)
			RenderSpikeEnvelopes();

		return 0;

	case WM_LBUTTONDBLCLK:               /* left mouse button double clicked */

		/* get the cursors Y coordinate */

		pt.y = HIWORD(lParam);
		pt.x = LOWORD(lParam);

		/* figure out the data of which channel was clicked on */

		j = -1;

		if (DisplayMode == DM_CONTINUOUS)
		{
			dy = 10000;
			dx = 10000;
			for (i = 0; i < N_AD_CHANNELS; ++i)
			{
				if (ElectrodesInfo[i].inuse == FALSE)
					continue;
				
				if (abs(TraceGraphDataArray[i].y_position - pt.y) < dy)
				{
					dy = abs(TraceGraphDataArray[i].y_position - pt.y);
					j = i;
				}
			}
		}

		if (DisplayMode == DM_LEVEL)
		{
			dy = (int)((double)pt.y / ((double)TRACEGRAPH_Y / (double)(ROWS_LEVEL + 1)));
			dx = (int)((double)pt.x / ((double)TRACEGRAPH_X / (double)COLS_LEVEL));

			j = dy * COLS_LEVEL + dx;
		}

		/* refresh the window that shows the magnified version of the selected trace
		   and the MEA layout window so it highlights the selected channel */

		if (j >= 0 && j < N_AD_CHANNELS)
		{
			SelectedTraceNum = j;

			InvalidateRect(hwndTraceSingleWindow, NULL, TRUE);
			UpdateWindow(hwndTraceSingleWindow);

			InvalidateRect(hwndMEALayoutWindow, NULL, TRUE);
			UpdateWindow(hwndMEALayoutWindow);
		}

		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, TraceGraphWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* TraceSingleWindow window callback procedure */

LRESULT CALLBACK
TraceSingleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU		hBoxMenu, hStatusMenu;
	static BOOL			MovingTH = FALSE;
	HDC                 hdc;
	POINT				pt;
	PAINTSTRUCT         ps;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndTraceSingleWindow = hwnd;

		/* create a private device context for this window */

		TraceSingleWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		TraceSingleWidth = LOWORD(lParam);
		TraceSingleHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the graph */

		if (DisplayMode == DM_CONTINUOUS)
			RenderTraceSingle();

		if (DisplayMode == DM_LEVEL)
			RenderSpikeEnvelopeSingle();

		return 0;

	case WM_LBUTTONDOWN:					/* left mouse button depressed */

		/* store the cursors coordinates */

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		/* check if the user cleared near the Threshold line */

		if (IsPtNearThreshold(SelectedTraceNum, pt.y) == FALSE)
			return 0;

		MovingTH = TRUE;

		/* capture the mouse */

		SetCapture(hwnd);

		return 0;

	case WM_MOUSEMOVE:						/* mouse moved */

		if (!MovingTH)
			return 0;

		/* store the cursors coordinates */

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		/* adjust the threshold position accordingly */

		corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum] = (float) ConvertPtToThreshold(pt.y);

		/* redraw the display */

		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);

		return 0;

	case WM_LBUTTONUP:                      /* left mouse button released */

		if (!MovingTH)
			return 0;

		MovingTH = FALSE;

		/* release the cursor */

		ReleaseCapture();

		/* redraw the display */

		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);

		return 0;

	case WM_CONTEXTMENU:            	/* right mouse button down */

		/* store the coordinates at which the right mouse button was pressed*/

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		StoredCursorPt = pt;
		ScreenToClient(hwnd, &StoredCursorPt);

		/* create a Popup menu at the current cursor position */

		hPopupMenu = CreatePopupMenu();
		if (hPopupMenu == NULL)
			return 0;

		/* Build the menu */

		AppendMenu(hPopupMenu, MF_ENABLED | MF_STRING, IDM_DISP_CONTINUOUS, "Continous");
		AppendMenu(hPopupMenu, MF_ENABLED | MF_STRING, IDM_DISP_LEVEL, "Level");
		AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hPopupMenu, MF_ENABLED | MF_STRING, IDM_ZOOMUP, "Decrease Vertical Range");
		AppendMenu(hPopupMenu, MF_ENABLED | MF_STRING, IDM_ZOOMDOWN, "Increase Vertical Range");
		AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hPopupMenu, MF_ENABLED | MF_STRING, IDM_APPLY_THRESH_TO_ALL, "Apply Threshold to All Channels");

		/* track the menu with the right mouse button */

		TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			pt.x, pt.y, 0, hwndMainWindow, NULL);

		/* destroy the menu */

		DestroyMenu(hPopupMenu);
		hPopupMenu = NULL;

		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, TraceSingleWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* SpikeRasterWindow window callback procedure */

LRESULT CALLBACK
SpikeRasterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC                 hdc;
	PAINTSTRUCT         ps;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndSpikeRasterWindow = hwnd;

		SpikeRasterWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		SpikeRasterWidth = LOWORD(lParam);
		SpikeRasterHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the Raster */

		RenderSpikeRaster();

		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, SpikeRasterWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* SpikeHistogramWindow window callback procedure */

LRESULT CALLBACK
SpikeHistogramWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC                 hdc;
	PAINTSTRUCT         ps;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndSpikeHistogramWindow = hwnd;

		/* create a private device context for this window */

		SpikeHistogramWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		SpikeHistogramWidth = LOWORD(lParam);
		SpikeHistogramHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the Raster */

		RenderRasterHistogram();

		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, SpikeHistogramWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* Digital Input/output window callback procedure */


LRESULT CALLBACK
DIOGraphWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC                 hdc;
	PAINTSTRUCT         ps;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndDIOGraphWindow = hwnd;

		/* create a private device context for this window */

		DIOGraphWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		DIOGraphWidth= LOWORD(lParam); 
		DIOGraphHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		RenderDigitalIO ();
	
		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, DIOGraphWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* user graph window procedure */

LRESULT CALLBACK
UserGraphWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC                 hdc;
	PAINTSTRUCT         ps;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndUserGraphWindow = hwnd;

		/* create a private device context for this window */

		UserGraphWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		UserGraphWidth = LOWORD(lParam);
		UserGraphHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the Raster */

		RenderUserGraph();
		
		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, UserGraphWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/* MEALayoutWindow callback procedure */


LRESULT CALLBACK
MEALayoutWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC                 hdc;
	PAINTSTRUCT         ps;
	POINT				pt;
	int					clickedon_elec;

	switch (message)
	{                                           /* window is being created */
	case WM_CREATE:

		/* store a global copy of the windows handle */

		hwndMEALayoutWindow = hwnd;

		/* create a private device context for this window */

		MEALayoutWindowDC = GetDC(hwnd);

		return 0;

	case WM_SIZE:                           /* windows size has changed */

		/* get the new dimensions of the window, store in global variables */

		MEALayoutWidth = LOWORD(lParam);
		MEALayoutHeight = HIWORD(lParam);

		return 0;

	case WM_PAINT:                          /* redraw the window */

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

		/* Render the Raster */

		RenderMEALayout(TRUE);

		return 0;

	case WM_LBUTTONDBLCLK:               /* left mouse button double clicked */

		/* get the cursors Y coordinate */

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		/* determine which electrode was clicked on */

		clickedon_elec = GetElectrodeFromLayout(pt.x, pt.y);

		/* refresh the window that shows the magnified version of the selected trace
		   and the MEA layout window so it highlights the selected channel */

		if (clickedon_elec >= 0)
		{
			SelectedTraceNum = clickedon_elec;

			InvalidateRect(hwndTraceSingleWindow, NULL, TRUE);
			UpdateWindow(hwndTraceSingleWindow);

			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
		}

		return 0;

	case WM_DESTROY:                        /* shutting down */

		ReleaseDC(hwnd, MEALayoutWindowDC);

		return 0;
	}

	/* default dealing with all the other messages */

	return DefWindowProc(hwnd, message, wParam, lParam);
}

/* -------------------  Text Message handling functions ---------------------------- */

/* To improve performance, text is not rendered in threads that calls this function.
   Instead, the text string is placed in a queue and rendered later by the GUI thread */

BOOL
WriteMessage(char *messagetxt, BOOL erasealltext)
{
	SYSTEMTIME		st;
	char			msgstr[TEXTMESSAGELEN ];
	BOOL			result;

	if (MessageWindowText == NULL)
		return FALSE;

	if (strnlen_s(messagetxt, MAXTEXTMESSAGELEN) >= MAXTEXTMESSAGELEN)
		return FALSE;

	/* if this value is set to true, all prior text is erased */

	if (erasealltext)
	{
		ClearTextMsgQueue();
		SendMessage(GetDlgItem (hwndControlDialogBox, 501), WM_SETTEXT, 0, (LPARAM) "");
	}

	if (strnlen_s(messagetxt, MAXTEXTMESSAGELEN) < 1)
		return TRUE;

	/* add a time stamp */

	GetLocalTime(&st);
	sprintf_s(msgstr, TEXTMESSAGELEN , "[%02u:%02u:%02u] %s\r\n", st.wHour, st.wMinute, st.wSecond, messagetxt);

	/* add the text string to the queue and notify the interface dialog box */

	result = AddTextMessageToQueue (msgstr);

	PostMessage(hwndControlDialogBox, WM_USER_NEWTEXTMESSAGE, 0, 0);
	
	return result;
}


/* Draw the new text message in the Text Message window. 
   This function is called from the GUI thread */

void
RenderNewTextMessages(HWND hEdit)
{
	int		n, maxlength;
	char	str[TEXTMESSAGELEN];
	int		msg_str_len;
	PCHAR	p;

	maxlength = (int)SendMessage(hEdit, EM_GETLIMITTEXT, 0, 0);

	/* render all messages waiting in queue */

	while ((msg_str_len = GetTextMessageFromQueue(str)) > 0)
	{
		/* append the message to the Text Message File */
		
		if (LogMessages)
			AppendTextMessageToFile(TextLogFileName, msg_str_len, str);

		n = (int) SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);

		/*	Verify that the new message does not cause a buffer overflow for the edit control.
		    If it does, the earliest messages in the Text Message window (an Edit control) 
			are removed one by one until the new text fits */

		while (n + msg_str_len >= maxlength)
		{
			SendMessage(hEdit, WM_GETTEXT, MAXMESSAGEDISPLAYBUF, (LPARAM) MessageWindowText);
			
			/* find the last character of the earliest message */

			p = MessageWindowText;
			while (((p - MessageWindowText) < MAXMESSAGEDISPLAYBUF) && (*p != '\n'))
				++p;

			/* copy back the text without the first message */

			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)(p + 1));

			/* get the new length of the text */

			n = (int)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
		}

		/* place the entry point in the Message window at the end and insert the 
		   new text at this point */

		SendMessage(hEdit, EM_SETSEL, n, n);
		SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)str);
	} 
}

/* -------------------- initialization and cleanup functions ----------------------- */


BOOL
InitializeCLEMGraphics(void)
{
	int			i;
	double		y_spacing, env_x_spacing, env_y_spacing;
	LPVOID		hHeap;

	InitializeGraphicObjects();

	if (CreateDisplayBitmaps() == FALSE)
		return FALSE;

	/* allocate memory blocks of voltage traces and spike detections */

	hHeap = GetProcessHeap();

	if ((TraceGraphDataMemBlock = (float *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, DATA_POINTS_PER_TRACE * N_AD_CHANNELS * sizeof(float))) == NULL)
		return FALSE;

	if ((TraceGraphSpikeEnvMemBlock = (float *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, DISPLAY_SAMPLS_PER_SPIKE * MAX_DISPLAY_SPIKES * N_AD_CHANNELS * sizeof(float))) == NULL)
	{
		HeapFree(hHeap, 0, (LPVOID)TraceGraphDataMemBlock);
		return FALSE;
	}

	if ((TraceGraphSpikeTimesMemBlock = (__int64 *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_DISPLAY_SPIKES * N_AD_CHANNELS * sizeof(__int64))) == NULL)
	{
		HeapFree(hHeap, 0, (LPVOID)TraceGraphDataMemBlock);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeEnvMemBlock);
		return FALSE;
	}

	if ((MessageWindowText = (PCHAR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAXMESSAGEDISPLAYBUF)) == NULL)
	{
		HeapFree(hHeap, 0, (LPVOID)TraceGraphDataMemBlock);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeEnvMemBlock);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeTimesMemBlock);
		return FALSE;
	}

	if ((TextMsgQueue = (TEXTMESSAGEITEM *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MAX_TEXT_MSGS * sizeof(TEXTMESSAGEITEM))) == NULL)
	{
		HeapFree(hHeap, 0, (LPVOID)MessageWindowText);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphDataMemBlock);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeEnvMemBlock);
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeTimesMemBlock);
		return FALSE;
	}

	/* calculate and store some fixed info regarding each displayed channel */

	y_spacing = (double)TRACEGRAPH_Y / (double)(N_AD_CHANNELS + 2);

	env_x_spacing = (double)TRACEGRAPH_X / (double)COLS_LEVEL;
	env_y_spacing = (double)TRACEGRAPH_Y / (double)(ROWS_LEVEL + 1);

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		/* store addresses of voltage data and spike data  */

		TraceGraphDataArray[i].datapoint = TraceGraphDataMemBlock + i * DATA_POINTS_PER_TRACE;

		/* clear the spike data counters */

		TraceGraphDataArray[i].spike_env_count = 0;
		TraceGraphDataArray[i].spike_env_last = 0;

		/* calculate the vertical postion of the graph in the display window */

		TraceGraphDataArray[i].y_position = (int)((i + 1) * y_spacing);

		TraceGraphDataArray[i].env_x = (int)((i % COLS_LEVEL) * env_x_spacing);
		TraceGraphDataArray[i].env_y = (int)((i / COLS_LEVEL + 1) * env_y_spacing);

		/* store the  pen to use when rendering the trace*/

		TraceGraphDataArray[i].pencolor = i % 7;
	}

	/* reset the data of each electrode */

	ResetGraphicsData();

	return TRUE;
}


void
CleanupCLEMGraphics(void)
{
	LPVOID		hHeap;

	DestroyGraphicObjects();

	DestroyDisplayBitmaps();

	hHeap = GetProcessHeap();

	if (TextMsgQueue != NULL)
		HeapFree(hHeap, 0, (LPVOID)TextMsgQueue);

	if (MessageWindowText != NULL)
		HeapFree(hHeap, 0, (LPVOID)MessageWindowText);

	if (TraceGraphSpikeTimesMemBlock != NULL)
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeTimesMemBlock);

	if (TraceGraphSpikeEnvMemBlock != NULL)
		HeapFree(hHeap, 0, (LPVOID)TraceGraphSpikeEnvMemBlock);

	if (TraceGraphDataMemBlock != NULL)
		HeapFree(hHeap, 0, (LPVOID)TraceGraphDataMemBlock);
}


/* -------------------- display resetting functions ----------------------- */


BOOL
ResetGraphicsData(void)
{
	int			i;

	/* reset all displayed data */

	memset(TraceGraphDataMemBlock, 0, DATA_POINTS_PER_TRACE * N_AD_CHANNELS * sizeof(float));
	memset(TraceGraphSpikeEnvMemBlock, 0, DISPLAY_SAMPLS_PER_SPIKE * MAX_DISPLAY_SPIKES * N_AD_CHANNELS * sizeof(float));
	memset(TraceGraphSpikeTimesMemBlock, 0, MAX_DISPLAY_SPIKES * N_AD_CHANNELS * sizeof(__int64));

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		TraceGraphDataArray[i].spike_env_count = 0;
		TraceGraphDataArray[i].spike_env_last = 0;
	}

	for (i = 0; i < RasterUnits; ++i)
		HistogramData[i] = 0;

	SpikesInBuffer = 0;

	LastVisitedVoltSample = -1;
	LastVisitedSpike = -1;
	LastVisitedVoltSampleTime = 0;
	FirstRasterVoltSampleTime = -1;

	return TRUE;
}


void
ClearDisplayedSpikes(void)
{
	int		i;

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		TraceGraphDataArray[i].spike_env_count = 0;
		TraceGraphDataArray[i].spike_env_last = 0;
	}
}


/* ===============================  internal functions ============================= */

/* -----------------------  Text Message handling functions ------------------------ */


/* Add a text message to the queue */

BOOL
AddTextMessageToQueue(char *str)
{
	int n;

	if (nTextMsgs == MAX_TEXT_MSGS)
		return FALSE;

	/* add message to queue */

	n = (int)strlen(str);
	strcpy_s(TextMsgQueue[LastTextMsg].str, TEXTMESSAGELEN, str);
	TextMsgQueue[LastTextMsg].length = n;

	/* update last message pointer */

	LastTextMsg = (LastTextMsg + 1) % MAX_TEXT_MSGS;
	nTextMsgs++;

	return TRUE;
}


/* retrieve the next text message from the queue: obtain the string and return the string length */

int
GetTextMessageFromQueue(char *str)
{
	int n;

	if (nTextMsgs == 0)
		return -1;

	/* obtain message */

	strcpy_s(str, TEXTMESSAGELEN, TextMsgQueue[FirstTextMsg].str);
	n = TextMsgQueue[FirstTextMsg].length;

	/* update pointer and counter */

	FirstTextMsg = (FirstTextMsg + 1) % MAX_TEXT_MSGS;
	nTextMsgs--;

	return n;
}


/* Clear the message queue */

void
ClearTextMsgQueue(void)
{
	nTextMsgs = 0;
	FirstTextMsg = LastTextMsg = 0;
}


/* ---------------------- Graphics initialization and cleanup functions ----------------------- */


void
InitializeGraphicObjects(void)
{
	int		red, green, blue, i;
	int 	spectincr[3][4] = { { 0, 4, 0, 0 },
								{ 4, 0, -4, 0 },
								{ -4, 0, 0, 4 } };	/* increments 3 luts x 4 segments */

	/* generate a pseudocolor spectrum lut */

	red = 0;
	green = 4;
	blue = 256;

	for (i = 0; i < 256; i++)
	{
		red = red + spectincr[0][i / 64];
		green = green + spectincr[1][i / 64];
		blue = blue + spectincr[2][i / 64];

		spectrumLUT[i].rgbRed = (BYTE)min(red, 255);
		spectrumLUT[i].rgbGreen = (BYTE)min(green, 255);
		spectrumLUT[i].rgbBlue = (BYTE)min(blue, 255);
		spectrumLUT[i].rgbReserved = PC_RESERVED;
	}

	spectrumLUT[0].rgbRed = 128;
	spectrumLUT[0].rgbGreen = 128;
	spectrumLUT[0].rgbBlue = 128;
	spectrumLUT[0].rgbReserved = PC_RESERVED;
}


void
DestroyGraphicObjects(void)
{
	/* nothing to do currently */

	return;
}


/* all graphics are first rendered to offscreen bitmaps which are then copied to the screen.
   allocate the necessary memory for these bitmaps */

BOOL
CreateDisplayBitmaps(void)
{
	LPVOID		hHeap;

	hHeap = GetProcessHeap();

	/* Allocate memory for the trace windows bitmap pixels.
	This is where the pixels composing the current traces are stored  */

	if ((pDIB_TracePixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, TRACEGRAPH_X * TRACEGRAPH_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_SinglePixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, TRACESINGLE_X * TRACESINGLE_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_RasterPixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, SPIKERASTER_X * SPIKERASTER_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_HistoPixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, SPIKEHISTOGRAM_X * SPIKEHISTOGRAM_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_MEALayoutPixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, MEALAYOUT_X * MEALAYOUT_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_UserGraphPixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY,  USERGRAPH_X * USERGRAPH_Y * 3L)) == NULL)
		return FALSE;

	if ((pDIB_DIOGraphPixels = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, DIOGRAPH_X * DIOGRAPH_Y * 3L)) == NULL)
		return FALSE;

	/* Allocate memory for the bitmap headers */

	pDIB_TraceInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_TraceInfo == NULL)
		return FALSE;

	pDIB_SingleInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_SingleInfo == NULL)
		return FALSE;

	pDIB_RasterInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_RasterInfo == NULL)
		return FALSE;

	pDIB_HistoInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_HistoInfo == NULL)
		return FALSE;

	pDIB_MEALayoutInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_MEALayoutInfo == NULL)
		return FALSE;

	pDIB_UserGraphInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_UserGraphInfo == NULL)
		return FALSE;

	pDIB_DIOGraphInfo = (LPBITMAPINFO)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (pDIB_DIOGraphInfo == NULL)
		return FALSE;

	/* initiate the Device Independent Bitmap info structures */

	pDIB_TraceInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_TraceInfo->bmiHeader.biWidth = TRACEGRAPH_X;
	pDIB_TraceInfo->bmiHeader.biHeight = TRACEGRAPH_Y;
	pDIB_TraceInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_TraceInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_TraceInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_TraceInfo->bmiHeader.biSizeImage = TRACEGRAPH_X * TRACEGRAPH_Y * 3L;
	pDIB_TraceInfo->bmiHeader.biClrUsed = 0;

	pDIB_SingleInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_SingleInfo->bmiHeader.biWidth = TRACESINGLE_X;
	pDIB_SingleInfo->bmiHeader.biHeight = TRACESINGLE_Y;
	pDIB_SingleInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_SingleInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_SingleInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_SingleInfo->bmiHeader.biSizeImage = TRACESINGLE_X * TRACESINGLE_Y * 3L;
	pDIB_SingleInfo->bmiHeader.biClrUsed = 0;

	pDIB_RasterInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_RasterInfo->bmiHeader.biWidth = SPIKERASTER_X;
	pDIB_RasterInfo->bmiHeader.biHeight = SPIKERASTER_Y;
	pDIB_RasterInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_RasterInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_RasterInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_RasterInfo->bmiHeader.biSizeImage = SPIKERASTER_X * SPIKERASTER_Y * 3L;
	pDIB_RasterInfo->bmiHeader.biClrUsed = 0;

	pDIB_HistoInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_HistoInfo->bmiHeader.biWidth = SPIKEHISTOGRAM_X;
	pDIB_HistoInfo->bmiHeader.biHeight = SPIKEHISTOGRAM_Y;
	pDIB_HistoInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_HistoInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_HistoInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_HistoInfo->bmiHeader.biSizeImage = SPIKEHISTOGRAM_X * SPIKEHISTOGRAM_Y * 3L;
	pDIB_HistoInfo->bmiHeader.biClrUsed = 0;

	pDIB_MEALayoutInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_MEALayoutInfo->bmiHeader.biWidth = MEALAYOUT_X;
	pDIB_MEALayoutInfo->bmiHeader.biHeight = MEALAYOUT_Y;
	pDIB_MEALayoutInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_MEALayoutInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_MEALayoutInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_MEALayoutInfo->bmiHeader.biSizeImage = MEALAYOUT_X * MEALAYOUT_Y * 3L;
	pDIB_MEALayoutInfo->bmiHeader.biClrUsed = 0;

	pDIB_UserGraphInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_UserGraphInfo->bmiHeader.biWidth = USERGRAPH_X;
	pDIB_UserGraphInfo->bmiHeader.biHeight = USERGRAPH_Y;
	pDIB_UserGraphInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_UserGraphInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_UserGraphInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_UserGraphInfo->bmiHeader.biSizeImage = USERGRAPH_X * USERGRAPH_Y * 3L;
	pDIB_UserGraphInfo->bmiHeader.biClrUsed = 0;

	pDIB_DIOGraphInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB_DIOGraphInfo->bmiHeader.biWidth = DIOGRAPH_X;
	pDIB_DIOGraphInfo->bmiHeader.biHeight = DIOGRAPH_Y;
	pDIB_DIOGraphInfo->bmiHeader.biPlanes = (WORD)1;
	pDIB_DIOGraphInfo->bmiHeader.biBitCount = (WORD)24;
	pDIB_DIOGraphInfo->bmiHeader.biCompression = BI_RGB;
	pDIB_DIOGraphInfo->bmiHeader.biSizeImage = DIOGRAPH_X * DIOGRAPH_Y * 3L;
	pDIB_DIOGraphInfo->bmiHeader.biClrUsed = 0;

	return TRUE;
}

/* free memory used for bitmaps */

void
DestroyDisplayBitmaps(void)
{
	LPVOID		hHeap;

	hHeap = GetProcessHeap();

	HeapFree(hHeap, 0, (LPVOID)pDIB_DIOGraphInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_UserGraphInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_MEALayoutInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_HistoInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_SingleInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_TraceInfo);
	HeapFree(hHeap, 0, (LPVOID)pDIB_RasterInfo);

	HeapFree(hHeap, 0, (LPVOID)pDIB_DIOGraphPixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_UserGraphPixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_MEALayoutPixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_HistoPixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_TracePixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_SinglePixels);
	HeapFree(hHeap, 0, (LPVOID)pDIB_RasterPixels);
}


/* -------------------- display rendering functions ----------------------- */

/* a callback function call periodically by a system timer
   obtains new data and renders it to all windows */

TIMERPROC
RenderTimerProc(HWND hWnd, INT nMsg, UINT nIDEvent, DWORD dwTime)
{
	/* do not handle stranded messages */

	if (DisplayTimerID == 0)
		return 0;

	if (FreezeDisplay)
		return 0;

	/* get new analog in data */

	UpdateTraceGraphData();

	/* render data to all windows */

	if (DisplayMode == DM_CONTINUOUS)
	{
		RenderTraceData();
		RenderTraceSingle();
	}

	if (DisplayMode == DM_LEVEL)
	{
		RenderSpikeEnvelopes();
		RenderSpikeEnvelopeSingle();
	}

	RenderSpikeRaster ();
	RenderRasterHistogram ();
	RenderDigitalIO ();
	RenderUserGraph();
	RenderMEALayout (TRUE);

	return 0;
}


/* get new A/D data and new spikes not seen during previous calls.
   This function sets the timestamp of the most recent data obtains, which is then used 
   by all rendering functions to draw data in sync */

void
UpdateTraceGraphData(void)
{
	int				i, j, k, delta_n;
	unsigned int	d;
	double			dx;
	__int64			firstentry, firstspiketime, l, currentspike;// , m, offset;
	unsigned short	electrode;
	float			*data, *env_data;
	__int64			*env_timestamp;

	/* if there is no data yet - do nothing */

	if (corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input < 1)
		return;

	/* get the latest voltage trace entries and times. 
	   Note that as data is stored in round robin fashion, wraparound is possible */

	LastVisitedVoltSample = (corefunc_global_var.Location_of_Last_Value_of_RAW_Analog_Input - 1 + corefunc_global_var.Samples_per_Channel) % corefunc_global_var.Samples_per_Channel;
	PrevLastVisitedVoltSampleTime = LastVisitedVoltSampleTime;

	/* this global value is used to synchronise the display of all data */

	LastVisitedVoltSampleTime = corefunc_global_var.Time_Stamp_Counter_Array[LastVisitedVoltSample];

	/* calculate the value of samples per trace point */

	dx = (double)(CurrentRecParms.TraceGraphTimeSlice * (double)CurrentRecParms.SampleRate) / (double)DATA_POINTS_PER_TRACE;

	/* get the number of volt trace points to update */

	delta_n = (int)((double)(LastVisitedVoltSampleTime - corefunc_global_var.Start_Sample_Time) / dx);

	if (delta_n > DATA_POINTS_PER_TRACE)
		delta_n = DATA_POINTS_PER_TRACE;

	for (j = 0; j < N_AD_CHANNELS; ++j) 
	{
		/*  store the new data */

		data = corefunc_global_var.RAW_Input_Volts + corefunc_global_var.Pointer_To_Electrode_In_RAW_Input_Volts[j];

		/* calculate the starting indices in the source and target arrays */

		k = DATA_POINTS_PER_TRACE - delta_n;
		l = (__int64)(delta_n * dx);
		firstentry = (LastVisitedVoltSample - l + corefunc_global_var.Samples_per_Channel) % corefunc_global_var.Samples_per_Channel;

		for (i = 0; i < delta_n; ++i)
		{
			l = (firstentry + (__int64)((double)i * dx)) % corefunc_global_var.Samples_per_Channel;
			TraceGraphDataArray[j].datapoint[k] = data[l];
			k++;
		}
	}

	/* ========  get new spikes ========== */

	if (corefunc_global_var.Spike_count == 0)
		return;
	
	/* get the last entry in the Time-Electrode table (for technical reasons, this array is 1 based, not 0 based) */

	currentspike = corefunc_global_var.Location_of_Last_Event_Analog_Spike - 1; 
	if (currentspike < 0)
		currentspike = corefunc_global_var.Size_Of_Events_In_Memory - 1;

	/* check if any spikes were added since last call. if not, return here */

	if (currentspike == LastVisitedSpike)
		return;

	/* get the last time stamp of the prior update. LastVisitedSpike is initialized to -1, 
	   so the function can tell that there was no prior visit */

	if (LastVisitedSpike < 0)
		firstspiketime = corefunc_global_var.Start_Sample_Time;
	else
		firstspiketime = corefunc_global_var.Analog_Spikes_Events[LastVisitedSpike].timestamp;

	/* keep tabs of how full the buffer is */

	SpikesInBuffer = corefunc_global_var.Spike_count;

	/* store the last spike entry for next time */

	LastVisitedSpike = currentspike;  
	
	/* move backward in new spike data until reaching the time stamp of the prior update 
	   (or Start_Sample_Time if this is the first update done) */

	while ((__int64) corefunc_global_var.Analog_Spikes_Events[currentspike].timestamp > firstspiketime)
	{
		/* get the electrode at which the spike was found */

		electrode = corefunc_global_var.Analog_Spikes_Events[currentspike].channel;

		/* a precaution */

		if (electrode >= N_AD_CHANNELS)
		{
			currentspike--;
			if (currentspike < 0)
				currentspike = corefunc_global_var.Size_Of_Events_In_Memory - 1;
			continue;
		}

		/* update the spike envelope counter up to a maximum of CurrentRecParms.SpikesToDisplay */ 

		TraceGraphDataArray[electrode].spike_env_count++;
		if (TraceGraphDataArray[electrode].spike_env_count > CurrentRecParms.SpikesToDisplay)
			TraceGraphDataArray[electrode].spike_env_count = CurrentRecParms.SpikesToDisplay;

		/* calculate the destination for the spikes envelope data */

		env_data = TraceGraphSpikeEnvMemBlock + (electrode * MAX_DISPLAY_SPIKES + TraceGraphDataArray[electrode].spike_env_last)
			* DISPLAY_SAMPLS_PER_SPIKE;

		/* copy the data*/

		for (d = 0; d <= (corefunc_global_var.Pre_Trigger_Samples + corefunc_global_var.Post_Trigger_Samples); ++d)
		{
			*env_data = *(corefunc_global_var.Analog_Spikes_Events[currentspike].value + d);
			env_data++;
		}

		/* calculate the destination for the envelope time stamp */

		env_timestamp = TraceGraphSpikeTimesMemBlock + (electrode * MAX_DISPLAY_SPIKES + TraceGraphDataArray[electrode].spike_env_last);

		/* copy the time stamp of this spike */

		*env_timestamp = corefunc_global_var.Analog_Spikes_Events[currentspike].timestamp;

		/* update the destination for the next spike */

		TraceGraphDataArray[electrode].spike_env_last = (TraceGraphDataArray[electrode].spike_env_last + 1) % CurrentRecParms.SpikesToDisplay;

		/* move to the prior spike in list */

		currentspike--;
		if (currentspike < 0)
			currentspike = corefunc_global_var.Size_Of_Events_In_Memory - 1;
	}
}

/* -------------------- Window rendering functions ----------------------- */


/* Render the voltage traces of all channels */

void
RenderTraceData(void)
{
	int		i, j;
	double	x, y, x1, y1, dx, y_scl;

	/* the graph is first rendered onto a bitmap which is then copied to display */

	/* wipe the bitmap */

	memset((PBYTE)pDIB_TracePixels, 255, TRACEGRAPH_X * TRACEGRAPH_Y * 3);

	/* calculate the scaling in x and y */

	dx = (double)TraceGraphWidth / (double)DATA_POINTS_PER_TRACE;
	y_scl = (double)(TraceGraphDataArray[1].y_position - TraceGraphDataArray[0].y_position) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		/* render the data bringing into account that the data is stored in round robin fashion */

		x = x1 = 0;
		y1 = TraceGraphDataArray[i].datapoint[0] * -y_scl + TraceGraphDataArray[i].y_position;

		for (j = 0; j < DATA_POINTS_PER_TRACE; ++j)
		{
			x += dx;
			y = (double)TraceGraphDataArray[i].datapoint[j] * -y_scl + (double)TraceGraphDataArray[i].y_position;
			DrawLineBresenham((int)x1, (int)y1, (int)x, (int)y, pDIB_TraceInfo, pDIB_TracePixels, TraceGraphDataArray[i].pencolor);
			x1 = x;
			y1 = y;
		}
	}

	/* bitblt the bitmap to the display */

	StretchDIBits(TraceGraphWindowDC, 0, 0, TRACEGRAPH_X, TRACEGRAPH_Y,
		0, 0, TRACEGRAPH_X, TRACEGRAPH_Y,
		pDIB_TracePixels, pDIB_TraceInfo, DIB_RGB_COLORS, SRCCOPY);
}


/* Render a single voltage trace in the enlarged view of the channel */

void
RenderTraceSingle(void)
{
	int			j, k, y_origin;
	double		x, y, x1, y1, spk_x, y_scl, dx, th;
	__int64		windowfirsttimestamp, timestamp;
	HFONT		oldfont;
	char		str[100];

	/* the graph is first rendered onto a bitmap which is then copied to display */

	/* wipe the bitmap */

	memset((PBYTE)pDIB_SinglePixels, 255, TRACESINGLE_X * TRACESINGLE_Y * 3);

	/* calculate the scaling in x and y */

	dx = (double)TraceSingleWidth / (double)DATA_POINTS_PER_TRACE;
	y_origin = TraceSingleHeight / 2;
	y_scl = (double)(TraceSingleHeight - 40) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	/* draw a line at 0V */

	DrawLineBresenham(0, y_origin, TraceSingleWidth - 1, y_origin, pDIB_SingleInfo, pDIB_SinglePixels, 8);

	/* draw a few scale bar lines */

	for (j = -2; j <= 2; ++j)
	{
		y = y_origin + j * ((TraceSingleHeight - 40) / 4);
		DrawLineBresenham(0, (int) y, 10, (int) y, pDIB_SingleInfo, pDIB_SinglePixels, 8);
	}

	/* render the data bringing into accound that the data is stored in round robin fashion */

	x = x1 = 0;
	y1 = (double)TraceGraphDataArray[SelectedTraceNum].datapoint[0] * -y_scl + y_origin;

	for (j = 0; j < DATA_POINTS_PER_TRACE; ++j)
	{
		x += dx;
		y = (double)TraceGraphDataArray[SelectedTraceNum].datapoint[j] * -y_scl + y_origin;
		DrawLineBresenham((int)x1, (int)y1, (int)x, (int)y, pDIB_SingleInfo, pDIB_SinglePixels,
			TraceGraphDataArray[SelectedTraceNum].pencolor);
		x1 = x;
		y1 = y;
	}

	/* render detected spikes on top of the trace:
	first, get the earliest time stamp that fits into the windows time slice */

	windowfirsttimestamp = LastVisitedVoltSampleTime - (__int64)(CurrentRecParms.TraceGraphTimeSlice * CurrentRecParms.SampleRate);

	/* calculate the x axis value of each time sample */

	dx = (double)TraceSingleWidth / (double)(CurrentRecParms.TraceGraphTimeSlice * CurrentRecParms.SampleRate);

	/* get the last spike envelope index stored for this electrode */

	k = TraceGraphDataArray[SelectedTraceNum].spike_env_last - 1;
	if (k < 0)
		k = CurrentRecParms.SpikesToDisplay - 1;

	for (j = 0; j < TraceGraphDataArray[SelectedTraceNum].spike_env_count; ++j)
	{
		timestamp = *(TraceGraphSpikeTimesMemBlock + SelectedTraceNum * MAX_DISPLAY_SPIKES + k);
		if (timestamp > windowfirsttimestamp)
		{
			/* render the spike symbol  */

			spk_x = dx * (double)(timestamp - windowfirsttimestamp);
			DrawLineBresenham((int)spk_x, (int)1, (int)spk_x, (int)10, pDIB_SingleInfo, pDIB_SinglePixels, 8);
		}
		else
			break;

		k--;
		if (k < 0)
			k = CurrentRecParms.SpikesToDisplay - 1;
	}

	/* draw a line at the threshold */

	th = corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum] * y_scl;
	DrawLineBresenham(0, (int) (y_origin - th), TraceSingleWidth - 1, (int) (y_origin - th), pDIB_SingleInfo, pDIB_SinglePixels, 10);

	/* bitblt the bitmap to the display */

	StretchDIBits(TraceSingleWindowDC, 0, 0, TRACESINGLE_X, TRACESINGLE_Y,
		0, 0, TRACESINGLE_X, TRACESINGLE_Y,
		pDIB_SinglePixels, pDIB_SingleInfo, DIB_RGB_COLORS, SRCCOPY);

	/* Annotate the display */

	oldfont = SelectObject(TraceSingleWindowDC, GetStockObject(ANSI_VAR_FONT));
	SetBkMode(TraceSingleWindowDC, TRANSPARENT);

	/* print out the channel details */

	j = sprintf_s(str, 100, "AD Channel %d (%s)", ElectrodesInfo[SelectedTraceNum].channel, ElectrodesInfo[SelectedTraceNum].name);
	TextOut(TraceSingleWindowDC, TraceSingleWidth - 150, 2, str, j + 1);

	/* print the Y scale info */

	j = sprintf_s(str, 100, "%.2f mV", CurrentRecParms.DisplayVoltageFullScale / 2);
	TextOut(TraceSingleWindowDC, 14, y_origin - (TraceSingleHeight - 40) / 2 - 5, str, j + 1);
	j = sprintf_s(str, 100, "%.2f mV", -CurrentRecParms.DisplayVoltageFullScale / 2);
	TextOut(TraceSingleWindowDC, 14, y_origin + (TraceSingleHeight - 40) / 2 - 8, str, j + 1);

	/* print the threshold info */

	j = sprintf_s(str, 100, "%.2f mV", corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum]);
	TextOut(TraceSingleWindowDC, 14, (int) (y_origin - th), str, j + 1);

	SelectObject(TraceSingleWindowDC, oldfont);

}


/* render the spike envelope for all channels */

void
RenderSpikeEnvelopes(void)
{
	int				i, k, j, env_points;
	int				max_y, min_y, range;
	double			x, y, x1, y1, dx, y_scl;
	float			*env_data;
	
	/* the graph is first rendered onto a bitmap which is then copied to display */

	memset((PBYTE)pDIB_TracePixels, 255, TRACEGRAPH_X * TRACEGRAPH_Y * 3);

	/* calculate the maximal vertical range of all traces */

	range = (int)((double)TRACEGRAPH_Y / (double)(ROWS_LEVEL + 1));

	/* get the number of data points in each trace */

	env_points = corefunc_global_var.Pre_Trigger_Samples + corefunc_global_var.Post_Trigger_Samples;

	dx = ((double)TraceGraphWidth / (double)COLS_LEVEL) / (double)(env_points + 10);
	y_scl = (double)(TraceGraphDataArray[COLS_LEVEL].env_y - TraceGraphDataArray[0].env_y) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	/* loop on all channels */

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		max_y = TraceGraphDataArray[i].env_y + range / 2;
		min_y = max_y - range;

		/* loop on stored traces of each channel */

		for (k = 0; k < TraceGraphDataArray[i].spike_env_count; ++k)
		{
			/* get pointer to spike envelope data */

			env_data = TraceGraphSpikeEnvMemBlock + (i * MAX_DISPLAY_SPIKES + k)* DISPLAY_SAMPLS_PER_SPIKE;

			/* calculate the coordinates of the first point in the trace */

			x = x1 = TraceGraphDataArray[i].env_x;
			y1 = *env_data * -y_scl + TraceGraphDataArray[i].env_y;
			if (y1 > max_y)
				y1 = max_y;
			if (y1 < min_y)
				y1 = min_y;

			/* draw all points. truncate in vertical dimension to prevent overflows */

			for (j = 0; j < env_points; ++j)
			{
				x += dx;
				env_data++;
				y = *env_data * -y_scl + (double)TraceGraphDataArray[i].env_y;
				if (y > max_y)
					y = max_y;
				if (y < min_y)
					y = min_y;
				DrawLineBresenham((int)x1, (int)y1, (int)x, (int)y, pDIB_TraceInfo, pDIB_TracePixels, TraceGraphDataArray[i].pencolor);
				x1 = x;
				y1 = y;
			}
		}
	}

	/* bitblt the bitmap to the display */

	StretchDIBits(TraceGraphWindowDC, 0, 0, TRACEGRAPH_X, TRACEGRAPH_Y,
		0, 0, TRACEGRAPH_X, TRACEGRAPH_Y,
		pDIB_TracePixels, pDIB_TraceInfo, DIB_RGB_COLORS, SRCCOPY);
}


/* render the magnified spike trace envelopes of the selected channel  */

void
RenderSpikeEnvelopeSingle(void)
{
	int				k, j, env_points, y_origin;
	double			x, y, x1, y1, dx, y_scl, th;
	float			*env_data;
	HFONT			oldfont;
	char			str[100];

	/* the graph is first rendered onto a bitmap which is then copied to display */

	memset((PBYTE)pDIB_SinglePixels, 255, TRACESINGLE_X * TRACESINGLE_Y * 3);

	/* get the number of data points in each trace */

	env_points = corefunc_global_var.Pre_Trigger_Samples + corefunc_global_var.Post_Trigger_Samples;

	dx = (double)TraceSingleWidth / (double)(env_points);
	y_origin = TraceSingleHeight / 2;
	y_scl = (double)(TraceSingleHeight - 40) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	/* draw a line at 0V and at the trigger time point */

	DrawLineBresenham(0, y_origin, TraceSingleWidth - 1, y_origin, pDIB_SingleInfo, pDIB_SinglePixels, 8);
	DrawLineBresenham((int)(dx * corefunc_global_var.Pre_Trigger_Samples), 0,
		(int)(dx * corefunc_global_var.Pre_Trigger_Samples), TraceSingleHeight - 1, pDIB_SingleInfo, pDIB_SinglePixels, 8);

	for (j = -2; j <= 2; ++j)
	{
		y = y_origin + j * ((TraceSingleHeight - 40) / 4);
		DrawLineBresenham(0, (int) y, 10, (int) y, pDIB_SingleInfo, pDIB_SinglePixels, 8);
	}

	for (k = 0; k < TraceGraphDataArray[SelectedTraceNum].spike_env_count; ++k)
	{
		/* get pointer to spike envelope data */

		env_data = TraceGraphSpikeEnvMemBlock + (SelectedTraceNum * MAX_DISPLAY_SPIKES + k)* DISPLAY_SAMPLS_PER_SPIKE;

		x = x1 = 0;
		y1 = *env_data * -y_scl + y_origin;

		for (j = 0; j < env_points; ++j)
		{
			x += dx;
			env_data++;
			y = *env_data * -y_scl + (double)y_origin;
			DrawLineBresenham((int)x1, (int)y1, (int)x, (int)y, pDIB_SingleInfo, pDIB_SinglePixels,
				TraceGraphDataArray[SelectedTraceNum].pencolor);
			x1 = x;
			y1 = y;
		}
	}

	/* draw a line at the threshold */

	th = corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum] * y_scl;
	DrawLineBresenham(0, (int) (y_origin - th), TraceSingleWidth - 1, (int) (y_origin - th), pDIB_SingleInfo, pDIB_SinglePixels, 10);

	/* bitblt the bitmap to the display */

	StretchDIBits(TraceSingleWindowDC, 0, 0, TRACESINGLE_X, TRACESINGLE_Y,
		0, 0, TRACESINGLE_X, TRACESINGLE_Y,
		pDIB_SinglePixels, pDIB_SingleInfo, DIB_RGB_COLORS, SRCCOPY);

	/* Annotate the display */

	oldfont = SelectObject(TraceSingleWindowDC, GetStockObject(ANSI_VAR_FONT));
	SetBkMode(TraceSingleWindowDC, TRANSPARENT);

	/* print out the channel details */

	j = sprintf_s(str, 100, "AD Channel %d (%s)", ElectrodesInfo[SelectedTraceNum].channel, ElectrodesInfo[SelectedTraceNum].name);
	TextOut(TraceSingleWindowDC, TraceSingleWidth - 150, 2, str, j + 1);

	/* print the Y scale info */

	j = sprintf_s(str, 100, "%.2f mV", CurrentRecParms.DisplayVoltageFullScale / 2);
	TextOut(TraceSingleWindowDC, 14, y_origin - (TraceSingleHeight - 40) / 2 - 5, str, j + 1);
	j = sprintf_s(str, 100, "%.2f mV", -CurrentRecParms.DisplayVoltageFullScale / 2);
	TextOut(TraceSingleWindowDC, 14, y_origin + (TraceSingleHeight - 40) / 2 - 8, str, j + 1);

	/* print the threhold info */

	j = sprintf_s(str, 100, "%.2f mV", corefunc_global_var.threshold_Per_Electrod[SelectedTraceNum]);
	TextOut(TraceSingleWindowDC, 14, (int) (y_origin - th), str, j + 1);

	SelectObject(TraceSingleWindowDC, oldfont);
}


/* render the spike raster - a dot for each spike of each channel */

void
RenderSpikeRaster(void)
{
	__int64					first_spk_to_display;
	__int64					i, j, n;
	__int64					firsttimestamp, windowfirsttimestamp, rasterbin, t, t_offset;
	double					dy, dx;
	int						x, bin, ypos[N_AD_CHANNELS];
	unsigned short			electrode;
	PBYTE					buf_p;
	
	/* no spikes detected yet - return here */

	if (corefunc_global_var.Location_of_Last_Event_Analog_Spike< 1)
		return;

	if (LastVisitedSpike < 0)
		return;

	/* the graph is first rendered onto a bitmap which is then copied to display */

	if (!FreezeDisplay)		/* If the display is frozen, do not re-render the bitmap */
	{
		/* get the earliest time stamp that fits into the windows time slice */

		windowfirsttimestamp = LastVisitedVoltSampleTime - (__int64)(CurrentRecParms.SpikeRasterTimeSlice * CurrentRecParms.SampleRate);

		firsttimestamp = windowfirsttimestamp;
		if (firsttimestamp < (__int64)corefunc_global_var.Start_Sample_Time)
			firsttimestamp = corefunc_global_var.Start_Sample_Time;

		/* a variable used to align the histograms to incoming data */

		if (FirstRasterVoltSampleTime < 0)
			FirstRasterVoltSampleTime = LastVisitedVoltSampleTime;

		/* move back along the time/electrode data to find the index of this earliest spike event */

		i = LastVisitedSpike;
		n = 0;

		while (((__int64)corefunc_global_var.Analog_Spikes_Events[i].timestamp > firsttimestamp) && (n <= SpikesInBuffer))
		{
			--i;
			n++;
			if (i < 0)
			{
				/* wrap arround to end of buffer only if it is full */

				if (SpikesInBuffer == GlobalRecParms.SpikeHistoryLength)
					i = (__int64)corefunc_global_var.Size_Of_Events_In_Memory - 1;
				else
					break;
			}
		}

		/* store the index of the first spike to display and the number of spikes to display */

		first_spk_to_display = i + 1; 
		if (first_spk_to_display >= (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
			first_spk_to_display = 0;

		/* calculate the width, in pixels, of one sample time */

		dx = ((double)SpikeRasterWidth / CurrentRecParms.SpikeRasterTimeSlice) / (double) CurrentRecParms.SampleRate;

		/* precalculate the Y coordinate for all electrodes */

		dy = (double)SpikeRasterHeight / (double)(N_AD_CHANNELS + 1);
		for (i = 0; i < N_AD_CHANNELS; ++i)
			ypos[i] = (int)((double)(i + 1) * dy);

		/* calculate the bin size and a running offset to align the histograms to incoming data */

		rasterbin = (LastVisitedVoltSampleTime - windowfirsttimestamp) / RasterUnits;
		t_offset = (LastVisitedVoltSampleTime - FirstRasterVoltSampleTime) % rasterbin;
		HistogramOffset = (int) ((double) t_offset * dx);

		for (i = 0; i < RasterUnits; ++i)
			HistogramData[i] = 0;
		
		/* erase the background */

		memset((PBYTE)pDIB_RasterPixels, 255, SPIKERASTER_X * SPIKERASTER_Y * 3);

		/* plot the spikes */

		i = first_spk_to_display;

		for (j = 0; j < n; ++j)
		{
			electrode = corefunc_global_var.Analog_Spikes_Events[i].channel;
			
			/* calculate offset from spike rasters windows left side and the histogram bin */

			t = corefunc_global_var.Analog_Spikes_Events[i].timestamp - windowfirsttimestamp;
			bin = (int)((t + t_offset) / rasterbin);

			if ((electrode < N_AD_CHANNELS) && (ElectrodesInfo[electrode].inuse) && (bin >= 0) && (bin < RasterUnits))
			{
				/* update the histograms data */

				HistogramData[bin]++;

				/* render a dot for the spike */

				x = (int)((double)t * dx);
				if ((x < SPIKERASTER_X) && (ypos[electrode] < SPIKERASTER_Y))
				{
					buf_p = (PBYTE)pDIB_RasterPixels +
						((SPIKERASTER_Y - ypos[electrode] - 1) * SPIKERASTER_X + x) * 3;
					*buf_p = 0;
					*(buf_p + 1) = 0;
					*(buf_p + 2) = 0;
				}
			}

			i++;
			if (i == (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
				i = 0;
		}
	}

	/* bitblt the bitmap to the display */

	StretchDIBits(SpikeRasterWindowDC, 0, 0, SPIKERASTER_X, SPIKERASTER_Y,
		0, 0, SPIKERASTER_X, SPIKERASTER_Y,
		pDIB_RasterPixels, pDIB_RasterInfo, DIB_RGB_COLORS, SRCCOPY);

}


/* render the spikes histogram
   note that the actual data is calculated in RenderSpikeRaster() 
   so it is important to call these functions in the right order */

void
RenderRasterHistogram(void)
{
	double	    dy, dx;
	int		    i, maxval;
	int			x, y;
	RECT	    rect;
	BOOL		saturation;
	PBYTE		p;
	BYTE		r, g, b;

	dx = (double)SpikeHistogramWidth / (double)RasterUnits;

	maxval = CurrentRecParms.SpikeHistoMaxVal;
	dy = (double)SpikeHistogramHeight / (double)maxval;

	/* erase the background */

	memset((PBYTE)pDIB_HistoPixels, 255, SPIKEHISTOGRAM_X * SPIKEHISTOGRAM_Y * 3);

	/* calculate the boudaries of each histogram bar */

	for (i = 0; i < RasterUnits; ++i)
	{
		rect.left = max (0, (int)((double)i * dx) - HistogramOffset);
		if (HistogramData[i] <= maxval)
		{
			rect.top = SpikeHistogramHeight - (int)(HistogramData[i] * dy);
			saturation = FALSE;
		}
		else
		{
			rect.top = SpikeHistogramHeight - (int)(maxval * dy);
			saturation = TRUE;
		}
		rect.right = min(rect.left + (int)dx - 1, SpikeHistogramWidth);
		rect.bottom = SpikeHistogramHeight;

		/* set bar colors */

		if (saturation)
		{
			r = (BYTE)192;
			g = (BYTE)24;
			b = (BYTE)55;
		}
		else
		{
			r = (BYTE)55;
			g = (BYTE)96;
			b = (BYTE)146;
		}

		/* render the bar in the bitmap buffer */

		for (y = rect.top; y < rect.bottom; ++y)
			for (x = rect.left; x < rect.right; ++x)
			{
				p = (PBYTE)pDIB_HistoPixels + ((SPIKEHISTOGRAM_X * (SpikeHistogramHeight - y + 1) + x) * 3);
				*p = b;
				*(p + 1) = g;
				*(p + 2) = r;
			}
	}

	/* bitblt the bitmap to the display */

	StretchDIBits(SpikeHistogramWindowDC, 0, 0, SPIKEHISTOGRAM_X, SPIKEHISTOGRAM_Y,
		0, 0, SPIKEHISTOGRAM_X, SPIKEHISTOGRAM_Y,
		pDIB_HistoPixels, pDIB_HistoInfo, DIB_RGB_COLORS, SRCCOPY);
}

	
/* render the Digital Input/Output data */

void
RenderDigitalIO(void)
{
	__int64					first_event_to_display;
	__int64					i, j, n;
	__int64					firsttimestamp, windowfirsttimestamp, t;
	double					dx;
	int						DI_x1, DO_x1, DI_y1, DO_y1;
	int						x, y, y_low, y_high;
	char					str[100];
	HFONT					oldfont;

	/* the graph is first rendered onto a bitmap which is then copied to display */
	
	if (LastVisitedVoltSample < 0 && !FreezeDisplay)		/* no data aquired yet: prepare a blank graph */
	{
		/* erase the background */

		memset((PBYTE)pDIB_DIOGraphPixels, 255, DIOGRAPH_X * DIOGRAPH_Y * 3);

		DrawLineBresenham(0, DIOGraphHeight / 2, DIOGraphWidth, DIOGraphHeight / 2, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 10);
	}

	if (LastVisitedVoltSample >= 0 && !FreezeDisplay)		/* re-render the bitmap unless the display is frozen */
	{
		/* get the earliest time stamp that fits into the windows time slice */

		windowfirsttimestamp = LastVisitedVoltSampleTime - (__int64)(CurrentRecParms.SpikeRasterTimeSlice * CurrentRecParms.SampleRate);

		firsttimestamp = windowfirsttimestamp;
		if (firsttimestamp < (__int64)corefunc_global_var.Start_Sample_Time) 
			firsttimestamp = corefunc_global_var.Start_Sample_Time;
		
		/* calculate the width, in pixels, of one sample time */

		dx = ((double)DIOGraphWidth / CurrentRecParms.SpikeRasterTimeSlice) / (double) CurrentRecParms.SampleRate;

		/* erase the background */

		memset((PBYTE)pDIB_DIOGraphPixels, 255, DIOGRAPH_X * DIOGRAPH_Y * 3);

		DrawLineBresenham(0, DIOGraphHeight / 2, DIOGraphWidth, DIOGraphHeight / 2, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 10);

		/*  =============== Render Digital Input ============== */

		/* calculate the initial horizontal position of the plot */

		t = firsttimestamp - windowfirsttimestamp;
		DI_x1 = (int)((double)t * dx);

		/* precalculate the Y coordinates  */

		y_high = DIOGraphHeight / 4 - 8;
		y_low = DIOGraphHeight / 4 + 8;

		/* move back along the digital IO events to find the index of this earliest  event 
		   Note: the first event should always exist !*/

		i = corefunc_global_var.Location_of_Last_Event_Digital_I;

		n = 0;
		while ((__int64) corefunc_global_var.Digital_I_Events[i].timestamp > firsttimestamp)
		{
			--i;
			++n;
			if (i < 0)
				i = corefunc_global_var.Size_Of_Events_In_Memory - 1;
		}

		/* at this point i is pointing to an event that has moved beyond the left margin
			(or does not exist yet).use it to determine the level for the leftmost horizontal line */

		if ((__int64)corefunc_global_var.Digital_I_Events[i].timestamp > 0)
		{
			if (corefunc_global_var.Digital_I_Events[i].value & DigInputDisplayMask)
				DI_y1 = y_high;
			else
				DI_y1 = y_low;
		}
		else
			DI_y1 = y_low;

		/* store the index of the first event within the time window to display */

		first_event_to_display = i + 1;
		if (first_event_to_display >= (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
			first_event_to_display = 0;
		
		i = first_event_to_display;

		for (j = 0; j < n; ++j)
		{
			/* calculate offset from spike rasters windows left side */

			t = corefunc_global_var.Digital_I_Events[i].timestamp - windowfirsttimestamp;
			x = (int)((double)t * dx);
			if (corefunc_global_var.Digital_I_Events[i].value & DigInputDisplayMask)
				y = y_high;
			else
				y = y_low;

			/* draw a horizontal and then a vertical line */

			DrawLineBresenham(DI_x1, DI_y1, x, DI_y1, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 5);
			DrawLineBresenham(x, DI_y1, x, y, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 5);

			i++;
			if (i == (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
				i = 0;

			DI_x1 = x;
			DI_y1 = y;
		}

		DrawLineBresenham(max(DI_x1, 0), DI_y1, DIOGraphWidth - 1, DI_y1, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 5);

		/*  =============== Render Digital Output ============== */

		/* calculate the initial horizontal position of the plot */

		t = firsttimestamp - windowfirsttimestamp;
		DO_x1 = (int)((double)t * dx);

		/* precalculate the Y coordinates  */

		y_high = (3 * DIOGraphHeight) / 4 - 8;
		y_low = (3 * DIOGraphHeight) / 4 + 8;
		
		/* move back along the digital IO events to find the index of this earliest  event */

		i = corefunc_global_var.Location_of_Last_Event_Digital_O;

		n = 0;
		while ((__int64)corefunc_global_var.Digital_O_Events[i].timestamp > firsttimestamp)
		{
			--i;
			++n;
			if (i < 0)
				i = corefunc_global_var.Size_Of_Events_In_Memory - 1;
		}

		/* at this point i is pointing to an event that has moved beyond, or is exactly on the left margin.
		   use it to determine the level for the leftmost horizontal line */

		if (corefunc_global_var.Digital_O_Events[i].value & DigOutputDisplayMask)
			DO_y1 = y_high;
		else
			DO_y1 = y_low;
		
		/* store the index of the first event within the time window to display */

		if ((__int64)corefunc_global_var.Digital_O_Events[i].timestamp < firsttimestamp)
		{
			first_event_to_display = i + 1;
			if (first_event_to_display >= (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
				first_event_to_display = 0;
		}

		/* plot the DO profile */

		i = first_event_to_display;

		for (j = 0; j < n; ++j)
		{
			/* calculate offset from spike rasters windows left side */

			t = corefunc_global_var.Digital_O_Events[i].timestamp - windowfirsttimestamp;
			x = (int)((double)t * dx);
			if (corefunc_global_var.Digital_O_Events[i].value & DigOutputDisplayMask)
				y = y_high;
			else
				y = y_low;

			/* draw a horizontal and then a vertical line */

			DrawLineBresenham(DO_x1, DO_y1, x, DO_y1, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 0);
			DrawLineBresenham(x, DO_y1, x, y, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 0);

			i++;
			if (i == (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
				i = 0;

			DO_x1 = x;
			DO_y1 = y;
		}

		DrawLineBresenham(max(DO_x1, 0), DO_y1, DIOGraphWidth - 1, DO_y1, pDIB_DIOGraphInfo, pDIB_DIOGraphPixels, 0);

	}
	
	/* bitblt the bitmap to the display */

	StretchDIBits (DIOGraphWindowDC, 0, 0, DIOGRAPH_X, DIOGRAPH_Y,
		0, 0, DIOGRAPH_X, DIOGRAPH_Y,
		pDIB_DIOGraphPixels, pDIB_DIOGraphInfo, DIB_RGB_COLORS, SRCCOPY);

	/* label the two graphs */

	oldfont = SelectObject(DIOGraphWindowDC, GetStockObject(ANSI_VAR_FONT));
	SetBkMode(UserGraphWindowDC, TRANSPARENT);

	j = 0;
	while (DigInputDisplayMask >> (j + 1))
		j++;
	x = sprintf_s(str, 100, "Digital Input, Line %lld", j + 1);
	TextOut(DIOGraphWindowDC, 2, 1, str, x);

	j = 0;
	while (DigOutputDisplayMask >> (j + 1))
		j++;
	x = sprintf_s(str, 100, "Digital Output, Line %lld", j + 1);
	TextOut(DIOGraphWindowDC, 2, DIOGRAPH_Y / 2 + 1, str, x);

	SelectObject(DIOGraphWindowDC, oldfont);
}


/* render the user data graphs */

void
RenderUserGraph(void)
{
	int						x1,	y1[2];
	__int64					first_event_to_display;
	__int64					i, j, n;
	__int64					firsttimestamp, windowfirsttimestamp, t;
	double					dx,dy;
	int						x, y[2], y_baseline[2];
	char					str[100];
	HFONT					oldfont;

	/* the graph is first rendered onto a bitmap which is then copied to display */

	if (LastVisitedVoltSample < 0 && !FreezeDisplay)		/* no data aquired yet: prepare a blank graph */
	{
		/* erase the background */

		memset((PBYTE)pDIB_UserGraphPixels, 255, USERGRAPH_X * USERGRAPH_Y * 3);

		DrawLineBresenham(0, UserGraphHeight / 2, UserGraphWidth, UserGraphHeight / 2, pDIB_UserGraphInfo, pDIB_UserGraphPixels, 10);
	}

	if (LastVisitedVoltSample >= 0 && !FreezeDisplay)		/* re-render the bitmap unless the display is frozen */
	{
		/* get the earliest time stamp that fits into the windows time slice */

		windowfirsttimestamp = LastVisitedVoltSampleTime - (__int64)(CurrentRecParms.SpikeRasterTimeSlice * CurrentRecParms.SampleRate);

		firsttimestamp = windowfirsttimestamp;
		if (firsttimestamp < (__int64)corefunc_global_var.Start_Sample_Time)
			firsttimestamp = corefunc_global_var.Start_Sample_Time;

		/* calculate the width, in pixels, of one sample time */

		dx = ((double)UserGraphWidth / CurrentRecParms.SpikeRasterTimeSlice) / (double) CurrentRecParms.SampleRate;

		/* precalculate the Y base values and units  */

		y_baseline[0] = UserGraphHeight / 4;
		y_baseline[1] = 3 * UserGraphHeight / 4;
		dy = -(double)(UserGraphHeight / 4);

		/* erase the background */

		memset((PBYTE)pDIB_UserGraphPixels, 255, USERGRAPH_X * USERGRAPH_Y * 3);

		DrawLineBresenham(0, UserGraphHeight / 2, UserGraphWidth, UserGraphHeight / 2, pDIB_UserGraphInfo, pDIB_UserGraphPixels, 10);

		/* calculate the initial horizontal position of the plots */

		t = firsttimestamp - windowfirsttimestamp;
		x1 = (int)((double)t * dx);

		/* move back along the User Analog events to find the index of this earliest event
		Note: the first event should always exist !*/

		i = corefunc_global_var.Location_of_Last_Event_User_Analog;

		n = 0;
		while ((__int64)corefunc_global_var.User_Analog_Events[i].timestamp > firsttimestamp)
		{
			--i;
			++n;

			if (i < 0)
				i = corefunc_global_var.Size_Of_Events_In_Memory - 1;
		}

		/* at this point i is pointing to an event that has moved beyond the left margin
		   (or does not exist yet).use it to determine the level for the leftmost horizontal line */

		if ((__int64)corefunc_global_var.User_Analog_Events[i].timestamp > 0)
		{
			y1[0] = (int)(corefunc_global_var.User_Analog_Events[i].value[0] * dy) + y_baseline[0];
			y1[1] = (int)(corefunc_global_var.User_Analog_Events[i].value[1] * dy) + y_baseline[1];
		}
		else
		{ 
			y1[0] = y_baseline[0];
			y1[1] = y_baseline[1];
		}

		/* store the index of the first event within the time window to display */

		first_event_to_display = i + 1;
		if (first_event_to_display >= (__int64) corefunc_global_var.Size_Of_Events_In_Memory)
			first_event_to_display = 0;

		/* plot the User data graphs */

		i = first_event_to_display;	

		for (j = 0; j < n; ++j)
		{
			/* calculate offset from spike rasters windows left side */

			t = corefunc_global_var.User_Analog_Events[i].timestamp - windowfirsttimestamp;
			x = (int)((double)t * dx);
			y[0] = (int)(corefunc_global_var.User_Analog_Events[i].value[0] * dy) + y_baseline[0];
			y[1] = (int)(corefunc_global_var.User_Analog_Events[i].value[1] * dy) + y_baseline[1];

			/* draw horizontal and then a vertical lines */

			DrawLineBresenham(x1, y1[0], x, y1[0], pDIB_UserGraphInfo, pDIB_UserGraphPixels, 1);
			DrawLineBresenham(x, y1[0], x, y[0], pDIB_UserGraphInfo, pDIB_UserGraphPixels,1);

			DrawLineBresenham(x1, y1[1], x, y1[1], pDIB_UserGraphInfo, pDIB_UserGraphPixels, 3);
			DrawLineBresenham(x, y1[1], x, y[1], pDIB_UserGraphInfo, pDIB_UserGraphPixels, 3);

			i++;
			if (i == (__int64)corefunc_global_var.Size_Of_Events_In_Memory)
				i = 0;
			
			x1 = x;
			y1[0] = y[0];
			y1[1] = y[1];
		}

		DrawLineBresenham(max(x1, 0), y1[0], UserGraphWidth - 1, y1[0], pDIB_UserGraphInfo, pDIB_UserGraphPixels, 1);
		DrawLineBresenham(max(x1, 0), y1[1], UserGraphWidth - 1, y1[1], pDIB_UserGraphInfo, pDIB_UserGraphPixels, 3);
	}

	/* bitblt the bitmap to the display */

	StretchDIBits(UserGraphWindowDC, 0, 0, USERGRAPH_X, USERGRAPH_Y,
		0, 0, USERGRAPH_X, USERGRAPH_Y,
		pDIB_UserGraphPixels, pDIB_UserGraphInfo, DIB_RGB_COLORS, SRCCOPY);

	/* label the two graphs */

	oldfont = SelectObject(UserGraphWindowDC, GetStockObject(ANSI_VAR_FONT));
	SetBkMode(UserGraphWindowDC, TRANSPARENT);

	x = sprintf_s(str, 100, "%s", "User Channel 1");
	TextOut(UserGraphWindowDC, 2, 1, str, x);

	x = sprintf_s(str, 100, "%s", "User Channel 2");
	TextOut(UserGraphWindowDC, 2, USERGRAPH_Y / 2 + 1, str, x);

	SelectObject(UserGraphWindowDC, oldfont);
}


/* render the electrode layout */

void
RenderMEALayout(BOOL fullupdate)
{
	__int64				firsttimestamp, bin_samples, n, j;
	unsigned short		electrode;
	double				dy, dx, dc;
	int					x, y, i, u, v, maxval, color;
	int					min_x, min_y, max_x, max_y, offset;
	HFONT				oldfont;
	PBYTE				buf_p;
	char				str[100];

	if (!FreezeDisplay)		/* If the display is frozen, do not generate a new histogram */
	{
		for (i = 0; i < N_AD_CHANNELS ; ++i)
			LayoutHistogram[i] = 0;

		/* get the number of events for each electrode in the last time bin */

		if (corefunc_global_var.Location_of_Last_Event_Analog_Spike>= 1)
		{
			if (LayoutMode == LM_LASTBIN)
			{
				/* if displaying spikes for the last bin only, get the earliest time stamp that fits into the last time bin */

				bin_samples = (__int64)((CurrentRecParms.SpikeRasterTimeSlice / (double)RasterUnits) * CurrentRecParms.SampleRate);

				firsttimestamp = LastVisitedVoltSampleTime - bin_samples;
			}
			else		/* i.e. LayoutMode == LM_ALLBINS */
			{
				/* if displaying spikes for the entire spike histogram, get the earliest time stamp that fits into the histograms time slice */

				firsttimestamp = LastVisitedVoltSampleTime - (__int64)(CurrentRecParms.SpikeRasterTimeSlice * CurrentRecParms.SampleRate);
			}

			if (firsttimestamp < (__int64)corefunc_global_var.Start_Sample_Time)
				firsttimestamp = corefunc_global_var.Start_Sample_Time;

			/* move back along the time/electrode data and generate a histogram according to electrode number */

			j = LastVisitedSpike;

			n = 0;
			while (((__int64) corefunc_global_var.Analog_Spikes_Events[j].timestamp > firsttimestamp) && (j > 0) && n <= SpikesInBuffer)
			{			
				electrode = corefunc_global_var.Analog_Spikes_Events[j].channel;
				if ((electrode < N_AD_CHANNELS) && (ElectrodesInfo[electrode].inuse))
					LayoutHistogram[electrode]++;
				--j;
				n++;
				if (j < 0)
				{
					/* wrap arround to end of buffer only if it is full */

					if (SpikesInBuffer == GlobalRecParms.SpikeHistoryLength)
						j = (__int64)corefunc_global_var.Size_Of_Events_In_Memory - 1;
					else
						break;
				}
			}
		}						/* end "corefunc_global_var.Head_Pointer_Spikes >= 1" */
	}

	/* erase the background */
	
	memset((PBYTE)pDIB_MEALayoutPixels, 255, MEALAYOUT_X * MEALAYOUT_Y * 3);

	/* draw a scalebar*/

	x = (MEALAYOUT_X - 128) / 2;
	y = MEALAYOUT_Y - 30;

	for (i = 0; i < 128; ++i)
	{
		for (u = 0; u < 10; ++u)
		{
			buf_p = (PBYTE)pDIB_MEALayoutPixels + ((MEALAYOUT_Y - y - u - 1) * MEALAYOUT_X + x + i) * 3;
			*buf_p = spectrumLUT[2 * i].rgbBlue;
			*(buf_p + 1) = spectrumLUT[2 * i].rgbGreen;
			*(buf_p + 2) = spectrumLUT[2 * i].rgbRed;
		}
	}

	/* calculate the electrode layout boundaries */

	maxval = 1;
	min_x = min_y = 100000;
	max_x = max_y = -1;

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (LayoutHistogram[i] > maxval)
			maxval = LayoutHistogram[i];
	}

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		if (ElectrodesInfo[i].xpos < min_x)
			min_x = ElectrodesInfo[i].xpos;
		if (ElectrodesInfo[i].ypos < min_y)
			min_y = ElectrodesInfo[i].ypos;
		if (ElectrodesInfo[i].xpos > max_x)
			max_x = ElectrodesInfo[i].xpos;
		if (ElectrodesInfo[i].ypos > max_y)
			max_y = ElectrodesInfo[i].ypos;
	}

	dx = (double)MEALayoutWidth / (double)(max_x - min_x + 2);
	dy = (double)(MEALayoutHeight - 30) / (double)(max_y - min_y + 2);
	dc = (double)(maxval) / 260.0;

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		x = (int)((double)(ElectrodesInfo[i].xpos - min_x + 1) * dx);
		y = (int)((double)(ElectrodesInfo[i].ypos - min_y + 1) * dy);
		color = (int)((double)LayoutHistogram[i] / dc);

		if (color > 255)
			color = 255;

		/* draw a 10x10 pixel antialiased circle into the bitmap using a template as a mask */

		for (u = 0; u < 10; ++u)
		{
			for (v = 0; v < 10; ++v)
			{
				offset = (MEALAYOUT_Y - (y + u - 5) - 1) * MEALAYOUT_X + x + v - 5;
				if ((offset >= 0) && (offset < (MEALAYOUT_X* MEALAYOUT_Y)))
				{
					buf_p = (PBYTE)pDIB_MEALayoutPixels + offset * 3;
					*buf_p = (BYTE) ((((spectrumLUT[color].rgbBlue - 255) * AntialieasedCircle[u][v]) >> 8) + 255);				/* a reduced version for combining the		*/
					*(buf_p + 1) = (BYTE) ((((spectrumLUT[color].rgbGreen - 255) * AntialieasedCircle[u][v]) >> 8) + 255);		/* BG and FG using only one multiplication  */
					*(buf_p + 2) = (BYTE) ((((spectrumLUT[color].rgbRed - 255) * AntialieasedCircle[u][v]) >> 8) + 255);		/* and no division							*/
				}
			}
		}

		/* draw a square around the selected electrode */

		if (i == SelectedTraceNum)
		{
			DrawLineBresenham(x - 10, y - 10, x + 10, y - 10, pDIB_MEALayoutInfo, pDIB_MEALayoutPixels, TraceGraphDataArray[i].pencolor);
			DrawLineBresenham(x - 10, y + 10, x + 10, y + 10, pDIB_MEALayoutInfo, pDIB_MEALayoutPixels, TraceGraphDataArray[i].pencolor);
			DrawLineBresenham(x - 10, y - 10, x - 10, y + 10, pDIB_MEALayoutInfo, pDIB_MEALayoutPixels, TraceGraphDataArray[i].pencolor);
			DrawLineBresenham(x + 10, y - 10, x + 10, y + 10, pDIB_MEALayoutInfo, pDIB_MEALayoutPixels, TraceGraphDataArray[i].pencolor);
		}
	}

	/* copy the bitmap to the screen */

	StretchDIBits(MEALayoutWindowDC, 0, 0, MEALAYOUT_X, MEALAYOUT_Y,
		0, 0, MEALAYOUT_X, MEALAYOUT_Y,
		pDIB_MEALayoutPixels, pDIB_MEALayoutInfo, DIB_RGB_COLORS, SRCCOPY);

	/* update the scale bar legend */

	oldfont = SelectObject(MEALayoutWindowDC, GetStockObject(ANSI_VAR_FONT));
	TextOut(MEALayoutWindowDC, 26, MEALayoutHeight - 30, "0", 2);
	i = sprintf_s(str, 100, "%d", maxval);
	TextOut(MEALayoutWindowDC, MEALayoutWidth - 26, MEALayoutHeight - 30, str, i + 1);
	SelectObject(MEALayoutWindowDC, oldfont);
}

/* an adapted version of the Bresenham line drawing algorithm
   note that it is assumed that xn >= x1 */

void
DrawLineBresenham(int x1, int y1, int xn, int yn, LPBITMAPINFO bminfo, PVOID pixels, int color_index)
{
	int	    dx, dy, di, ds, dt, multfact;
	BYTE    *dest;
	int	    offset, limit;
	int	    size_x, size_y;
	int	    r, g, b;

	/* a necessary precaution */

	if (x1 < 0)
		x1 = 0;
	if (xn < 0)
		xn = 0;
	if (x1 > bminfo->bmiHeader.biWidth)
		x1 = bminfo->bmiHeader.biWidth;
	if (xn > bminfo->bmiHeader.biWidth)
		xn = bminfo->bmiHeader.biWidth;
	if (y1 < 0)
		y1 = 0;
	if (yn < 0)
		yn = 0;
	if (y1 > bminfo->bmiHeader.biHeight)
		y1 = bminfo->bmiHeader.biHeight;
	if (yn > bminfo->bmiHeader.biHeight)
		yn = bminfo->bmiHeader.biHeight;

	r = PenColors[color_index].rgbRed;
	g = PenColors[color_index].rgbGreen;
	b = PenColors[color_index].rgbBlue;

	size_x = bminfo->bmiHeader.biWidth;
	size_y = bminfo->bmiHeader.biHeight;

	limit = size_x * size_y * 3;

	dx = xn - x1;
	
	if (yn < y1)
		multfact = -1;
	else
		multfact = 1;

	dy = abs(yn - y1);

	if (dx > dy)
	{
		di = 2 * dy - dx;
		ds = 2 * dy;
		dt = 2 * (dy - dx);

		offset = ((size_x * (size_y - 1 - y1)) + x1) * 3;
		if (offset < limit && offset >= 0)
		{
			dest = (PBYTE)pixels + offset;
			*dest = (BYTE) r;
			*(dest + 1) = (BYTE) g;
			*(dest + 2) = (BYTE) b;
		}

		while (x1 < xn)
		{
			x1++;
			if (di < 0)
				di = di + ds;
			else
			{
				y1 += multfact;
				di = di + dt;
			}

			offset = ((size_x * (size_y - 1 - y1)) + x1) * 3;
			if (offset >= limit || offset < 0)
				continue;
			dest = (PBYTE)pixels + offset;
			*dest = (BYTE) r;
			*(dest + 1) = (BYTE) g;
			*(dest + 2) = (BYTE) b;
		}
	}
	if (dx <= dy)
	{
		di = 2 * dx - dy;
		ds = 2 * dx;
		dt = 2 * (dx - dy);

		offset = ((size_x * (size_y - 1 - y1)) + x1) * 3;
		if (offset < limit && offset >= 0)
		{
			dest = (PBYTE)pixels + offset;
			*dest = (BYTE) r;
			*(dest + 1) = (BYTE) g;
			*(dest + 2) = (BYTE) b;
		}

		while ((multfact == 1 ? (y1 < yn) : (yn < y1)))
		{
			y1 += multfact;
			if (di < 0)
				di = di + ds;
			else
			{
				x1++;
				di = di + dt;
			}

			offset = ((size_x * (size_y - 1 - y1)) + x1) * 3;
			if (offset >= limit || offset < 0)
				continue;
			dest = (PBYTE)pixels + offset;
			*dest = (BYTE) r;
			*(dest + 1) = (BYTE) g;
			*(dest + 2) = (BYTE) b;
		}
	}
}

/* ------------------------- helper functions ---------------------------- */

/* figure out which channel/electrode was double clicked on */

int
GetElectrodeFromLayout(int clicked_x, int clicked_y)
{
	double				dy, dx;
	int					x, y, i;
	int					min_x, min_y, max_x, max_y;
	int					result;

	result = -1;

	min_x = min_y = 100000;
	max_x = max_y = -1;

	/* back calculate which electrode the user double clicked on */

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		if (ElectrodesInfo[i].xpos < min_x)
			min_x = ElectrodesInfo[i].xpos;
		if (ElectrodesInfo[i].ypos < min_y)
			min_y = ElectrodesInfo[i].ypos;
		if (ElectrodesInfo[i].xpos > max_x)
			max_x = ElectrodesInfo[i].xpos;
		if (ElectrodesInfo[i].ypos > max_y)
			max_y = ElectrodesInfo[i].ypos;
	}

	dx = (double)MEALayoutWidth / (double)(max_x - min_x + 2);
	dy = (double)(MEALayoutHeight - 30) / (double)(max_y - min_y + 2);

	for (i = 0; i < N_AD_CHANNELS; ++i)
	{
		if (ElectrodesInfo[i].inuse == FALSE)
			continue;

		x = (int)((double)(ElectrodesInfo[i].xpos - min_x + 1) * dx);
		y = (int)((double)(ElectrodesInfo[i].ypos - min_y + 1) * dy);

		if ((abs(clicked_x - x) < 5) && (abs(clicked_y - y) < 5))
		{
			result = i;
			break;
		}
	}

	return result;
}

/* check if the user is clicking near the threshold line in the single trace/spike windows*/

BOOL
IsPtNearThreshold(int electrode, int y)
{
	double y_origin, y_scl;
	double th_pos;

	y_origin = TraceSingleHeight / 2;
	y_scl = (double)(TraceSingleHeight - 40) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	th_pos = y_origin - corefunc_global_var.threshold_Per_Electrod[electrode] * y_scl;

	if (abs((int)(th_pos - y)) < 3)
		return TRUE;
	else
		return FALSE;
}


/* convert the mouse Y coordinate to threshold values */

double
ConvertPtToThreshold(int y)
{
	double y_origin, y_scl;

	if (y < 0)
		y = 0;

	if (y >= TraceSingleHeight)
		y = TraceSingleHeight - 1;

	y_origin = TraceSingleHeight / 2;
	y_scl = (double)(TraceSingleHeight - 40) / CurrentRecParms.VoltageFullScale;
	y_scl = y_scl * (CurrentRecParms.VoltageFullScale / CurrentRecParms.DisplayVoltageFullScale);

	return (double)(y_origin - y) / y_scl;
}

