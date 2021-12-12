#include "Lang.h"

int main (int argc, const char **argv)
{
    OpenLogFile ("LangLog.html", "wt");
    Config io_config = {};
    GetArgs (argc, argv, &io_config);



    CloseLogFile ();
    return 0;
}
