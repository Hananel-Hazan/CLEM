/*
Closed Loop Experiment Manager

Window functions of the various modal dialog boxes

DialogBoxes.c
--------

Window procedures of dialog boxes in CLEM.

Public functions:
TimingSetup()
AboutDlgProc()
ThresholdSetup()
IOParametersSetup()
DataLogFileSetup()
PlaybackFileSetup()
ConvertFileSetup()
DigitalIODisplaySetup()
AddTextEntry()
FileOpenDlg()
FileSaveDlg()

*/


#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma hdrstop
#pragma warning ( disable : 4100)


#include "CLEM.h"
#include "GraphWindow.h"
#include "DialogBoxes.h"
#include "CoreFunction/CoreFunction.hpp"
#include <stdBOOL.h>
#include <Shlobj.h>
#include <Shlwapi.h>


#define WAIT_RMS_CALC	2

void	InitializeOFNstruct(OPENFILENAME *ofn, int filetype, HWND hwnd);
BOOL	ChooseDirectory(char *chosendir);

extern CoreFunction   *CoreObj;

/* Timed experiment sequence dialog box */

BOOL CALLBACK
TimingSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int			CurrDisplayedTopAction, SelectedAction;
	static RECT			RectFocus;
	static int			ItemSpacing, ItemOffset;
	RECT				rect;
	HBRUSH				redbrush;
	POINT				corner;
	BOOL				entry;
	int					i, j;
	char				str[20];
	HDC					hdc;
	PAINTSTRUCT			ps;
	
	switch (message)
	{
	case WM_INITDIALOG:

		CurrDisplayedTopAction = 0;
		SelectedAction = -1;

		/* initialize the dropdown combo controls and hide all controls */

		for (i = 1; i <= 6; ++i)
		{
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_INSERTSTRING, ACTION_PRINT, (LPARAM) "Write Comment to Log");
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_INSERTSTRING, ACTION_LOG, (LPARAM) "Start Logging");
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_INSERTSTRING, ACTION_LOADDLL, (LPARAM) "Load Closed-loop Plugin");
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_INSERTSTRING, ACTION_LOGANDEXEC, (LPARAM) "Start Logging & Closed-Loop");
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_INSERTSTRING, ACTION_PAUSE, (LPARAM) "Pause");
			SendMessage(GetDlgItem(hDlg, i * 100 + 1), CB_SETCURSEL, ACTION_PRINT, 0);

			ShowWindow(GetDlgItem(hDlg, i * 100 ), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, i * 100 + 1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, i * 100 + 2), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, i * 100 + 3), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, i * 100 + 4), SW_HIDE);

			SendMessage(GetDlgItem(hDlg, i * 100 + 3), EM_LIMITTEXT, 255, 0);
		}

		/* obtain dimensions and spacing of lines for the "focus" rectangle */

		GetWindowRect(GetDlgItem(hDlg, 200), &rect);	/* leftmost control */
		ItemSpacing = rect.top;
		GetWindowRect(GetDlgItem(hDlg, 100), &rect);
		ItemSpacing = ItemSpacing - rect.top;
		corner.x = rect.left;
		corner.y = rect.top;
		ScreenToClient(hDlg, &corner);
		RectFocus.left = corner.x - 2;
		RectFocus.top = corner.y - 6;
		ItemOffset = RectFocus.top;
		GetWindowRect(GetDlgItem(hDlg, 103), &rect);  /* rightmost control */
		corner.x = rect.right;
		corner.y = rect.bottom;
		ScreenToClient(hDlg, &corner);
		RectFocus.right = corner.x + 2;
		RectFocus.bottom = corner.y + 2;

		/* fall through */

	case WM_USER_REFRESH:

		if (ActionEntries < 7)
			CurrDisplayedTopAction = 0;
		if (ActionEntries <= SelectedAction)
			SelectedAction = ActionEntries - 1;

		/* expose only the minimal number of lines */
		
		for (i = 1; i <= 6; ++i)
		{
			ShowWindow(GetDlgItem(hDlg, i * 100),     (i <= ActionEntries ? SW_SHOW : SW_HIDE));
			ShowWindow(GetDlgItem(hDlg, i * 100 + 1), (i <= ActionEntries ? SW_SHOW : SW_HIDE));
			ShowWindow(GetDlgItem(hDlg, i * 100 + 2), (i <= ActionEntries ? SW_SHOW : SW_HIDE));
			ShowWindow(GetDlgItem(hDlg, i * 100 + 3), (i <= ActionEntries ? SW_SHOW : SW_HIDE));
			ShowWindow(GetDlgItem(hDlg, i * 100 + 4), (i <= ActionEntries ? SW_SHOW : SW_HIDE));

			EnableWindow(GetDlgItem(hDlg, i * 100),     !TimedExperiment);
			EnableWindow(GetDlgItem(hDlg, i * 100 + 1), !TimedExperiment);
			EnableWindow(GetDlgItem(hDlg, i * 100 + 2), !TimedExperiment);
			EnableWindow(GetDlgItem(hDlg, i * 100 + 3), !TimedExperiment);
			EnableWindow(GetDlgItem(hDlg, i * 100 + 4), !TimedExperiment);
		}

		for (i = 0; i < min(6, ActionEntries); ++i)
		{
			sprintf_s(str, 20, "%03d", i + CurrDisplayedTopAction + 1);
			SetDlgItemText(hDlg, i * 100 + 100, str);
			SendMessage(GetDlgItem(hDlg, i * 100 + 101), CB_SETCURSEL, ActionTable[i + CurrDisplayedTopAction].action, 0);
			sprintf_s(str, 20, "%.2f", ActionTable[i + CurrDisplayedTopAction].duration);
			SetDlgItemText(hDlg, i * 100 + 102, str);
			ActionTable[i + CurrDisplayedTopAction].argstring[255] = 0;
			SetDlgItemText(hDlg, i * 100 + 103, ActionTable[i + CurrDisplayedTopAction].argstring);
			CheckDlgButton(hDlg, i * 100 + 104, ActionTable[i + CurrDisplayedTopAction].active);
		}

		/* enable or disable the scrollbar according to the number of active entries */

		if (ActionEntries > 6)
		{ 
			EnableWindow(GetDlgItem(hDlg, 10), TRUE);
			SetScrollRange(GetDlgItem(hDlg, 10), SB_CTL, 0, ActionEntries - 6, TRUE);
			SetScrollPos(GetDlgItem(hDlg, 10), SB_CTL, 0, TRUE);
		}
		else
			EnableWindow(GetDlgItem(hDlg, 10), FALSE);

		EnableWindow(GetDlgItem(hDlg, 3), !TimedExperiment);
		EnableWindow(GetDlgItem(hDlg, 4), !TimedExperiment);
		
		return TRUE;

	case WM_VSCROLL:                /* vertical scroll bar messages */

		switch (LOWORD(wParam))
		{
			case SB_PAGEDOWN:
			case SB_LINEDOWN:
				CurrDisplayedTopAction = min(CurrDisplayedTopAction + 1, ActionEntries - 6);
				break;

			case SB_PAGEUP:
			case SB_LINEUP:
				CurrDisplayedTopAction = max(CurrDisplayedTopAction - 1, 0);
				break;

			case SB_TOP:
				CurrDisplayedTopAction = 0;
				break;

			case SB_BOTTOM:
				CurrDisplayedTopAction = ActionEntries - 6;
				break;

			case SB_THUMBPOSITION:
				CurrDisplayedTopAction = HIWORD(wParam);
				break;

			default:
				return 0;
		}

		/* set the scroll position to the new settings */

		SetScrollPos(GetDlgItem(hDlg, 10), SB_CTL, CurrDisplayedTopAction, TRUE);

		/* update the controls to display the right data */

		for (i = 0; i < 6; ++i)
		{
			sprintf_s(str, 20, "%03d", CurrDisplayedTopAction + i + 1);
			SetDlgItemText(hDlg, i * 100 + 100, str);
			SendMessage(GetDlgItem(hDlg, i * 100 + 101), CB_SETCURSEL, ActionTable[i + CurrDisplayedTopAction].action, 0);
			sprintf_s(str, 20, "%.2f", ActionTable[i + CurrDisplayedTopAction].duration);
			SetDlgItemText(hDlg, i * 100 + 102, str);
			ActionTable[i + CurrDisplayedTopAction].argstring[255] = 0;
			SetDlgItemText(hDlg, i * 100 + 103, ActionTable[i + CurrDisplayedTopAction].argstring);
			CheckDlgButton(hDlg, i * 100 + 104, ActionTable[i + CurrDisplayedTopAction].active);
		}

		/* update the window to redraw the focus rectangle correctly */

		InvalidateRect(hDlg, NULL, TRUE);

		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case 100:									/* messages from static controls */
			case 200:
			case 300:
			case 400:
			case 500:
			case 600:

			entry = CurrDisplayedTopAction + LOWORD(wParam) / 100 - 1;

			/* set the focus on this line */

			if (HIWORD(wParam) == STN_CLICKED)				
			{
				SelectedAction = entry;
				InvalidateRect(hDlg, NULL, TRUE);
			}

			return TRUE;
		
			case 101:										/* messages from dropdown lists */
			case 201:
			case 301:
			case 401:
			case 501:
			case 601:
		
				entry = CurrDisplayedTopAction + LOWORD(wParam) / 100 - 1;

				/* set the focus on this line */

				if (HIWORD(wParam) == CBN_SETFOCUS)			
				{
					SelectedAction = entry;
					InvalidateRect(hDlg, NULL, TRUE);
					return TRUE;
				}

				i = (int) SendMessage(GetDlgItem(hDlg, LOWORD(wParam)), CB_GETCURSEL, 0, 0);
				ActionTable[entry].action = i;

				return TRUE;

			case 102:										/* messages from Duration edit controls */
			case 202:
			case 302:
			case 402:
			case 502:
			case 602:

				entry = CurrDisplayedTopAction + LOWORD(wParam) / 100 - 1;

				/* set the focus on this line */

				if (HIWORD(wParam) == EN_SETFOCUS)			
				{
					SelectedAction = entry;
					InvalidateRect(hDlg, NULL, TRUE);
					return TRUE;
				}
				if (HIWORD(wParam) == EN_CHANGE)
				{ 
					GetDlgItemText(hDlg, LOWORD(wParam), str, 20);
					ActionTable[entry].duration = atof(str);
				}
				return TRUE;

			case 103:										/* messages from arguments string edit controls */
			case 203:
			case 303:
			case 403:
			case 503:
			case 603:

				entry = CurrDisplayedTopAction + LOWORD(wParam) / 100 - 1;

				/* set the focus on this line */

				if (HIWORD(wParam) == EN_SETFOCUS)			
				{
					SelectedAction = entry;
					InvalidateRect(hDlg, NULL, TRUE);
					return TRUE;

				}
				if (HIWORD(wParam) == EN_CHANGE)			
					GetDlgItemText(hDlg, LOWORD(wParam), ActionTable[entry].argstring, 255);

				return TRUE;

			case 104:							/* messages from arguments checkboxes */
			case 204:
			case 304:
			case 404:
			case 504:
			case 604:

				if (HIWORD(wParam) == BN_CLICKED)
				{
					/* set the focus on this line */

					entry = CurrDisplayedTopAction + LOWORD(wParam) / 100 - 1;	
										
					SelectedAction = entry;							
					InvalidateRect(hDlg, NULL, TRUE);

					ActionTable[entry].active = !ActionTable[entry].active;
					CheckDlgButton (hDlg, LOWORD(wParam), ActionTable[entry].active);
				}

				return TRUE;

			case 3:								/* add an action */  

				if (ActionEntries < MAXACTIONS)
					ActionEntries++;
				else
					return TRUE;

				for (i = 0; i < min(6, ActionEntries); ++i)
				{
					ShowWindow(GetDlgItem(hDlg, i * 100 + 100), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, i * 100 + 101), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, i * 100 + 102), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, i * 100 + 103), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, i * 100 + 104), SW_SHOW);
				}

				/* Shift table entries to make space for the new entry */

				for (i = MAXACTIONS - 1; i > SelectedAction + 1; --i)
				{
					ActionTable[i].action    = ActionTable[i - 1].action;
					ActionTable[i].duration = ActionTable[i - 1].duration;
					memcpy_s(ActionTable[i].argstring, 256, ActionTable[i - 1].argstring, 256);
					ActionTable[i].active = ActionTable[i - 1].active;
				}
				
				SelectedAction++;
				ActionTable[SelectedAction].action = 0;
				ActionTable[SelectedAction].duration = 0;
				memset(ActionTable[SelectedAction].argstring, 0, 256);
				ActionTable[i].active = TRUE;
				
				/* update the display */

				if ((SelectedAction - CurrDisplayedTopAction) > 5)
					CurrDisplayedTopAction = SelectedAction - 5;
				if (ActionEntries > 6)
				{
					EnableWindow(GetDlgItem(hDlg, 10), TRUE);
					SetScrollRange(GetDlgItem(hDlg, 10), SB_CTL, 0, ActionEntries - 6, TRUE);
					SetScrollPos(GetDlgItem(hDlg, 10), SB_CTL, CurrDisplayedTopAction, TRUE);	
				}
				for (i = 0; i < min(6, ActionEntries); ++i)
				{
					sprintf_s(str, 20, "%03d", CurrDisplayedTopAction + i + 1);
					SetDlgItemText(hDlg, i * 100 + 100, str);
					SendMessage(GetDlgItem(hDlg, i * 100 + 101), CB_SETCURSEL, ActionTable[i + CurrDisplayedTopAction].action, 0);
					sprintf_s(str, 20, "%.2f", ActionTable[i + CurrDisplayedTopAction].duration);
					SetDlgItemText(hDlg, i * 100 + 102, str);
					ActionTable[i + CurrDisplayedTopAction].argstring[255] = 0;
					SetDlgItemText(hDlg, i * 100 + 103, ActionTable[i + CurrDisplayedTopAction].argstring);
					CheckDlgButton(hDlg, i * 100 + 104, ActionTable[i + CurrDisplayedTopAction].active);
				}

				PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
				InvalidateRect(hDlg, NULL, TRUE);

				return TRUE;

			case 4:								/* delete an action */

				if (SelectedAction > ActionEntries)
					return TRUE;

				if (ActionEntries > 0)
					ActionEntries--;

				/* hide lines if necessary */

				if (ActionEntries < 6)		
				{
					for (i = 5; i >= ActionEntries; --i)
					{
						ShowWindow(GetDlgItem(hDlg, i * 100 + 100), SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, i * 100 + 101), SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, i * 100 + 102), SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, i * 100 + 103), SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, i * 100 + 104), SW_HIDE);
					}
				}
				else
					CurrDisplayedTopAction = max(CurrDisplayedTopAction--, 0);

				/* Delete table entries to make space for the new entry */

				for (i = SelectedAction; i < MAXACTIONS - 2; ++i)
				{
					ActionTable[i].action = ActionTable[i + 1].action;
					ActionTable[i].duration = ActionTable[i + 1].duration; 
					memcpy_s (ActionTable[i].argstring, 256, ActionTable[i + 1].argstring, 256);
					ActionTable[i].active = ActionTable[i + 1].active;
				}
				ActionTable[MAXACTIONS - 1].action = 0;
				ActionTable[MAXACTIONS - 1].duration = 0;
				memset (ActionTable[MAXACTIONS - 1].argstring, 0, 256);
				ActionTable[MAXACTIONS - 1].active = TRUE;
				
				if (SelectedAction >= ActionEntries)
					SelectedAction = ActionEntries - 1;
			
				/* update the display */

				if (ActionEntries < 7)
				{ 
					CurrDisplayedTopAction = 0;
					EnableWindow(GetDlgItem(hDlg, 10), FALSE);
				}
				else
				{
					SetScrollRange(GetDlgItem(hDlg, 10), SB_CTL, 0, ActionEntries - 6, TRUE);
					SetScrollPos(GetDlgItem(hDlg, 10), SB_CTL, CurrDisplayedTopAction, TRUE);
				}
				for (i = 0; i < min(6, ActionEntries); ++i)
				{
					sprintf_s(str, 20, "%03d", CurrDisplayedTopAction + i + 1);
					SetDlgItemText(hDlg, i * 100 + 100, str);
					SendMessage(GetDlgItem(hDlg, i * 100 + 101), CB_SETCURSEL, ActionTable[i + CurrDisplayedTopAction].action, 0);
					sprintf_s(str, 20, "%.2f", ActionTable[i + CurrDisplayedTopAction].duration);
					SetDlgItemText(hDlg, i * 100 + 102, str);
					ActionTable[i + CurrDisplayedTopAction].argstring[255] = 0; 
					SetDlgItemText(hDlg, i * 100 + 103, ActionTable[i + CurrDisplayedTopAction].argstring);
					CheckDlgButton(hDlg, i * 100 + 104, ActionTable[i + CurrDisplayedTopAction].active);
				}

				PostMessage(hwndControlDialogBox, WM_USER_REFRESH, 0, 0);
				InvalidateRect(hDlg, NULL, TRUE);

				return TRUE;

			case IDCANCEL:

				ShowWindow(hDlg, SW_HIDE);

				return TRUE;
		}
		break;

		case WM_PAINT:

			hdc = BeginPaint(hDlg, &ps);
			
			if (!TimedExperiment)
			{
				/* draw an "input focus" rectangle around the current line */

				j = SelectedAction - CurrDisplayedTopAction;
				if (j >= 0 && j < 6)
				{
					i = RectFocus.bottom - RectFocus.top;
					RectFocus.top = ItemOffset + ItemSpacing * j;
					RectFocus.bottom = RectFocus.top + i;
					DrawFocusRect(hdc, &RectFocus);
				}
			}
			else
			{ 
				/* draw a red rectangle around the line being executed */

				j = CurrActionEntry - CurrDisplayedTopAction;
				if (j >= 0 && j < 6)
				{
					CopyRect(&rect, &RectFocus);
					i = rect.bottom - rect.top;
					rect.top = ItemOffset + ItemSpacing * j;
					rect.bottom = rect.top + i + 1;
					redbrush = CreateSolidBrush(RGB(255, 0, 0));
					FrameRect(hdc, &rect, redbrush);
					DeleteObject(redbrush);
				}
			}
			
			EndPaint(hDlg, &ps);
			
			return TRUE;		
	}
	return FALSE;
}

