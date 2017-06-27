/*
Closed Loop Experiment Manager

Files.h

(header file for files.c)

*/


int	LoadMEALayoutFile(char * filename, ELECTRODEINFO *electrodeinfoarray, int maxelectrode);

int	LoadActionsFromFile(char *filename);

BOOL SaveActionsToFile(char *filename);

BOOL LoadParametersFromIniFile(char *inifilename);

void CreateDefaultTextMessageFileName(char *fname);

BOOL AppendTextMessageToFile(char *filename, int length, PSTR buf);