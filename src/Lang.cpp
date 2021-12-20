#include "Lang.h"

static int COMP_ERR = 0;

int SyntaxErr (const char *msg, ...)
{
    COMP_ERR = 1;
    printf ("SYNTAX ERROR: ");

    va_list arg = {};
    va_start (arg, msg);
    vprintf (msg, arg);
    va_end (arg);

    return 0;
}

static void MovePtr (Trans *trans)
{
    trans->tok++;
}

static TNode *GetTok (Trans *trans)
{
    return *trans->tok;
}

static int CheckTok (Trans *trans, const char *req)
{
    int64_t hash = SimpleHash (req, (int) strlen (req));
    return GetTok (trans)->data == hash;
}

#define PHRASE(words, num, text, ph_hash)                                       \
    case num:                                                                   \
    {                                                                           \
        $$ int64_t hash = 0;                                                    \
        TNode *got = GetTok (trans);                                            \
        for (int wd = 0; wd < words; wd++)                                      \
        {                                                                       \
            hash += GetTok (trans)->data;                                       \
            MovePtr (trans);                                                    \
        }                                                                       \
        if (hash == ph_hash)                                                    \
            return 1;                                                           \
        else                                                                    \
        {                                                                       \
            SyntaxErr ("Invalid syntax. Expected: " text                        \
                       "\ngot: %.*s\n", got->len, got->declared);               \
            LogMsg ("Invalid syntax code: %s\n", got->declared);                \
            return 0;                                                           \
        }                                                                       \
    }
static int Require_ (Trans *trans, int req)
{
    switch (req)
    {
        #include "Phrases.h"
        default:
            break;
    }

    LogMsg ("Invalid requirement: %d\n", req);
    return 0;
}
#undef PHRASE

#define Require(req) if (!Require_ (trans, req)) {COMP_ERR = 1; return root;}

#define LOCAL_IDS(CODE)                                                         \
    initIds = trans->IdsNum;                                                    \
    CODE                                                                        \
    $ RmId (TRANS_IDS, trans->IdsNum - initIds);

static TNode *CreateNodeWithStr (const char *id, int type = TYPE_ID)
{
    $$ int len   = (int) strlen (id);
    TNode *node = CreateNode (SimpleHash (id, len), type, id);
    node->len   = len;

    return node;
}

TNode *GetSt (Trans *trans, const char *end_cond)
{
    if (CheckTok (trans, end_cond)) return NULL;

    $ TNode *stmt = ST (NULL, GetOP (trans));
    TNode *curr = stmt;

    while (!CheckTok (trans, end_cond) && curr)
    {
        $ curr = ST (curr, GetOP (trans));
        if (!curr->right) break;
    }

    return curr;
}

TNode *GetG (Trans *trans, int *ce)
{
    TNode *root = NULL;

    Require (INIT);
    root = GetSt (trans, "А");
    *ce = COMP_ERR;

    Require (PROG_END);
    return root;
}

TNode *GetID (Trans *trans)
{
    TNode *root = GetTok (trans);

    for (int func_id = 0; func_id < UnaryNum; func_id++)
    {
        if (root->data == UnaryFuncs[func_id])
        {
            TNode *action = GetTok (trans);
            action->type  = TYPE_UNARY;
            MovePtr (trans);
            action->right = GetP (trans);
            return action;
        }
    }

    int idNum = FindId (TRANS_IDS, root->data);

    if (idNum >= 0)
    {
        MovePtr (trans);
        root->type  = TYPE_VAR;
        if (CheckTok (trans, "("))
        {
            Require (IDX);
            root->right = GetN (trans);
            Require (IDXEND);
        }
        else
        {
            root->right = CreateNode (0, TYPE_CONST);
        }
        return root;
    }

    SyntaxErr ("Get ID: Using an undeclared variable: %.*s\n",
               root->len,
               root->declared);
    LogMsg ("SyntaxErr: %s\n", root->declared);
    MovePtr (trans);
    return root;
}

