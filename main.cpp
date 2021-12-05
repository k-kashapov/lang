#include "Lang.h"

int main (int argc, const char **argv)
{
    Config io_config = {};
    GetArgs (argc, argv, &io_config);

    char *code = read_file (io_config.input_file);
    if (!code)
    {
        printf ("File empty!\n");
        return -1;
    }

    TNode **nodes = LexicAnalysis (code);

    printf ("DIO!!!!!!!\n");

    free (code);
    free (nodes);

    return 0;
}
