#include "Lang.h"

int64_t SimpleHash (const void *data, int len)
{
    int64_t hash        = 0;
    const char *data_ch = (const char *) data;

    for (int byte = 0; byte < len; byte++)
    {
        hash += data_ch[byte] * (byte + 1);
    }

    return hash;
}

static void MovePtr (Trans *trans)
{
    trans->s++;
}

static TNode *GetTok (Trans *trans)
{
    return *trans->s;
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

#define Require(val) if (!Require_ (trans, val)) return NULL;

TNode *CreateID (const char *id)
{
    $$ int len   = (int) strlen (id);
    TNode *node = CreateNode (SimpleHash (id, len), TYPE_ID, id);
    node->len   = len;
    return node;
}

TNode *GetSt (Trans *trans, const char *end_cond)
{
    if (CheckTok (trans, end_cond)) return NULL;

    $ TNode *stmt = ST (NULL, GetOP (trans));
    TNode *curr = stmt;

    while (!CheckTok (trans, end_cond))
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
        return NULL;
    }
    MovePtr (trans);

    return node;
}

TNode *GetP (Trans *trans)
{
    $$ TNode *token = GetTok (trans);

    if (HASH_EQ (token, BIBA)) // Left bracket '('
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

TNode *GetPow (Trans *trans)
{
    $$ TNode *val = GetP (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "^"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetP (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "^"))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetP (trans);
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

TNode *GetCond (Trans *trans)
{
    $$ TNode *val = GetSum (trans);

    $$ if (!val) return NULL;

    printf ("checktok = %d\nnext cond = %.*s\n", (CheckTok (trans, ">") || CheckTok (trans, "<") || CheckTok (trans, ">=") || CheckTok (trans, "<=") || CheckTok (trans, "!=")), GetTok (trans)->len, GetTok (trans)->declared);

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
    $$ TNode *val = GetCond (trans);

    if (!val) return NULL;

    if (CheckTok (trans, "||") ||
        CheckTok (trans, "&&"))
    {
        TNode *action = GetTok (trans);
        MovePtr (trans);

        TNode *rVal   = GetCond (trans);
        action->left  = val;
        action->right = rVal;
        TNode *curr   = action;

        while (CheckTok (trans, "||") ||
               CheckTok (trans, "&&"))
        {
            TNode *new_action = GetTok (trans);
            new_action->left  = curr->right;
            curr->right       = new_action;
            curr              = new_action;
            curr->right       = GetCond (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetFunc (Trans *trans)
{
    $ Require (DEFSTART);
    TNode *val = CreateID ("define");

    int initIds = trans->IdsNum;

    TNode *name = GetTok (trans);
    MovePtr (trans);

    if (name->type != TYPE_ID)
    {
        SyntaxErr ("Invalid function type: %d at %.*s\n",
                   name->type, name->len, name->declared);
        return NULL;
    }

    Require (DEFNAME);
    val->left       = CreateID ("function");
    val->left->left = name;
    Require (DEFPARAM);

    TNode *curr = val->left;
    curr->right = CreateID ("parameter");
    curr        = curr->right;
    curr->right = GetTok (trans);

    $ AddId (TRANS_IDS, curr->right->data);

    if (curr->right->type != TYPE_ID)
    {
        SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
        DestructNode (val);
        return NULL;
    }
    MovePtr (trans);

    while (!CheckTok (trans, "."))
    {
        curr->left  = CreateID ("parameter");
        curr        = curr->left;
        curr->right = GetTok (trans);

        $ AddId (TRANS_IDS, curr->right->data);

        if (curr->right->type != TYPE_ID)
        {
            SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
            DestructNode (val);
            return NULL;
        }
        MovePtr (trans);
    }
    MovePtr (trans);

    TNode *body = GetSt (trans, "Козырная");

    MovePtr (trans);

    TNode *ret = CreateID ("return");
    body  = ST (body, ret);

    if (!CheckTok (trans, ","))
    {
        ret->right = GetE (trans);
    }

    val->right = body;

    Require (RETEND);

    $ RmId (TRANS_IDS, trans->IdsNum - initIds);

    return val;
}

TNode *GetCall (Trans *trans)
{
    $ Require (ANEK);

    TNode *val  = CreateID ("call");
    val->right  = CreateID ("function");
    TNode *curr = val->right;

    curr->right = CreateID ("parameter");
    curr        = curr->right;
    curr->right = GetTok (trans);
    MovePtr (trans);

    if (curr->right->type != TYPE_ID)
    {
        SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
        DestructNode (val);
        return NULL;
    }

    while (!CheckTok (trans, "."))
    {
        curr->left  = CreateID ("parameter");
        curr        = curr->left;
        curr->right = GetTok (trans);

        if (curr->right->type != TYPE_ID)
        {
            SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
            DestructNode (val);
            return NULL;
        }
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

    TNode *val = CreateID ("if");

    $ val->left = GetE (trans);
    val->right  = CreateID ("decision");
    TNode *curr = val->right;
    $ Require (IFTHEN);

    curr->left = GetSt (trans, "Ставь");
    $ Require (IFELSE);

    curr->right = GetSt (trans, "и");
    $ Require (IFEND);

    return val;
}

TNode *GetWhile (Trans *trans)
{
    $ Require (WHILESTART);

    TNode *val  = CreateID ("while");

    val->right  = GetE (trans);
    $ Require (WHILEDO);

    val->left   = GetSt (trans, "Дверь");
    $ Require (WHILEEND);

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

    if (FindId (TRANS_IDS, var->data) >= 0)
    {
        var->type   = TYPE_VAR;
        TNode *node = CreateID ("=");
        node->type  = TYPE_OP;
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

    Require (DECLEND);

    if (FindId (TRANS_IDS, idtok->data) >= 0)
    {
        SyntaxErr ("Re-declaration of variable: %.*s\n",
                   idtok->len, idtok->declared);
        return NULL;
    }

    $ AddId (TRANS_IDS, idtok->data);

    idtok->type     = TYPE_VAR;
    TNode *val      = CreateNode (SimpleHash ("=", 1), TYPE_OP, idtok->declared, idtok);
    val->right      = CreateNode (0, TYPE_CONST);
    val->right->len = 1;

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
