#include "Lang.h"

static int COMPILATION_ERR = 0;

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
            COMPILATION_ERR = REQUIRE_FAIL;                                     \
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

#define Require(req) if (!Require_ (trans, req)) return NULL;

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

TNode *GetG (Trans *trans)
{
    $ Require (INIT);

    TNode *root = GetSt (trans, "А");

    Require (END);
    return root;
}

TNode *GetN (Trans *trans)
{
    $$ TNode *node = GetTok (trans);

    if (node->type != TYPE_CONST)
    {
        SyntaxErr ("Invalid type: expected CONST, got %.*s (%d)\n",
                   node->len, node->declared, node->type);
        return node;
    }
    MovePtr (trans);

    return node;
}

TNode *GetP (Trans *trans)
{
    $$ TNode *token = GetTok (trans);

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
    else if (token->type == TYPE_ID)
    {
        for (int func_id = 0; func_id < UnaryNum; func_id++)
        {
            if (token->data == UnaryFuncs[func_id])
            {
                TNode *action = GetTok (trans);
                MovePtr (trans);
                action->right = GetP (trans);
                return action;
            }
        }

        if (FindId (TRANS_IDS, token->data) >= 0)
        {
            MovePtr (trans);
            token->type = TYPE_VAR;
            return token;
        }

        SyntaxErr ("Get ID: Using an undeclared variable: %.*s\n",
                   token->len,
                   token->declared);
        return NULL;
    }
    else
    {
        return GetN (trans);
    }
}

TNode *GetNeg (Trans *trans)
{
    $$ TNode *val = GetTok (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "Нифига"))
    {
        Require (NEG);
        val = CreateNode ('!', TYPE_OP, val->declared, NULL, GetP (trans));
        val->len = 1;
    }
    else
    {
        val = GetP (trans);
    }

    return val;
}

