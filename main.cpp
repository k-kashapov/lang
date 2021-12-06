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

    int nodesNum  = 0;
    TNode **nodes = LexicAnalysis (code, &nodesNum);

    Trans trans  = {};
    trans.IdsArr = (Id *) calloc (INIT_IDS_NUM, sizeof (Id));
    trans.s      = nodes;

    TNode *res = GetG (&trans);

    OpenGraphFile ("dotInput.dot");
    CreateNodeImage (res, "res.png");
    CloseGraphFile();

    printf ("\nDIO!!!!!!!\n");

    FreeTransTree (res, nodes, nodesNum);

    free (trans.IdsArr);
    free (code);
    free (nodes);

    return 0;
}
