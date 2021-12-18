#include "TreeAsm.h"

static void PrintA (const char *msg, ...)
{
    $$ for (int tab = 0; tab < Tabs; tab++)
    {
        fprintf (AsmFile, "\t");
    }

    va_list arg;
    va_start (arg, msg);
    vfprintf (AsmFile, msg, arg);
    va_end (arg);
    fprintf (AsmFile, "\n");
}

static int MoveReg (const char *reg, int delta)
{
    Tabs++;
    $$ PrintA ("push %d ; moving ptr %s", delta, reg);
    PrintA ("push %s", reg);
    PrintA ("add");
    PrintA ("pop %s ; ptr %s moved", reg, reg);
    Tabs--;
    return 0;
}

static int PrintCallArgs (TNode *node)
{
    if (!RIGHT || !CURR) return 0;

    int newMemOffs = FreeOffset;
    do
    {
        NodeToAsm (RIGHT);
        PrintA ("pop [%s]", FREE);
        MoveReg (FREE, 1);
        FreeOffset++;
        CURR = LEFT;
    }
    while (CURR);

    int deltaMem = newMemOffs - MemOffset;
    MoveReg (MEM, deltaMem);
    MemOffset  = newMemOffs;
    FreeOffset -= deltaMem;

    return 0;
}

static int PrintCALL (TNode *node)
{
    $ node = RIGHT;

    int initMemOffs  = MemOffset;
    int initFreeOffs = FreeOffset;
    PrintCallArgs (RIGHT);

    PrintA ("call :f%ld ; function %.*s",
            abs (LEFT->data), LEFT->len, LEFT->declared);

    MoveReg (FREE, initFreeOffs - FreeOffset);
    FreeOffset = initFreeOffs;
    MoveReg (MEM, initMemOffs - MemOffset);
    MemOffset = initMemOffs;

    PrintA ("push %s", RES);

    return 0;
}

static int PrintNeg (TNode *node)
{
    $ static int negsNum = 0;
    PrintA ("; !");

    Tabs++;
    PrintA ("push 0");
    NodeToAsm (RIGHT);
    PrintA ("je :%dis0", negsNum);
    PrintA ("push 0");
    PrintA ("jmp :%dnegEnd", negsNum);
    PrintA ("%dis0:", negsNum);
    PrintA ("push 1");
    PrintA ("%dnegEnd:", negsNum);
    Tabs--;

    negsNum++;
    return 0;
}

static int PrintOUT (TNode *node)
{
    $ if (!node) return 1;
    NodeToAsm (RIGHT);
    PrintA ("out ; %.*s", RIGHT->len, RIGHT->declared);
    PrintA ("pop tx ; to trash");
    return 0;
}

static int PrintIF (TNode *node)
{
    $ static int ifNum = 0;

    PrintA ("; if statement");
    Tabs++;
    NodeToAsm (LEFT);

    PrintA ("push 0");
    PrintA ("je :%dfalse", ifNum);

    TNode *decis = RIGHT;

    NodeToAsm (decis->left);
    PrintA ("jmp :%denif", ifNum);

    PrintA ("%dfalse:", ifNum);
    NodeToAsm (decis->right);
    Tabs--;
    PrintA ("%denif:", ifNum);

    ifNum++;
    return 0;
}

static int PrintAssn (TNode *node)
{
    $ int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    int id_pos = FindId (ASM_IDS, LEFT->data, MemOffset);

    if (id_pos >= 0)
    {
        if (LEFT->right->data != 0)
        {
            int offset = (int) LEFT->right->data + MEM_OFS (id_pos);
            PrintA ("pop [%s+%d] ; %.*s", MEM, offset, LEFT->len, LEFT->declared);
        }
        else
        {
            PrintA ("pop [%s+%d] ; %.*s", MEM, MEM_OFS (id_pos), LEFT->len, LEFT->declared);
        }
    }
    else
    {
        LogMsg ("var declared = %.*s; len = %d", LEFT->len, LEFT->declared, LEFT->len);
        PrintA
        (
            "pop [%s+%d] ; declared %.*s", // save value to FREE + OFFSET
            FREE, FreeOffset, LEFT->len, LEFT->declared
        );
        char isConst = 0;
        if (LEFT->left) isConst = 1;

        int len = (int) LEFT->right->data;
        if (len < 1)
        {
            SyntaxErr ("Attempting to allocate array of size %d < 1, %s\n", len, LEFT->declared);
            return ZERO_CAP_DECL;
        }

        AddId (ASM_IDS, LEFT->data, isConst, len, FreeOffset);
        FreeOffset += len;
        MoveReg (FREE, len);
    }

    return 0;
}