TNode *GetN (Trans *trans)
{
    $$ TNode *node = GetTok (trans);
    MovePtr (trans);

    if (node->type != TYPE_CONST)
    {
        SyntaxErr ("Invalid type: expected CONST, got %.*s (%d)\n",
                   node->len, node->declared, node->type);
    }

    return node;
}

TNode *GetP (Trans *trans)
{
    TNode *root = NULL;
    if (CheckTok (trans, "Биба")) // Left bracket '('
    {
        MovePtr (trans);
        TNode *node = GetE (trans);

        MovePtr (trans); // Right bracket
        return node;
    }
    else if (CheckTok (trans, "Анекдот"))
    {
        return GetCall (trans);
    }
    else if (CheckTok (trans, "ввод"))
    {
        Require (IN_PH);
        return CreateNodeWithStr ("scan", TYPE_SERVICE);
    }
    else if (GetTok (trans)->type == TYPE_ID)
    {
        return GetID (trans);
    }
    else
    {
        return GetN (trans);
    }
}

TNode *GetNeg (Trans *trans)
{
    $$ TNode *root = GetTok (trans);

    if (!root) return NULL;

    if (CheckTok (trans, "Нифига"))
    {
        Require (NEG);
        root = CreateNode ('!', TYPE_OP, root->declared, NULL, GetP (trans));
        root->len = 1;
    }
    else
    {
        root = GetP (trans);
    }

    return root;
}

TNode *GetPow (Trans *trans)
{
    $$ TNode *root = GetNeg (trans);

    if (!root) return NULL;

    if (CheckTok (trans, "^"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetNeg (trans);
        action->left  = root;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "^"))
        {
            TNode *new_action = GetTok (trans);
            MovePtr (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetNeg (trans);
        }

        root = action;
    }

    return root;
}

TNode *GetT (Trans *trans)
{
    $$ TNode *root = GetPow (trans);

    if (!root) return NULL;

    if (CheckTok (trans, "дофига") ||
        CheckTok (trans, "/"))
    {
        TNode *action = GetTok (trans);
        action->type  = TYPE_OP;
        if (action->data != '/') action->data = '*';
        MovePtr (trans);

        TNode *rVal   = GetPow (trans);
        action->left  = root;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "дофига") ||
               CheckTok (trans, "/"))
        {
            TNode *new_action = GetTok (trans);
            MovePtr (trans);
            if (new_action->data != '/') new_action->data = '*';
            new_action->type  = TYPE_OP;
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetPow (trans);
        }

        root = action;
    }

    return root;
}

TNode *GetSum (Trans *trans)
{
    $$ TNode *root = GetT (trans);

    if (!root) return NULL;

    if (CheckTok (trans, "+") ||
        CheckTok (trans, "-"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetT (trans);
        action->left  = root;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "+") ||
               CheckTok (trans, "-"))
        {
            TNode *new_action = GetTok (trans);
            MovePtr (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetT (trans);
        }

        root = action;
    }

    return root;
}

TNode *GetCmp (Trans *trans)
{
    $$ TNode *root = GetSum (trans);

    $$ if (!root) return NULL;

    if (CheckTok (trans, ">")  ||
        CheckTok (trans, "<")  ||
        CheckTok (trans, ">=") ||
        CheckTok (trans, "<=") ||
        CheckTok (trans, "==") ||
        CheckTok (trans, "!="))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetSum (trans);
        action->left  = root;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, ">")  ||
               CheckTok (trans, "<")  ||
               CheckTok (trans, ">=") ||
               CheckTok (trans, "<=") ||
               CheckTok (trans, "==") ||
               CheckTok (trans, "!="))
        {
            TNode *new_action = GetTok (trans);
            MovePtr (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetSum (trans);
        }

        root = action;
    }

    return root;
}

TNode *GetE (Trans *trans)
{
    $$ TNode *root = GetCmp (trans);

    if (!root) return NULL;

    if (CheckTok (trans, "||") ||
        CheckTok (trans, "&&"))
    {
        TNode *action = GetTok (trans);
        action->type  = TYPE_OP;
        MovePtr (trans);

        TNode *rVal   = GetCmp (trans);
        action->left  = root;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "||") ||
               CheckTok (trans, "&&"))
        {
            TNode *new_action = GetTok (trans);
            MovePtr (trans);
            new_action->type  = TYPE_OP;
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetCmp (trans);
        }

        root = action;
    }

    return root;
}

