/*
Closed Loop Experiment Manager

Files.c
--------

Utility routines for loading and saving settings from and to files

Public functions:
----------------
LoadMEALayoutFile()
LoadActionsFromFile()
SaveActionsToFile()
LoadParametersFromIniFile()
CreateDefaultTextMessageFileName()
AppendTextMessageToFile()
*/


#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#pragma hdrstop

#include "CLEM.h"
#include "GraphWindow.h"
#include "CoreFunction/GlobalVar.h"
#include <stdBOOL.h>
#include "files.h"

#pragma warning ( disable : 4100)

/* local function prototypes */

BOOL GetValueFromIniFile(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, LPCTSTR lpFileName, BOOL override_noinifile);


/* load an electrode layout file;
   essentially a text file in which each line follows the following format:
   <channel>	<X position>	<Y position>	<description (upto 100 characters)>
   function returns the number of valid electrodes loaded
  */

int
LoadMEALayoutFile(char * filename, ELECTRODEINFO *electrodeinfoarray, int maxelectrode)
{

	FILE 			*fin;
	char			ch;
	int 			n, i, k, valid_copied, arraysize;
	char			buf[200], ename[200];
	LPVOID			hHeap;
	ELECTRODEINFO	*e;

	/* open the file */

	i = fopen_s (&fin, filename, "rt");
	if (i != 0)
		return 0;

	/* allocate memory for a scratch electrode info array */

	arraysize = sizeof(ELECTRODEINFO) * N_AD_CHANNELS;
	hHeap = GetProcessHeap();
	if ((e = (ELECTRODEINFO *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, arraysize)) == NULL)
	{
		if (fin != NULL)
			fclose(fin);
		return 0;
	}

	/* get the entries*/

	n = 0;
	i = 0;

	do
	{
		ch = EOF;
		if (fin != NULL)
			ch = (char) getc(fin);
		if (ch == EOF)
			break;

		buf[i] = ch;
		i++;
		if (i >= 200)
		{
			n = -1;
			break;
		}
		if (ch == '\n')
		{
			k = sscanf_s(buf, "%d %d %d %[^'\n']s", &(e[n].channel), &(e[n].xpos), &(e[n].ypos), ename, 200);
			ename[199] = 0;
			strcpy_s(e[n].name, 100, ename);
			e[n].name[99] = 0;
			i = 0;
			n++;
			memset(buf, 0, 200);
			memset(ename, 0, 200);
		}
	} while (ch != EOF);

	/* if all is well, copy the data to the global array */

	if (n > 0)
	{
		/* first, mark all channels as unused */

		for (i = 0; i < N_AD_CHANNELS; ++i)
			electrodeinfoarray[i].inuse = FALSE;

		/* now copy all data, marking each copied channel as active */

		valid_copied = 0;
		for (k = 0; k < n; ++k)
		{
			i = e[k].channel - 1;
			if (i >= 0 && i < N_AD_CHANNELS)
			{
				electrodeinfoarray[i].channel = e[k].channel;
				electrodeinfoarray[i].xpos = e[k].xpos;
				electrodeinfoarray[i].ypos = e[k].ypos;
				electrodeinfoarray[i].inuse = TRUE;
				strcpy_s(electrodeinfoarray[i].name, 100, e[k].name);
				valid_copied++;
			}
		}
	}
	else
		valid_copied = -1;

	if (fin != NULL)
		fclose(fin);

	HeapFree(hHeap, 0, (LPVOID)e);

	return valid_copied;
}


/* load an experiment action sequence from a file */

int
LoadActionsFromFile(char *filename)
{
	FILE			*fin;
	int				tblsize, i, n, k;
	char			buf[512], ch;
	ACTIONENTRY		*act_tbl;

	/* make a scratch copy of the action sequence table */

	tblsize = sizeof(ACTIONENTRY) * MAXACTIONS;
	act_tbl = (ACTIONENTRY *) malloc(tblsize);
	if (act_tbl == NULL)
		return 0;

	/* open the file */

	i = fopen_s (&fin, filename, "rt");
	if (i != 0)
		return 0;

	/* read the action sequence */

	n = 0;
	do
	{
		ch = EOF;
		if (fin != NULL)
			ch = (char) getc(fin);
		if (ch == EOF)
			break;

		buf[i] = ch;
		i++;
		if (i >= 512)
		{
			n = -1;
			break;
		}
		if (ch == '\n')
		{
			if (buf[0] != '%')			/* ignore lines that start with a % sign - allowing comments */
			{
				memset(act_tbl[n].argstring, 0, 256);
				k = sscanf_s(buf, "%d %lf %[^'\n']s", &(act_tbl[n].action), &act_tbl[n].duration, act_tbl[n].argstring, 256);
				act_tbl[n].active = TRUE;
				n++;
				if(n == MAXACTIONS)
					break;
			}
			memset(buf, 0, 512);
			i = 0;
		}
	} while (ch != EOF);

	/* store the action entries into the global table and clean up */

	if (fin != NULL)
		fclose(fin);
	
	memcpy_s(ActionTable, tblsize, act_tbl, tblsize);
	ActionEntries = n;

	free(act_tbl);

	return n;
}


