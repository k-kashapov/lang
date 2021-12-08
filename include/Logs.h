#include <stdio.h>
#include <stdarg.h>

#define $ LogMsg ("%s (%d)\n", __FUNCTION__, __LINE__);

void OpenLogFile (const char *name, const char *mode);

void LogMsg (const char *msg, ...);

void LogErr (const char *msg, ...);

void CloseLogFile (void);
