/*
Closed Loop Experiment Manager

DialogBoxes.h

(header file for DialogBoxes.c)

*/



#define		FT_PLUGIN			0
#define		FT_SETTINGS			1
#define		FT_ELECTRODELAYOUT	2
#define		FT_ACTIVITY			3
#define		FT_TEXTLOG			4
#define		FT_TIMEDEXP			5


BOOL CALLBACK     	AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK     	IOParametersSetup(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK		ThresholdSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		TimingSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		DataLogFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		PlaybackFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		ConvertFileSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		DigitalIODisplaySetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK		AddTextEntry(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL				FileOpenDlg (HWND hwnd, int filetype, PSTR FileName, PSTR TitleName);

BOOL				FileSaveDlg (HWND hwnd, int filetype, PSTR FileName, PSTR TitleName);
