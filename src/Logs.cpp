#include "Logs.h"

static FILE *LogFile = stderr;

void OpenLogFile (const char *name, const char *mode)
{
    LogFile = fopen (name, mode);
    if (!LogFile)
    {
        printf ("ERROR: UNABLE TO OPEN LOG FILE!!!\n"
                "name = %s;\nmode = %s;\n", name, mode);
        LogFile = stderr;
        return;
    }

    fprintf (LogFile, "<html>\n"
                      "<head>\n"
                      "<meta charset=\"utf-8\">\n"
                      "</head>\n"
                      "<body>\n<pre>\n");
    return;
}

void CloseLogFile (void)
{
    if (LogFile != stderr)
    {
        fprintf (LogFile, "</pre></body></html>\n");
        fclose (LogFile);
        LogFile = stderr;
    }
}

void LogMsg (const char *msg, ...)
{
    va_list arg;
    va_start (arg, msg);
    vfprintf (LogFile, msg, arg);
    va_end (arg);

    fflush (LogFile);
}


void LogErr (const char *msg, ...)
{
    fprintf (LogFile, "\n<HR>\n"
                     "ERROR: ");
    va_list arg;
    va_start (arg, msg);
    vfprintf (LogFile, msg, arg);
    va_end (arg);

    fprintf (LogFile, "\n<HR>\n");
    fflush (LogFile);
}
