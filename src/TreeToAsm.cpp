#include "Logs.h"
#include "Lang.h"

static FILE *AsmFile = NULL;
static int  IdsNum   = 0;
static Id   *IdsArr  = NULL;

static int NodeToAsm (TNode *node);
static int PrintSt (TNode *node);
static int PrintDefine (TNode *node);

static int PrintSt (TNode *node)
{
    $ int rErr = NodeToAsm (node->right);
    if (rErr) return rErr;

    if (node->left)
    {
        $ int lErr = NodeToAsm (node->left);
        return lErr;
    }

    return OK;
}

static int PrintDefine (TNode *node)
{
    $ assert (node);

    TNode *params = node->left;
    fprintf (AsmFile, "func%ld:\n", abs(params->left->data));

    int initIds = IdsNum;
    for (TNode *curr_param = params->right;
         curr_param->left;
         curr_param = curr_param->left)
    {
        Id new_id        = {};
        new_id.hash      = curr_param->data;
        IdsArr[IdsNum++] = new_id;
    }


    for (int curr_id = initIds; curr_id < IdsNum; curr_id++)
    {
        IdsArr[curr_id] = {};
    }

    return 0;
}

static int NodeToAsm (TNode *node)
{
    $ assert (node);

    if (node->type == TYPE_STATEMENT)
    {
        return PrintSt (node);
    }

    if (HASH_EQ (node, DEF))
    {
        PrintDefine (node);
    }
    else
    {
        fprintf (AsmFile, "JABA\n");
    }

    return OK;
}

int Translate (TNode *root, const char *name)
{
    AsmFile = fopen (name, "wt");
    if (!AsmFile)
    {
        LogErr ("Unable to open asm file; name = %s\n", name);
        return OPEN_FILE_FAILED;
    }

    IdsArr = (Id *) calloc (INIT_IDS_NUM, sizeof (Id));

    int err = NodeToAsm (root);

    return err;
}