TNode *GetRet (Trans *trans)
{
    TNode *root = CreateNodeWithStr ("return", TYPE_SERVICE);

    Require (DEFRET);

    TNode *ret_val = NULL;
    if (CheckTok (trans, ","))
    {
        ret_val = CreateNode (0, TYPE_CONST);
    }
    else
    {
        ret_val = GetE (trans);
    }
    root->right = ret_val;

    Require (RETEND);
    return root;
}

static TNode *GetFuncParams (Trans *trans)
{
    TNode *root = CreateNodeWithStr ("function", TYPE_SERVICE);

    if (CheckTok (trans, "Правила"))
    {
        Require (DEFPARAM);
        TNode *curr       = root;
        curr->right       = CreateNodeWithStr ("parameter", TYPE_SERVICE);
        curr              = curr->right;
        curr->right       = GetTok (trans);
        curr->right->type = TYPE_VAR;
        MovePtr (trans);

        $ AddId (TRANS_IDS, curr->right->data);

        while (!CheckTok (trans, "."))
        {
            curr->left        = CreateNodeWithStr ("parameter", TYPE_SERVICE);
            curr              = curr->left;
            curr->right       = GetTok (trans);
            curr->right->type = TYPE_VAR;

            $ AddId (TRANS_IDS, curr->right->data);

            MovePtr (trans);
        }
        MovePtr (trans);
    }

    return root;
}

TNode *GetFunc (Trans *trans)
{
    TNode *root = CreateNodeWithStr ("define", TYPE_SERVICE);
    $ Require (DEFSTART);

    int initIds = 0;

    LOCAL_IDS
    (
    TNode *name = NULL;
    name = GetTok (trans);
    MovePtr (trans);

    Require (DEFNAME);

    if (name->type != TYPE_ID)
    {
        SyntaxErr ("Invalid function type: %d at %.*s\n",
                   name->type, name->len, name->declared);
        return root;
    }

    root->left       = GetFuncParams (trans);
    root->left->left = name;

    Require (ALGA);

    TNode *body  = GetSt (trans, "Развернулся");
    root->right = body;

    Require (BACK_ALGA);
    )

    return root;
}

TNode *GetCall (Trans *trans)
{
    TNode *root  = CreateNodeWithStr ("call", TYPE_SERVICE);
    $ Require (ANEK);
    TNode *curr = root;

    curr->right       = CreateNodeWithStr ("parameter", TYPE_SERVICE);
    curr              = curr->right;
    curr->right       = GetE (trans);

    while (!CheckTok (trans, "."))
    {
        curr->left  = CreateNodeWithStr ("parameter", TYPE_SERVICE);
        curr        = curr->left;
        TNode *var  = GetE (trans);
        curr->right = var;
    }

    MovePtr (trans);

    Require (ANEKNAME);
    root->left = GetTok (trans);
    if (root->left->type != TYPE_ID)
    {
        SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
        DestructNode (root);
        return NULL;
    }

    MovePtr (trans);
    Require (DOT);

    return root;
}

TNode *GetIF (Trans *trans)
{
    TNode *root = CreateNodeWithStr ("if", TYPE_SERVICE);
    $ Require (IFSTART);

    $ root->left = GetE (trans);
    if (!root->left) return NULL;

    root->right  = CreateNodeWithStr ("decision", TYPE_SERVICE);
    TNode *curr = root->right;
    $ Require (IFTHEN);

    int initIds = 0;

    LOCAL_IDS
    (
    curr->left = GetSt (trans, "Ставь");
    $ Require (IFELSE);
    )

    LOCAL_IDS
    (
    curr->right = GetSt (trans, "и");
    $ Require (IFEND);
    )

    return root;
}

TNode *GetWhile (Trans *trans)
{
    int initIds = 0;
    LOCAL_IDS
    (
    TNode *root = CreateNodeWithStr ("while", TYPE_SERVICE);
    $ Require (WHILESTART);

    root->left = GetE (trans);
    $ Require (RAZ);

    root->right = GetSt (trans, "Дверь");
    $ Require (WHILEEND);
    )
    return root;
}