/* "About" Dialog box */

BOOL CALLBACK
AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/* Dialog box for setting up IO parameters */

BOOL CALLBACK
IOParametersSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int				sr, cv, sh, st, stp, srf;
	char			str[200];

	switch (message)
	{
	case WM_INITDIALOG:

		/* copy current values to controls */

		SetDlgItemInt(hDlg, 100, GlobalRecParms.BoardSampleRate, FALSE);
		SetDlgItemInt(hDlg, 200, GlobalRecParms.ContVoltageStorageDuration, FALSE);
		SetDlgItemInt(hDlg, 201, GlobalRecParms.SpikeHistoryLength, FALSE);
		SetDlgItemInt(hDlg, 300, GlobalRecParms.SpikeTraceDuration, FALSE);
		SetDlgItemInt(hDlg, 301, GlobalRecParms.SpikeTracePreTrigger, FALSE);
		SetDlgItemInt(hDlg, 302, GlobalRecParms.SpikeRefractoryPeriodMicrosecond, FALSE);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:

			/* get the values, check ranges and copy  to appropriate global variables */

			sr = GetDlgItemInt(hDlg, 100, NULL, FALSE);
			if (sr < MIN_AD_FREQ || sr > MAX_AD_FREQ)
			{
				sprintf_s(str, 200, "Sampling frequency must be in the range of %d to %d Hz",
					MIN_AD_FREQ, MAX_AD_FREQ);
				MessageBox(hDlg, str, "Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			cv = GetDlgItemInt(hDlg, 200, NULL, FALSE);
			if (cv < MIN_VOLT_HISTORY || cv > MAX_VOLT_HISTORY)
			{
				sprintf_s(str, 200, "Voltage trace history duration must be in the range of %d to %d minutes",
					MIN_VOLT_HISTORY, MAX_VOLT_HISTORY);
				MessageBox(hDlg, str, "Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			sh = GetDlgItemInt(hDlg, 201, NULL, FALSE);
			if (sh < MIN_SPIKE_HISTORY || sh > MAX_SPIKE_HISTORY)
			{
				sprintf_s(str, 200, "Spike must be in the range of %d to %d events",
					MIN_SPIKE_HISTORY, MAX_SPIKE_HISTORY);
				MessageBox(hDlg, str, "Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			st = GetDlgItemInt(hDlg, 300, NULL, FALSE);
			if (st < MIN_SPIKE_TRACE || st > MAX_SPIKE_TRACE)
			{
				sprintf_s(str, 200, "Spike trace duration must be in the range of %d to %d msec",
					MIN_SPIKE_TRACE, MAX_SPIKE_TRACE);
				MessageBox(hDlg, str, "Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			stp = GetDlgItemInt(hDlg, 301, NULL, FALSE);
			if (stp < 1 || stp >= st)
			{
				MessageBox(hDlg, "pretrigger duration must be less than the trace duration",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			srf = GetDlgItemInt(hDlg, 302, NULL, FALSE);
			if (srf < MINREFRACTORYMICROSEC)
			{
				sprintf_s(str, 200, "The  mininal refractory period (microsec) is %d", MINREFRACTORYMICROSEC);
				MessageBox(hDlg, str, "Error", MB_OK | MB_ICONEXCLAMATION);
				return TRUE;
			}

			GlobalRecParms.BoardSampleRate = sr;
			GlobalRecParms.ContVoltageStorageDuration = cv;
			GlobalRecParms.SpikeHistoryLength = sh;
			GlobalRecParms.SpikeTraceDuration = st;
			GlobalRecParms.SpikeTracePreTrigger = stp;
			GlobalRecParms.SpikeRefractoryPeriodMicrosecond = srf;
			
			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}

/* setup spike thresholds */

BOOL CALLBACK
ThresholdSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static float	tholds[N_AD_CHANNELS];
	static float	globalTH;
	static float	multfactor;
	static float	measureduration;
	static UINT_PTR	waiting_timer;
	int				i;
	char			msg[100];
	float			val;
	static HCURSOR	hCursor;

	switch (message)
	{
	case WM_INITDIALOG:

		multfactor = CurrentRecParms.ThresholdRMSMultFactor;
		measureduration = CurrentRecParms.ThresholdRMSMeasureDuration;
		globalTH = CurrentRecParms.GlobalThreshold;

		sprintf_s(msg, 100, "%.3f", globalTH);
		SetDlgItemText(hDlg, 200, msg);
		sprintf_s(msg, 100, "%.2f", measureduration);
		SetDlgItemText(hDlg, 202, msg);
		sprintf_s(msg, 100, "%.2f", multfactor);
		SetDlgItemText(hDlg, 203, msg);

		for (i = 0; i < N_AD_CHANNELS; ++i)
			tholds[i] = corefunc_global_var.threshold_Per_Electrod[i];

		sprintf_s(msg, 100, "%.3f", tholds[0]);
		SetDlgItemText(hDlg, 201, msg);

		if (!Recording && !PlayingBack)
		{
			EnableWindow(GetDlgItem(hDlg, 102), FALSE);
			EnableWindow(GetDlgItem(hDlg, 103), FALSE);
		}

		for (i = 0; i < N_AD_CHANNELS; ++i)
		{
			if (ElectrodesInfo[i].inuse == FALSE)
				continue;

			sprintf_s(msg, 100, "%d  %s", i + 1, ElectrodesInfo[i].name);
			SendMessage(GetDlgItem(hDlg, 300), CB_INSERTSTRING, i, (LPARAM)msg);
		}

		SendMessage(GetDlgItem(hDlg, 300), CB_SETCURSEL, 0, 0);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:						/* Set threshold for all channels button */

			GetDlgItemText(hDlg, 200, msg, 100);
			val = (float)atof(msg);
			if (val < 0 && val > -10.0)
			{
				globalTH = val;
				for (i = 0; i < N_AD_CHANNELS; ++i)
					corefunc_global_var.threshold_Per_Electrod[i] = val;
			}

			sprintf_s(msg, 100, "%.3f", globalTH);
			SetDlgItemText(hDlg, 201, msg);

			/* update threshold display in single trace window */

			InvalidateRect(hwndTraceSingleWindow, NULL, FALSE);
			UpdateWindow(hwndTraceSingleWindow);

			return TRUE;

		case 101:					/* Set threshold for one channel button  */

			GetDlgItemText(hDlg, 201, msg, 100);
			val = (float)atof(msg);
			if (val < 0 && val > -10.0)
			{
				i = (int)SendMessage(GetDlgItem(hDlg, 300), CB_GETCURSEL, 0, 0);
				corefunc_global_var.threshold_Per_Electrod[i] = val;
			}

			/* update threshold display in single trace window */

			InvalidateRect(hwndTraceSingleWindow, NULL, FALSE);
			UpdateWindow(hwndTraceSingleWindow);

			return TRUE;

		case 102:					/* calculate now (for all channels) button */

			/* if a threshold calculation is in progress, do nothing */
			
			if (corefunc_global_var.Calculating_Threshold > 0)
				return TRUE;

			EnableWindow(GetDlgItem(hDlg, 102), FALSE);
			EnableWindow(GetDlgItem(hDlg, 103), FALSE);

			Recalculate_Threshold_For_All_Electrodes(CoreObj, (int) (measureduration * 1000), multfactor);

			/* setup a timer to call back after the calculation is finished */

			if (waiting_timer)
				return 0;

			waiting_timer = SetTimer(hDlg, WAIT_RMS_CALC, 500, NULL);
			if (waiting_timer != 0)
			{
				WriteMessage("Recalculating thresholds for all channels. Please wait......", FALSE);

				/* capture the mouse, set to "wait" icon */

				hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				ShowCursor(TRUE);
			}

			return 0;			

		case 103:				/* calculate now (for one channel button */

			/* if a threshold calculation is in progress, do nothing */

			if (corefunc_global_var.Calculating_Threshold > 0)
				return TRUE;

			EnableWindow(GetDlgItem(hDlg, 102), FALSE);
			EnableWindow(GetDlgItem(hDlg, 103), FALSE);

			i = (int)SendMessage(GetDlgItem(hDlg, 300), CB_GETCURSEL, 0, 0);
			Recalculate_Threshold_For(CoreObj, i, (int) (measureduration * 1000), multfactor);

			/* setup a timer to call back after the calculation is finished */

			if (waiting_timer)
				return 0;

			waiting_timer = SetTimer(hDlg, WAIT_RMS_CALC, 500, NULL);
			if (waiting_timer != 0)
			{
				sprintf_s(msg, 100, "Recalculating threshold for channel %s. Please wait......", ElectrodesInfo[i].name);
				WriteMessage(msg, FALSE);

				/* capture the mouse, set to "wait" icon */

				hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				ShowCursor(TRUE);
			}

			return 0;

		case 202:				/* all channels threshold value Edit control */	

			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				GetDlgItemText(hDlg, 202, msg, 100);
				measureduration = (float)atof(msg);
			}
			return TRUE;

		case 203:				/* specific channel threshold value Edit control */

			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				GetDlgItemText(hDlg, 203, msg, 100);
				multfactor = (float)atof(msg);
			}
			return TRUE;

		case 300:				/* channel Combo control  */

			if (HIWORD(wParam) != CBN_SELCHANGE)
				return TRUE;

			i = (int)SendMessage(GetDlgItem(hDlg, 300), CB_GETCURSEL, 0, 0L);
			sprintf_s(msg, 100, "%.3f", corefunc_global_var.threshold_Per_Electrod[i]);
			SetDlgItemText(hDlg, 201, msg);
			return TRUE;

		case IDOK:				/* OK button: store values and exit */

			if (waiting_timer)
				KillTimer(hDlg, waiting_timer);
			waiting_timer = 0;

			CurrentRecParms.ThresholdRMSMultFactor = multfactor;
			CurrentRecParms.ThresholdRMSMeasureDuration = measureduration;
			GetDlgItemText(hDlg, 200, msg, 100);
			globalTH = (float)atof(msg);
			CurrentRecParms.GlobalThreshold = globalTH;

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:			/* Cancel button */

			if (waiting_timer)
				KillTimer(hDlg, waiting_timer);
			waiting_timer = 0;

			for (i = 0; i < N_AD_CHANNELS; ++i)
				corefunc_global_var.threshold_Per_Electrod[i] = tholds[i];

			EndDialog(hDlg, FALSE);

			return TRUE;
		}

		break;

	case WM_TIMER:

		/* verify that the call is from the appropriate timer (an error here should never happen) */

		if (wParam != WAIT_RMS_CALC)
			return TRUE;

		/* still calculating ? continue to wait...*/

		if (corefunc_global_var.Calculating_Threshold > 0)
			return TRUE;

		/* if not, update the settings */

		KillTimer(hDlg, waiting_timer);
		waiting_timer = 0;

		/* release the mouse */

		ShowCursor(FALSE);
		SetCursor(hCursor);

		WriteMessage("Done", FALSE);

		/* display the new settings */

		for (i = 0; i < N_AD_CHANNELS; ++i)
			tholds[i] = corefunc_global_var.threshold_Per_Electrod[i];

		i = (int) SendMessage(GetDlgItem(hDlg, 300), CB_GETCURSEL, 0, 0);
		if (i >= 0 && i < N_AD_CHANNELS)
		{
			sprintf_s(msg, 100, "%.3f", tholds[i]);
			SetDlgItemText(hDlg, 201, msg);
		}

		EnableWindow(GetDlgItem(hDlg, 102), TRUE);
		EnableWindow(GetDlgItem(hDlg, 103), TRUE);

		return TRUE;
	}

	return FALSE;
}


/* define data logging settings and options */

BOOL CALLBACK
DataLogFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL			r;
	static char		dirname[MAX_PATH];

	switch (message)
	{
	case WM_INITDIALOG:

		/* copy currnt settings to all controls */

		SetDlgItemText(hDlg, 101, LogFilePath);
		SetDlgItemText(hDlg, 103, LogFilePrefix);
		SetDlgItemInt(hDlg, 104, MaxLogFileSize, FALSE);
		SetDlgItemInt(hDlg, 105, FirstLogFileNumber, FALSE);
		CheckDlgButton(hDlg, 110, LogVolts ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 111, LogSpikes ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 112, LogDigIO ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 113, LogAnalogOut ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 114, LogSpikeTimes ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 115, LogUserGeneratedData ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, 116, LogMessages ? BST_CHECKED : BST_UNCHECKED);
	
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case 102:				/* "..." button for selecting a directory */

			/* open a dialog box to obtain the directory name */

			r = ChooseDirectory(dirname);
			if (r == TRUE)
				SetDlgItemText(hDlg, 101, dirname);

			return TRUE;

		case IDOK:				/* OK button */

			/* store the new settings */

			GetDlgItemText(hDlg, 101, LogFilePath, MAX_PATH);
			GetDlgItemText(hDlg, 103, LogFilePrefix, 100);
			MaxLogFileSize = GetDlgItemInt(hDlg, 104, NULL, FALSE);
			FirstLogFileNumber = GetDlgItemInt(hDlg, 105, NULL, FALSE);
			LogVolts = (BOOL)IsDlgButtonChecked(hDlg, 110);
			LogSpikes = (BOOL)IsDlgButtonChecked(hDlg, 111);
			LogDigIO = (BOOL)IsDlgButtonChecked(hDlg, 112);
			LogAnalogOut = (BOOL)IsDlgButtonChecked(hDlg, 113);
			LogSpikeTimes = (BOOL)IsDlgButtonChecked(hDlg, 114);
			LogUserGeneratedData = (BOOL)IsDlgButtonChecked(hDlg, 115);
			LogMessages = (BOOL)IsDlgButtonChecked(hDlg, 116);

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:			/* Cancel button */

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}


/* Select the files to playback */

BOOL CALLBACK
PlaybackFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int			i, j, k, first, last;
	BOOL		r;
	char		fname[MAX_PATH + _MAX_FNAME + _MAX_EXT];
	char	    str[MAX_PATH + _MAX_FNAME + _MAX_EXT];
	char		nstr[100];
	LPSTR		name, nmbr, ext;

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case 100:				/* "Select First File" button */

			/* open a standard Windows file open dialog box and get the first file */

			fname[0] = 0;
			r = FileOpenDlg(hDlg, FT_ACTIVITY, fname, "Load a Recording File");
			if (r != TRUE)
				return TRUE;

			/* copy full name to dialog box */

			SetDlgItemText(hDlg, 101, fname);

			str[MAX_PATH + _MAX_FNAME + _MAX_EXT - 1] = 0;

			/* copy directory */

			name = PathFindFileName(fname);
			for (i = 0; i < (name - fname); ++i)
				str[i] = fname[i];
			str[i] = 0;
			SetDlgItemText(hDlg, 102, str);

			/* copy prefix */

			strcpy_s(str, MAX_PATH + _MAX_FNAME + _MAX_EXT, name);
			j = (int) strlen(str);
			for (i = 0; i < j; ++i)
				if (str[i] == '.')
				{
					str[i] = 0;
					break;
				}
			SetDlgItemText(hDlg, 103, str);

			/* get file extension*/

			SetDlgItemText(hDlg, 106, "");
			
			ext = PathFindExtension(fname);
			if (strcmp(ext, ".vlt") == 0)
				SetDlgItemText(hDlg, 106, "Continuous");
			if (strcmp(ext, ".spk") == 0)
				SetDlgItemText(hDlg, 106, "Spikes");
			if (strcmp(ext, ".spt") == 0)
				SetDlgItemText(hDlg, 106, "Spike Times");		

			/* get file ordinal number */

			name = name + i + 1;
			j = (int) (ext - name);
			i = 0;
			while (name[i] != '.' && i < j)
			{
				str[i] = name[i];
				i++;
			}
			if (name[i] == '.')
			{
				str[i] = 0;
				j = i;
				for (i = 0; i < j; ++i)
					if (!isdigit((unsigned char) name[i]))		/* verify an all digit string */
						break;
				if (i == j)
				{
					strncpy_s(nstr, 100, name, j);
					SetDlgItemText(hDlg, 104, nstr);

					first = atoi(str);

					/* find last file */

					i = last = first;
					strcpy_s(str, MAX_PATH + _MAX_FNAME + _MAX_EXT, fname);
					nmbr = strstr(str, nstr);
					while (i < 10000000)
					{
						i++;
						sprintf_s(nstr, 100, "%0*d", j, i);
						for (k = 0; k < j; ++k)
							if (nmbr)
								nmbr[k] = nstr[k];
						if (!PathFileExists(str))
							break;
						else
							last++;
					}

					/* display the last file name */

					sprintf_s(nstr, 100, "%0*d", j, last);
					SetDlgItemText(hDlg, 105, nstr);
				}
			}

			return TRUE;

		case IDOK:				/* OK button */

			/* store settings */

			GetDlgItemText(hDlg, 102, PlaybackFileDir, MAX_PATH);
			GetDlgItemText(hDlg, 103, PlayBackFileNamePrefix, _MAX_FNAME);
			FirstPlaybackFile = GetDlgItemInt(hDlg, 104, NULL, FALSE);
			LastPlaybackFile = GetDlgItemInt(hDlg, 105, NULL, FALSE);
			GetDlgItemText(hDlg, 106, str, MAX_PATH + _MAX_FNAME + _MAX_EXT);
			if (strcmp(str, "Continuous") == 0)
				PlaybackFileType = PBFILETYPEVOLT;
			if (strcmp(str, "Spikes") == 0)
				PlaybackFileType = PBFILETYPESPIKES;
			if (strcmp(str, "Spike Times") == 0)
				PlaybackFileType = PBFILETYPESPIKESTIMES;

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:			/* Cancel button */

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL CALLBACK
ConvertFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int			i, j, k, first, last;
	BOOL		r;
	char		fname[MAX_PATH + _MAX_FNAME + _MAX_EXT];
	char	    str[MAX_PATH + _MAX_FNAME + _MAX_EXT];
	char		nstr[100];
	char		dirname[MAX_PATH];
	LPSTR		name, nmbr, ext;

	switch (message)
	{

	case WM_INITDIALOG:

		SetDlgItemText(hDlg, 108, ConvertOutFileDir);
		SetDlgItemText(hDlg, 102, ConvertInFileDir);
		SetDlgItemText(hDlg, 103, ConvertInFileNamePrefix);
		sprintf_s(nstr, 100, "%06d", FirstConvertFile);
		SetDlgItemText(hDlg, 104, nstr);
		sprintf_s(nstr, 100, "%06d", LastConvertFile);
		SetDlgItemText(hDlg, 105, nstr);
		if (ConvertFileType == DM_CONTINUOUS)
			SetDlgItemText(hDlg, 106, "Continuous");
		else
			SetDlgItemText(hDlg, 106, "Spikes");

		if (Converting)
		{
			for (i = 100; i <= 108; ++i)
				EnableWindow(GetDlgItem(hDlg, i), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		}
		else
			ShowWindow(GetDlgItem(hDlg, IDABORT), SW_HIDE);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case 100:				/* "Select First File" button */

			/* open a standard Windows file open dialog box and get the first file */

			fname[0] = 0;
			r = FileOpenDlg(hDlg, FT_ACTIVITY, fname, "Load the first file to convert");
			if (r != TRUE)
				return TRUE;

			/* copy its full name to dialog box */

			SetDlgItemText(hDlg, 101, fname);

			str[MAX_PATH + _MAX_FNAME + _MAX_EXT - 1] = 0;

			/* copy directory */

			name = PathFindFileName(fname);
			for (i = 0; i < (name - fname); ++i)
				str[i] = fname[i];
			str[i] = 0;
			SetDlgItemText(hDlg, 102, str);

			/* copy prefix */

			strcpy_s(str, MAX_PATH + _MAX_FNAME + _MAX_EXT, name);
			j = (int) strlen(str);
			for (i = 0; i < j; ++i)
				if (str[i] == '.')
				{
					str[i] = 0;
					break;
				}
			SetDlgItemText(hDlg, 103, str);

			/* get file extension*/

			SetDlgItemText(hDlg, 106, "");

			ext = PathFindExtension(fname);
			
			if (strcmp(ext, ".vlt") == 0)
				SetDlgItemText(hDlg, 106, "Continuous");
			
			if (strcmp(ext, ".spk") == 0)
					SetDlgItemText(hDlg, 106, "Spikes");

			if (strcmp(ext, ".spt") == 0)
				SetDlgItemText(hDlg, 106, "Spike Times");
			
			/* get file ordinal number */

			name = name + i + 1;
			j = (int) (ext - name);
			i = 0;
			while (name[i] != '.' && i < j)
			{
				str[i] = name[i];
				i++;
			}
			if (name[i] == '.')
			{
				str[i] = 0;
				j = i;
				for (i = 0; i < j; ++i)
					if (!isdigit((unsigned char) name[i]))		/* verify an all digit string */
						break;
				if (i == j)
				{
					strncpy_s(nstr, 100, name, j);
					SetDlgItemText(hDlg, 104, nstr);

					first = atoi(str);

					/* find last file */

					i = last = first;
					strcpy_s(str, MAX_PATH + _MAX_FNAME + _MAX_EXT, fname);
					nmbr = strstr(str, nstr);
					while (i < 10000000)
					{
						i++;
						sprintf_s(nstr, 100, "%0*d", j, i);
						for (k = 0; k < j; ++k)
							if (nmbr)
								nmbr[k] = nstr[k];
						if (!PathFileExists(str))
							break;
						else
							last++;
					}

					/* show the last file */

					sprintf_s(nstr, 100, "%0*d", j, last);
					SetDlgItemText(hDlg, 105, nstr);
				}
			}

			return TRUE;

		case 107:

			/* open a dialog box to obtain the directory name */

			r = ChooseDirectory(dirname);
			if (r == TRUE)
				SetDlgItemText(hDlg, 108, dirname);

			return TRUE;

		case IDABORT:

			EndDialog(hDlg, 3);

			return TRUE;

		case IDOK:

			GetDlgItemText(hDlg, 108, ConvertOutFileDir, MAX_PATH);
			GetDlgItemText(hDlg, 102, ConvertInFileDir, MAX_PATH);
			GetDlgItemText(hDlg, 103, ConvertInFileNamePrefix, _MAX_FNAME);
			FirstConvertFile = GetDlgItemInt(hDlg, 104, NULL, FALSE);
			LastConvertFile = GetDlgItemInt(hDlg, 105, NULL, FALSE);
			GetDlgItemText(hDlg, 106, str, MAX_PATH + _MAX_FNAME + _MAX_EXT);
			if (strcmp(str, "Continuous") == 0)
				ConvertFileType = DM_CONTINUOUS;
			if (strcmp(str, "Spikes") == 0)
				ConvertFileType = DM_LEVEL; 

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}

/* select the digital IO lines to display */

BOOL CALLBACK
DigitalIODisplaySetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int		input_line, output_line;

	switch (message)
	{
	case WM_INITDIALOG:

		input_line = 0;
		while (DigInputDisplayMask >> (input_line + 1))
			input_line++;

		output_line = 0;
		while (DigOutputDisplayMask >> (output_line + 1))
			output_line++;

		CheckRadioButton(hDlg, 100, 107, 100 + input_line);
		CheckRadioButton(hDlg, 200, 207, 200 + output_line);
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:				/* digital input line radio buttons */
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:

			CheckRadioButton(hDlg, 100, 107, LOWORD(wParam));
			input_line = LOWORD(wParam) - 100;
			return TRUE;

		case 200:				/* digital output line radio buttons */
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:

			CheckRadioButton(hDlg, 200, 207, LOWORD(wParam));
			output_line = LOWORD(wParam) - 200;

			return TRUE;

		case IDOK:				/* OK button - store settings */

			DigInputDisplayMask = 1 << input_line;
			DigOutputDisplayMask = 1 << output_line;

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}


/* ad a manual text message entry into the text message log */

BOOL CALLBACK
AddTextEntry(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char		str[200];

	switch (message)
	{
	case WM_INITDIALOG:

		/* limit the length of the text the user can enter */

		SendMessage (GetDlgItem(hDlg, 100), EM_LIMITTEXT, 190, 0);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:

			/* get the text and add a message */

			GetDlgItemText(hDlg, 100, str, 190);

			WriteMessage(str, FALSE);

			EndDialog(hDlg, TRUE);

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, FALSE);

			return TRUE;
		}
		break;
	}
	return FALSE;
}



/* open a standard Wondows File Open dialog box for the give file type */

BOOL
FileOpenDlg(HWND hwnd, int filetype, PSTR FileName, PSTR TitleName)
{
	OPENFILENAME ofn;

	InitializeOFNstruct(&ofn, filetype, hwnd);

	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = FileName;
	ofn.lpstrTitle = TitleName;

	return GetOpenFileName(&ofn);
}


/* open a standard Wondows File Save dialog box for the give file type */

BOOL
FileSaveDlg(HWND hwnd, int filetype, PSTR FileName, PSTR TitleName)
{
	OPENFILENAME ofn;

	InitializeOFNstruct(&ofn, filetype, hwnd);

	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = FileName;
	ofn.lpstrTitle = TitleName;
	ofn.Flags = ofn.Flags | OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}


/* --------------------------------------- helper functions ------------------------------------ */

/* fill in an OPENFILENAME data structure according to the requested file type  */

void
InitializeOFNstruct(OPENFILENAME *ofn, int filetype, HWND hwnd)
{
	static char *szFilter[6] = { "Plugin Files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0\0",
		"Setting Files (*.clm)\0*.clm\0All Files (*.*)\0*.*\0\0",
		"Electrode layout Files (*.elt)\0*.elt\0All Files (*.*)\0*.*\0\0",
		"Spike Files (*.spk)\0*.spk\0Voltage Files (*.vlt)\0*.vlt\0Spike Time Files (*.spt)\0*.spt\0All Files (*.*)\0*.*\0\0",
		"Messages Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0\0",
		"Experiment Seq Files (*.esq)\0*.esq\0All Files (*.*)\0*.*\0\0" };

	static char *szExt[6] = { "dll", "clm", "elt", "spk", "log", "esq"};

	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = hwnd;
	ofn->hInstance = NULL;
	ofn->lpstrFilter = szFilter[filetype];
	ofn->lpstrCustomFilter = NULL;
	ofn->nMaxCustFilter = 0;
	ofn->nFilterIndex = 0;
	ofn->lpstrFile = NULL;
	ofn->nMaxFile = _MAX_PATH;
	ofn->lpstrFileTitle = NULL;
	ofn->nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn->lpstrInitialDir = NULL;
	ofn->lpstrTitle = NULL;
	ofn->Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn->nFileOffset = 0;
	ofn->nFileExtension = 0;
	ofn->lpstrDefExt = szExt[filetype];
	ofn->lCustData = 0L;
	ofn->lpfnHook = NULL;
	ofn->lpTemplateName = NULL;
}


/* a helper function for opening a Windows shell directory selection dialog box */

BOOL
ChooseDirectory(char *chosendir)
{
	BROWSEINFO	bi;
	LPITEMIDLIST lpItem;

	bi.hwndOwner = hwndMainWindow;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = chosendir;
	bi.lpszTitle = "Select a Folder for Storing the Data ";
	bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	lpItem = SHBrowseForFolder(&bi);

	if (lpItem == NULL)
		return FALSE;

	SHGetPathFromIDList(lpItem, chosendir);

	CoTaskMemFree(lpItem);

	return TRUE;
}



