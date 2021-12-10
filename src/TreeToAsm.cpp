#include "Lang.h"

static FILE *AsmFile   = NULL;
static int  IdsNum     = 0;
static Id   *IdsArr    = NULL;
static int  FreeOffset = 0;

static int NodeToAsm   (TNode *node);
static int PrintSt     (TNode *node);
static int PrintDefine (TNode *node);

const char *RES  = "rx"; /* a register for calculations result
                           i.e. function's return value        */

const char *FREE = "fx"; /* a register that holds free space
                            begin position                   */

#define CURR    node
#define LEFT    node->left
#define RIGHT   node->right
#define DATA    node->data
#define TYPE    node->type
#define IDS     IdsArr
#define IDNUM   IdsNum
#define ASM_IDS &IDS, &IDNUM

static void PrintA (const char *msg, ...)
{
    va_list arg;
    va_start (arg, msg);
    vfprintf (AsmFile, msg, arg);
    va_end (arg);
}

static int PrintAssn (TNode *node)
{
    PrintA ("push %s\n", RES);

    int id_pos = FindId (ASM_IDS, LEFT->data);

    if (id_pos >= 0)
    {
        PrintA ("pop [%ld]\n", id_pos);
    }
    else
    {
        PrintA
        (
            "pop [%s+%d]\n", // save value to FREE + OFFSET
            FREE, FreeOffset, FREE, FREE
        );

        // AddId (ASM_IDS, LEFT->data);
        FreeOffset++;
    }

    PrintA ("ret\n");

    return 0;
}

static int PrintSt (TNode *node)
{
    $ int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    if (LEFT)
    {
        $ int lErr = NodeToAsm (LEFT);
        return lErr;
    }

    return OK;
}

static int PrintDefine (TNode *node)
{
    $ assert (CURR);

    TNode *params = LEFT;
    int   initIds = IDNUM;

    $ PrintA ("f%ld:\n", abs(params->left->data));

    for (TNode *curr_param = params->right;
         curr_param->left;
         curr_param = curr_param->left)
    {
        AddId (ASM_IDS, curr_param->right->data);
    }

    PrintSt (RIGHT);

    for (int curr_id = initIds; curr_id < IdsNum; curr_id++)
    {
        IDS[curr_id] = {};
    }

    return 0;
}

static int NodeToAsm (TNode *node)
{
    $ assert (CURR);

    if (TYPE == TYPE_STATEMENT)
    {
        $ return PrintSt (CURR);
    }

    if (HASH_EQ (CURR, DEF))
    {
        $ return PrintDefine (CURR);
    }
    else if (DATA == '=')
    {
        PrintAssn (CURR);
    }
    else
    {
        PrintA ("JABA\n");
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
    IdsNum = 0;

    int err = NodeToAsm (root);

    free (IdsArr);

    return err;
}
