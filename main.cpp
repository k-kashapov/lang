#include "Lang.h"

int main (int argc, const char **argv)
{
    OpenLogFile ("LangLog.html", "wt");
    Config io_config = {};
    GetArgs (argc, argv, &io_config);

    TNode *res = NULL;

    if (io_config.settings & READ_BASE)
    {
        char *buf = NULL;
        res = BuildTreeFromBase (&io_config, &buf);

        if (io_config.settings & REVERSE)
            Reverse (res);

        Translate (res, "asm.txt");

        DestructNode (res);
        free (buf);
    }
    else
    {
        char *code = read_file (io_config.input_file);
        if (!code)
        {
            printf ("File empty!\n");
            return -1;
        }

        int nodesNum = 0;
        TNode **nodes = LexicAnalysis (code, &nodesNum);

        Trans trans   = {};
        trans.IdsArr  = (Id *)     calloc (INIT_IDS_NUM, sizeof (Id));
        trans.FuncArr = (FuncId *) calloc (INIT_IDS_NUM, sizeof (FuncId));
        trans.tok     = nodes;
        trans.ce      = 0;

        int comp_err = 0;
        res = GetG (&trans, &comp_err);

        if (comp_err)
        {
            FreeTransTree (res, nodes, nodesNum);
            free (trans.IdsArr);
            free (trans.FuncArr);
            free (code);
            free (nodes);
            CloseLogFile ();
            return 0;
        }

        CreateNodeImage (res, "res.png");
        SaveNode (res, "base.txt");
        Translate (res, "asm.txt");

        FreeTransTree (res, nodes, nodesNum);
        CloseLogFile ();

        free (trans.IdsArr);
        free (trans.FuncArr);
        free (code);
        free (nodes);
    }

    return 0;
}
