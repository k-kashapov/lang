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

    Trans trans   = {};
    trans.IdsArr  = (Id *)     calloc (INIT_IDS_NUM, sizeof (Id));
    trans.FuncArr = (FuncId *) calloc (INIT_IDS_NUM, sizeof (FuncId));
    trans.s       = nodes;

    TNode *res = GetG (&trans);

    OpenGraphFile ("dotInput.dot");
    CreateNodeImage (res, "res.png");
    CloseGraphFile();

    FreeTransTree (res, nodes, nodesNum);

    free (trans.IdsArr);
    free (trans.FuncArr);
    free (code);
    free (nodes);

    return 0;
}