TNode *GetOP (Trans *trans)
{
    $ if (CheckTok (trans, "\""))
    {
        return GetDec (trans);
    }

    if (CheckTok (trans, "Господа"))
    {
        return GetFunc (trans);
    }

    if (CheckTok (trans, "Кто"))
    {
        return GetIF (trans);
    }

    if (CheckTok (trans, "В"))
    {
        return GetWhile (trans);
    }

    if (CheckTok (trans, "Анекдот"))
    {
        return GetCall (trans);
    }

    if (CheckTok (trans, "Голос"))
    {
        TNode *root = CreateNodeWithStr ("print", TYPE_SERVICE);
        Require (OUT_PH);
        root->right = GetE (trans);
        MovePtr (trans);
        return root;
    }

    if (CheckTok (trans, "Козырная"))
    {
        return GetRet (trans);
    }

    return Assn (trans);
}

TNode *Assn (Trans *trans)
{
    TNode *root = CreateNodeWithStr ("=", TYPE_OP);

    $ Require (ASSNSTART);

    TNode *var = GetID (trans);

    Require (ASSNIS);

    TNode *rootue = GetE (trans);
    int   idNum  = FindId (TRANS_IDS, var->data);

    if (idNum >= 0)
    {
        if (trans->IdsArr[idNum].isConst == 1)
        {
            SyntaxErr ("Assn: Changing constant variable is forbidden: %.*s\n",
                       var->len, var->declared);
            return NULL;
        }

        var->type   = TYPE_VAR;
        root->left  = var;
        root->right = rootue;

        Require (DOT);
        return root;
    }

    SyntaxErr ("Assn: Using an undeclared variable: %.*s\n", var->len, var->declared);
    return NULL;
}

TNode *GetDec (Trans *trans)
{
    TNode *root = NULL;
    $ Require (KAVYCHKA);

    TNode *idtok = GetTok (trans);
    MovePtr (trans);

    int id_pos = FindId (TRANS_IDS, idtok->data);
    if (id_pos >= 0)
    {
        SyntaxErr ("Re-declaration of variable: %.*s\nId_pos: %d\n",
                   idtok->len, idtok->declared, id_pos);
        LogMsg ("SyntaxErr!!! code: %s\n", idtok->declared);
        return NULL;
    }
    root  = CreateNode ('=', TYPE_OP, idtok->declared, idtok);
    idtok->type = TYPE_VAR;

    char isConst = 0;

    if (CheckTok (trans, "всегда"))
    {
        isConst = 1;
        MovePtr (trans);
        root->right = GetN (trans);
        LogMsg ("Declared const var: %.*s = %d\n",
                root->left->len, root->left->declared, root->right->data);
        idtok->left = CreateNodeWithStr ("const", TYPE_SERVICE);
    }
    else
    {
        root->right      = CreateNode (0, TYPE_CONST);
        root->right->len = 1;
        LogMsg ("Declared var: %.*s = 0\n",
                root->left->len, root->left->declared);
    }

    Require (DECLEND);

    TNode *arr_len = GetN (trans);
    if (arr_len->data <= 0)
    {
        SyntaxErr ("Invalid array length: arr %.*s of len %d\n",
                   idtok->len, idtok->declared, arr_len->data);
        return NULL;
    }
    Require (RAZ);

    idtok->right = arr_len;

    $ AddId (TRANS_IDS, idtok->data, isConst, (int) arr_len->data);

    return root;
}

void FreeTransTree (TNode *root, TNode **nodes, int nodesNum)
{
    $ if (!root)
    {
        for (int node = 0; node < nodesNum; node++)
        {
            free (nodes[node]);
        }
        return;
    }

    for (int node = 0; node < nodesNum; node++)
    {
        nodes[node]->type = TYPE_DEAD;
    }

    DestructNode (root);

    for (int node = 0; node < nodesNum; node++)
    {
        free (nodes[node]);
    }
}
