#ifndef LOGS_DEF_H
#define LOGS_DEF_H

#include <stdio.h>
#include <stdarg.h>

#define $                                                                       \
        {                                                                       \
            LogMsg ("<HR>\n"                                           \
                    "%s at %s (%d)\n",                                          \
                  __FUNCTION__, __FILE__, __LINE__);                            \
        }

#define $$ LogMsg ("\t%s at %s (%d)\n", __FUNCTION__, __FILE__, __LINE__);

void OpenLogFile (const char *name, const char *mode);
void LogMsg (const char *msg, ...);
void LogErr (const char *msg, ...);
void CloseLogFile (void);

#endif
