#include "Lang.h"

static FILE *AsmFile   = NULL;
static int  IdsNum     = 0;
static Id   *IdsArr    = NULL;
static int  FreeOffset = 0;
static int  Tabs       = 0;

static int NodeToAsm   (TNode *node);
static int PrintSt     (TNode *node);
static int PrintVar    (TNode *node);
static int PrintID     (TNode *node);
static int PrintDefine (TNode *node);
static int PrintConst  (TNode *node);
static int PrintOP     (TNode *node);
static void PrintA (const char *msg, ...);
static int PrintRet (TNode *node);

const char *RES  = "rx"; /* a register for calculations result
                           i.e. function's return value        */

const char *FREE = "fx"; /* a register that holds free space
                            begin position                   */
#define CURR    node
#define LEFT    node->left
#define RIGHT   node->right
#define DATA    node->data
#define DECL    node->declared
#define LEN     node->len
#define TYPE    node->type
#define IDS     IdsArr
#define IDNUM   IdsNum
#define ASM_IDS &IDS, &IDNUM
#define SAVE()  PrintA ("pop %s\n", RES)

static void PrintA (const char *msg, ...)
{
    for (int tab = 0; tab < Tabs; tab++)
    {
        fputc ('\t', AsmFile);
    }

    va_list arg;
    va_start (arg, msg);
    vfprintf (AsmFile, msg, arg);
    va_end (arg);
}

static int PrintOP (TNode *node)
{
    int lErr = NodeToAsm (LEFT);
    if (lErr) return lErr;

    int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    return 0;
}

static int PrintConst (TNode *node)
{
    PrintA ("push %d ; const value\n", DATA);
    SAVE();

    return 0;
}

static int PrintRet (TNode *node)
{
    if (!RIGHT)
    {
        PrintA ("push 0");
        PrintA ("pop %s\n", RES);
        SAVE();
        return 0;
    }

    int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    PrintA ("pop %s\n", RES);
    SAVE();

    return 0;
}

static int PrintDefine (TNode *node)
{
    $ assert (CURR);

    TNode *params = LEFT;
    int   initIds = IDNUM;

    $ PrintA ("f%ld:\n", abs(params->left->data));
    Tabs++;

    for (TNode *curr_param = params->right;
         curr_param->left;
         curr_param = curr_param->left)
    {
        AddId (ASM_IDS, curr_param->right->data);
    }

    PrintSt (RIGHT);

    Tabs--;

    for (int curr_id = initIds; curr_id < IdsNum; curr_id++)
    {
        IDS[curr_id] = {};
    }

    return 0;
}

static int PrintID (TNode *node)
{
    if (HASH_EQ (CURR, DEF))
    {
        $ return PrintDefine (CURR);
    }
    else if (HASH_EQ (CURR, RET))
    {
        $ return PrintRet (CURR);
    }

    PrintA ("; %.*s\n", LEN, DECL);

    if (LEFT)
    {
        $ int lErr = NodeToAsm (LEFT);
        if (lErr) return lErr;
    }

    if (RIGHT)
    {
        $ int rErr = NodeToAsm (RIGHT);
        if (rErr) return rErr;
    }

    return 0;
}

static int PrintVar (TNode *node)
{
    int id_pos = FindId (ASM_IDS, DATA);

    if (id_pos >= 0)
    {
        PrintA ("push [%ld] ; %.*s\n", LEN, DECL);
        SAVE();
    }
    else
    {
        SyntaxErr ("Undeclared variable used: %.*s\n", LEN, DECL);
        return UNDECLARED;
    }

    return 0;
}

static int PrintAssn (TNode *node)
{
    int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    PrintA ("push %s\n", RES);

    int id_pos = FindId (ASM_IDS, LEFT->data);

    if (id_pos >= 0)
    {
        PrintA ("pop [%ld]\n", id_pos);
    }
    else
    {
        LogMsg ("node declared = %.*s; len = %d\n", LEFT->len, LEFT->declared, LEN);
        PrintA
        (
            "pop [%s+%d] ; %.*s\n", // save value to FREE + OFFSET
            FREE, FreeOffset, LEFT->len, LEFT->declared
        );

        AddId (ASM_IDS, LEFT->data);
        FreeOffset++;
    }

    return 0;
}

static int PrintSt (TNode *node)
{
    if (LEFT)
    {
        $ int lErr = NodeToAsm (LEFT);
        if (lErr) return lErr;
    }

    $ int rErr = NodeToAsm (RIGHT);
    return rErr;
}

static int NodeToAsm (TNode *node)
{
    $ assert (CURR);

    switch (TYPE)
    {
        case TYPE_STATEMENT:
            $ return PrintSt (CURR);
        case TYPE_VAR:
            $ return PrintVar (CURR);
        case TYPE_ID:
            $ return PrintID (CURR);
        case TYPE_CONST:
            $ return PrintConst (CURR);
        case TYPE_OP:
            $ return PrintOP (CURR);
        default:
            break;
    }

    if (DATA == '=')
    {
        $ return PrintAssn (CURR);
    }
    else
    {
        PrintA ("; node (%.*s) of type %d\n", LEN, DECL, node->type);
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
