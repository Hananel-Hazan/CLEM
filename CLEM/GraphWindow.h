/*
Closed Loop Experiment Manager

GraphWindow.h

(Header file for GraphWindow.c)

*/



BOOL				InitializeCLEMGraphics (void);

void				CleanupCLEMGraphics (void);

LRESULT CALLBACK    TraceGraphWndProc (HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK    TraceSingleWndProc (HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK    SpikeRasterWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK    SpikeHistogramWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK	DIOGraphWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK    UserGraphWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK    MEALayoutWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

TIMERPROC			RenderTimerProc (HWND hWnd,  INT nMsg, UINT nIDEvent, DWORD dwTime);

void				RenderNewTextMessages(HWND hEdit);

BOOL				ResetGraphicsData(void);

void				ClearDisplayedSpikes(void);