/* save an experiment action sequence to a file */

BOOL
SaveActionsToFile(char *filename)
{
	FILE         *fout;
	int			 byteswritten, i, length;
	char		 buf[512];

	/* open the file */

	i = fopen_s (&fout, filename, "wt");
	if (i != 0)
		return FALSE;

	/* write the action sequence */

	for (i = 0; i < ActionEntries; ++i)
	{
		length = sprintf_s(buf, 512, "%d\t%f\t%s\n", ActionTable[i].action, ActionTable[i].duration, ActionTable[i].argstring);
		byteswritten = 0;
		if (fout != NULL)
			byteswritten = (int) fwrite(buf, 1, length, fout);
		if (byteswritten != length)
		{
			if (fout != NULL)
				fclose(fout);
			return FALSE;
		}
	}
	
	/* clean up */

	if (fout != NULL)
		fclose(fout);

	return TRUE;
}


/* load  system parameters from a file that follows the legacy Windows INI file format. 
   Warn the user if the file does not exist or an entry was not found.
   Uses the helper function GetValueFromIniFile (see below, helper functions section) */

BOOL
LoadParametersFromIniFile(char *inifilename)
{
	char	buf[50];
	char	full_inifilename[350];
	char	messagestr[500];
	int		response;
	BOOL	override_noinifile = FALSE;

	if (inifilename == NULL)
	{
		strcpy_s(full_inifilename, 350, AppDirectory);
		strcat_s(full_inifilename, 350, "\\CLEM default.ini");
	}
	else
		strcpy_s(full_inifilename, 350, inifilename);

	if (!PathFileExists(full_inifilename))
	{
		sprintf_s(messagestr, 200, "INI file %s not found; Continue anyway?", full_inifilename);
		response = MessageBox(NULL, messagestr, "Warning", MB_ICONASTERISK | MB_OKCANCEL);
		if (response == IDCANCEL)
			return FALSE;
		override_noinifile = TRUE;
	}
	
	if (!GetValueFromIniFile("DAQ", "DAQModel", "", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.BoardModelString, 30, buf);

	if (!GetValueFromIniFile ("DAQ", "VoltageFullScale", "0.5", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.VoltageFullScale = atof(buf);

	if (!GetValueFromIniFile ("DAQ", "DAQSampleRate", "16000", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.BoardSampleRate = atoi(buf);

	if (!GetValueFromIniFile ("DAQ", "DAQOutputFreq", "16000", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.BoardAOFreq = atoi(buf);

	GlobalRecParms.SampleRate = GlobalRecParms.BoardSampleRate;

	if (!GetValueFromIniFile("Memory", "VoltageHistoryStorageDuration", "1", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.ContVoltageStorageDuration = atoi(buf);

	if (!GetValueFromIniFile("Memory", "SpikeHistoryLength", "50000", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeHistoryLength = atoi(buf);

	if (!GetValueFromIniFile("DAQ", "InputBufferSizePerChannel", "64", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.InputBufferSizePerChannel = atoi(buf);

	if (!GetValueFromIniFile("DAQ", "OutputBufferSizePerChannel", "64", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.OutputBufferSizePerChannel = atoi(buf);

	if (!GetValueFromIniFile("DAQ", "SpikeRefractoryPeriodMicrosecond", "1100", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeRefractoryPeriodMicrosecond = atoi(buf);

	if (!GetValueFromIniFile("DAQ", "DAQCardPath", "pwrdaq://Dev0", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.DAQCardPath, 30, buf);

	if (!GetValueFromIniFile("DAQ", "DAQAnalogInputChannels", "0:63", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.DAQAnalogInputChannels, 20, buf);

	if (!GetValueFromIniFile("DAQ", "DAQAnalogOutputChannels", "0:1", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.DAQAnalogOutputChannels, 20, buf);

	if (!GetValueFromIniFile("Display", "SpikeTraceDurationMsec", "10", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeTraceDuration = atoi(buf);

	if (!GetValueFromIniFile("Display", "SpikeTracePreTriggerMsec", "2", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeTracePreTrigger = atoi(buf);

	if (!GetValueFromIniFile("Display", "SpikesToDisplay", "5", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikesToDisplay = atoi(buf);

	if (!GetValueFromIniFile("Display", "TraceGraphTimeSliceMsec", "5", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.TraceGraphTimeSlice = atof(buf);

	if (!GetValueFromIniFile("Display", "SpikeRasterTimeSliceSec", "60", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeRasterTimeSlice = atof(buf);

	if (!GetValueFromIniFile("Display", "SpikeHistoMaxVal", "70", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.SpikeHistoMaxVal = atoi(buf);

	if (!GetValueFromIniFile("Display", "DisplayVoltageFullScale", "2.0", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.DisplayVoltageFullScale = atof(buf);

	if (!GetValueFromIniFile("Thresholds", "ThresholdRMSMultFactor", "7.0", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.ThresholdRMSMultFactor = (float)atof(buf);

	if (!GetValueFromIniFile("Thresholds", "ThresholdRMSMeasureDurationSec", "1", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.ThresholdRMSMeasureDuration = (float)atof(buf);

	if (!GetValueFromIniFile("Thresholds", "GlobalThreshold", "-0.1", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.GlobalThreshold = (float)atof(buf);

	if (!GetValueFromIniFile("Thresholds", "LastChannelToInclude ", "63", buf, full_inifilename, override_noinifile))
		return FALSE;
	GlobalRecParms.LastChannelToInclude = atoi(buf);

	if (!GetValueFromIniFile("Extensions", "SpikesFile", ".spk", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.SpikesFileExt, 10, buf);

	if (!GetValueFromIniFile("Extensions", "AnalogInputFile", ".vlt", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.AnalogInputFileExt, 10, buf);

	if (!GetValueFromIniFile("Extensions", "SpikeTimeFile", ".spt", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.SpikeTimesExt, 10, buf);

	if (!GetValueFromIniFile("Extensions", "AnalogOutFile", ".aot", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.AnalogOutFileExt, 10, buf);

	if (!GetValueFromIniFile("Extensions", "DigitalIOFile", ".dig", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.DigitalIOFileExt, 10, buf);

	if (!GetValueFromIniFile("Extensions", "UserDataFile", ".udt", buf, full_inifilename, override_noinifile))
		return FALSE;
	strcpy_s(GlobalRecParms.UserDataFileExt, 10, buf);

	return TRUE;
}



/* create a default file name for the Text Message Log on the users desktop */

void 
CreateDefaultTextMessageFileName(char *fname)
{
	HRESULT			hr;
	PWSTR			p;
	SYSTEMTIME		st;
	char			path[MAX_PATH];
	char			defname[100];
	int				n = 1;
		
	/* Get the path to the users desktop and convert to ASCII */

	hr = SHGetKnownFolderPath (&FOLDERID_Desktop, 0, NULL, &p);
	if (hr != S_OK)
	{
		MessageBoxA(hwndMainWindow, "Failed to obtain path to users desktop", "Error", MB_ICONSTOP | MB_OK);
		strcpy_s(path, MAX_PATH, "C:\\");
	}
	else
	{
		n = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)p, -1, path, MAX_PATH, NULL, NULL);
		CoTaskMemFree((LPVOID)p);
	}
	
	if (n < 1)
	{
		MessageBox(NULL, "Failed to obtain path to user's desktop", "Error", MB_ICONSTOP | MB_OK);
		return;
	}

	/* create a filename based on the current date and time */

	GetLocalTime(&st);
	sprintf_s(defname, 100, "\\MessageLog_%04u_%02u_%02u_%02u_%02u.log", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	strcat_s(path, MAX_PATH, defname);

	strcpy_s(fname, MAX_PATH, path);	
}


/* append a single text message to the text messages log file */

BOOL
AppendTextMessageToFile(char *filename, int length, PSTR buf)
{
	FILE         *fout;
	int			 byteswritten = 0, i;

	/* open the file in append mode */

	i = fopen_s(&fout, filename, "a+b");
	if (i != 0)
		return FALSE;

	/* write the file data */

	if (fout != NULL)
		byteswritten = (int)fwrite(buf, 1, length, fout);
	if (byteswritten != length)
	{
		if (fout != NULL)
			fclose(fout);
		return FALSE;
	}

	/* clean up */

	if (fout != NULL)
		fclose(fout);

	return TRUE;
}


/* ---------------------------------------- helper functions ----------------------------------------------*/

/* get a value from the INI file*/

BOOL
GetValueFromIniFile(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, LPCTSTR lpFileName, BOOL override_noinifile)
{
	char	buf[50];
	char	messagestr[200];
	int		i, response;

	i = GetPrivateProfileString(lpAppName, lpKeyName, "", buf, 50, lpFileName);
	if (i == 0 && !override_noinifile)
	{
		sprintf_s(messagestr, 200, "Could not load a value for the key %s; Continue?", lpKeyName);
		response = MessageBox(NULL, messagestr, "Warning", MB_ICONASTERISK | MB_OKCANCEL);
		if (response == IDCANCEL)
			return FALSE;
		strcpy_s(lpReturnedString, 50, lpDefault);
	}
	else
	{
		strcpy_s(lpReturnedString, 50, buf);
	}
	return TRUE;
}

