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

    Trans trans = {};
    trans.s = code;
    trans.IdsArr = (Id *) calloc (INIT_IDS_NUM, sizeof (Id));

    int val = GetG(&trans);

    printf ("res = %d\n", val);

    free (trans.IdsArr);

    return 0;
}