#define IF_SERVICE(serv) if (DATA == ServiceNodes[serv]) return Print##serv (CURR);
static int PrintSERV (TNode *node)
{
    $ IF_SERVICE (IF);
    $ IF_SERVICE (DEF);
    $ IF_SERVICE (RET);
    $ IF_SERVICE (OUT);
    $ IF_SERVICE (CALL);

    return 0;
}
#undef IF_SERVICE

#define OP_CASE(op, act)                                                        \
    case op:                                                                    \
        NodeToAsm (LEFT);                                                       \
        NodeToAsm (RIGHT);                                                      \
        PrintA (act "");                                                        \
        return 0

#define COMP_CASE(val, action)                                                  \
    case val:                                                                   \
        PrintA ("; " action);                                                 \
        Tabs++;                                                                 \
        NodeToAsm (LEFT);                                                       \
        NodeToAsm (RIGHT);                                                      \
        PrintA (action " :%dcmp", cmpNum);                                      \
        PrintA ("push 0");                                                      \
        PrintA ("jmp :%dcmpEnd", cmpNum);                                       \
        PrintA ("%dcmp:", cmpNum);                                              \
        PrintA ("push 1");                                                      \
        PrintA ("%dcmpEnd:", cmpNum);                                           \
        Tabs--;                                                                 \
        cmpNum++;                                                               \
        break

static int PrintOP (TNode *node)
{
    $ static int cmpNum = 0;
    switch (node->data)
    {
        case '=':
            return PrintAssn (CURR);
            break;
        case '!':
            return PrintNeg (CURR);
            break;
        COMP_CASE (EE, "je");
        COMP_CASE (AE, "jae");
        COMP_CASE (BE, "jbe");
        COMP_CASE (NE, "jn");
        OP_CASE ('+', "add");
        OP_CASE ('-', "sub");
        OP_CASE ('*', "mul");
        OP_CASE ('/', "div");
        default:
            break;
    }

    return 0;
}
#undef OP_CASE
#undef COMP_CASE

static int PrintConst (TNode *node)
{
    PrintA ("push %d ; const value", DATA);

    return 0;
}

static int PrintRET (TNode *node)
{
    $ if (!RIGHT)
    {
        PrintA ("push 0");
        SAVE();
        return 0;
    }

    int rErr = NodeToAsm (RIGHT);
    if (rErr) return rErr;

    SAVE();
    PrintA ("ret");

    return 0;
}

static int PrintDEF (TNode *node)
{
    $ assert (CURR);

    TNode *params = LEFT;
    int   initIds = IDNUM;

    long hash = abs(params->left->data);
    $ PrintA ("f%ld:", hash);
    Tabs++;

    for (TNode *curr_param = params->right;
         curr_param && curr_param->left;
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
    if (hash == MAIN_HASH)
        PrintA ("hlt");

    return 0;
}

static int PrintID (TNode *node)
{
    PrintA ("; %.*s", LEN, DECL);

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
    int id_pos = FindId (ASM_IDS, DATA, MemOffset);

    if (id_pos >= 0)
    {
        printf ("right = %p; decl = %.*s\n", RIGHT, LEN, DECL);
        if (!RIGHT || RIGHT->data == 0)
        {
            PrintA ("push [%s+%ld] ; %.*s", MEM, MEM_OFS (id_pos), LEN, DECL);
        }
        else
        {
            int offset = MEM_OFS (id_pos) + (int) RIGHT->data;
            PrintA ("push [%s+%ld] ; %.*s", MEM, offset, LEN, DECL);
        }
    }
    else
    {
        SyntaxErr ("Undeclared variable used: %.*s\n", LEN, DECL);
        return UNDECLARED;
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

    PrintA ("");

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
        case TYPE_SERVICE:
            $ return PrintSERV (CURR);
        default:
            break;
    }

    PrintA ("; node (%.*s) of type %d", LEN, DECL, node->type);

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

    PrintA ("push 0");
    PrintA ("pop %s ; available memory offset", MEM);
    PrintA ("push 0");
    PrintA ("pop %s ; free memory offset", FREE);
    PrintA ("push 0");
    PrintA ("pop %s ; call result\n", RES);
    PrintA ("call :f1058 ; main\n");

    int err = NodeToAsm (root);
    if (err) printf ("Node to asm: errors occured: %d", err);

    free (IdsArr);

    return err;
}