TNode *GetPow (Trans *trans)
{
    $$ TNode *val = GetNeg (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "^"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetNeg (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "^"))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetNeg (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetT (Trans *trans)
{
    $$ TNode *val = GetPow (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "*") ||
        CheckTok (trans, "/"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetPow (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "*") ||
               CheckTok (trans, "/"))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetPow (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetSum (Trans *trans)
{
    $$ TNode *val = GetT (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "+") ||
        CheckTok (trans, "-"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetT (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "+") ||
               CheckTok (trans, "-"))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetT (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetCmp (Trans *trans)
{
    $$ TNode *val = GetSum (trans);

    $$ if (!val) return NULL;

    if (CheckTok (trans, ">")  ||
        CheckTok (trans, "<")  ||
        CheckTok (trans, ">=") ||
        CheckTok (trans, "<=") ||
        CheckTok (trans, "!="))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetSum (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, ">")  ||
               CheckTok (trans, "<")  ||
               CheckTok (trans, ">=") ||
               CheckTok (trans, "<=") ||
               CheckTok (trans, "!="))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetSum (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetE (Trans *trans)
{
    $$ TNode *val = GetCmp (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "||") ||
        CheckTok (trans, "&&"))
    {
        TNode *action = GetTok (trans);
        action->type  = TYPE_OP;
        MovePtr (trans);

        TNode *rVal   = GetCmp (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "||") ||
               CheckTok (trans, "&&"))
        {
            TNode *new_action = GetTok (trans);
            new_action->type  = TYPE_OP;
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetCmp (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetFunc (Trans *trans)
{
    $ Require (DEFSTART);
    TNode *val = CreateNodeWithStr ("define", TYPE_SERVICE);

    int initIds = 0;

    LOCAL_IDS
    (
    TNode *name = GetTok (trans);
    MovePtr (trans);

    if (name->type != TYPE_ID)
    {
        SyntaxErr ("Invalid function type: %d at %.*s\n",
                   name->type, name->len, name->declared);
        return NULL;
    }

    Require (DEFNAME);
    val->left       = CreateNodeWithStr ("function", TYPE_SERVICE);
    val->left->left = name;
    Require (DEFPARAM);

    TNode *curr       = val->left;
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

    TNode *body = GetSt (trans, "Козырная");

    MovePtr (trans);

    TNode *ret = CreateNodeWithStr ("return", TYPE_SERVICE);
    body  = ST (body, ret);

    if (!CheckTok (trans, ","))
    {
        ret->right = GetE (trans);
    }

    val->right = body;

    Require (RETEND);
    )

    return val;
}

TNode *GetCall (Trans *trans)
{
    $ Require (ANEK);

    TNode *val  = CreateNodeWithStr ("call", TYPE_SERVICE);
    val->right  = CreateNodeWithStr ("function", TYPE_SERVICE);
    TNode *curr = val->right;

    curr->right       = CreateNodeWithStr ("parameter", TYPE_SERVICE);
    curr              = curr->right;
    curr->right       = GetTok (trans);
    curr->right->type = TYPE_VAR;
    MovePtr (trans);

    while (!CheckTok (trans, "."))
    {
        curr->left        = CreateNodeWithStr ("parameter", TYPE_SERVICE);
        curr              = curr->left;
        TNode *var        = GetTok (trans);
        if (FindId (TRANS_IDS, var->data) < 0)
        {
            SyntaxErr ("Call: Using an undeclared variable: %.*s\n",
                       var->len, var->declared);
            return NULL;
        }

        curr->right = var;

        MovePtr (trans);
    }

    MovePtr (trans);

    Require (ANEKNAME);
    val->right->left = GetTok (trans);
    if (val->right->left->type != TYPE_ID)
    {
        SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
        DestructNode (val);
        return NULL;
    }

    MovePtr (trans);
    Require (DOT);

    return val;
}

TNode *GetIF (Trans *trans)
{
    $ Require (IFSTART);

    TNode *val = CreateNodeWithStr ("if", TYPE_SERVICE);

    $ val->left = GetE (trans);
    if (!val->left) return NULL;

    val->right  = CreateNodeWithStr ("decision", TYPE_SERVICE);
    TNode *curr = val->right;
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

    return val;
}

TNode *GetWhile (Trans *trans)
{
    int initIds = 0;
    LOCAL_IDS
    (
    $ Require (WHILESTART);

    TNode *val  = CreateNodeWithStr ("while", TYPE_SERVICE);

    val->right  = GetE (trans);
    $ Require (RAZ);

    val->left   = GetSt (trans, "Дверь");
    $ Require (WHILEEND);
    )
    return val;
}

TNode *GetOP (Trans *trans)
{
    $ TNode *val = NULL;
    while (CheckTok (trans, "\""))
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

    val = Assn (trans);

    Require (DOT);
    return val;
}

TNode *Assn (Trans *trans)
{
    $ Require (ASSNSTART);

    TNode *var = GetTok (trans);
    MovePtr (trans);

    Require (ASSNIS);

    TNode *value = GetE (trans);
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
        TNode *node = CreateNodeWithStr ("=", TYPE_OP);
        node->left  = var;
        node->right = value;
        return node;
    }

    SyntaxErr ("Assn: Using an undeclared variable: %.*s\n", var->len, var->declared);
    return NULL;
}

TNode *GetDec (Trans *trans)
{
    $ Require (KAVYCHKA);

    TNode *idtok = GetTok (trans);
    MovePtr (trans);
    if (FindId (TRANS_IDS, idtok->data) >= 0)
    {
        SyntaxErr ("Re-declaration of variable: %.*s\n",
                   idtok->len, idtok->declared);
        return NULL;
    }
    TNode *val  = CreateNode ('=', TYPE_OP, idtok->declared, idtok);
    idtok->type = TYPE_VAR;

    if (CheckTok (trans, "всегда"))
    {
        MovePtr (trans);
        val->right = GetN (trans);

        $ AddId (TRANS_IDS, idtok->data, 1);
        idtok->left = CreateNodeWithStr ("const", TYPE_SERVICE);
    }
    else
    {
        val->right      = CreateNode (0, TYPE_CONST);
        val->right->len = 1;
        $ AddId (TRANS_IDS, idtok->data, 0);
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

    return val;
}

void FreeTransTree (TNode *root, TNode **nodes, int nodesNum)
{
    $ for (int node = 0; node < nodesNum; node++)
    {
        nodes[node]->type = TYPE_DEAD;
    }

    DestructNode (root);

    for (int node = 0; node < nodesNum; node++)
    {
        free (nodes[node]);
    }
}
