#include "Lang.h"

#define ST(l, r) CreateNode (0, TYPE_STATEMENT, NULL, l, r)

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

static int Require (Trans *trans, const char *req)
{
    char *req_str = strdup (req);
    for (char *strtoken = strtok (req_str, " "); strtoken; strtoken = strtok (NULL, " "))
    {
        int64_t hash = SimpleHash (strtoken, (int) strlen (strtoken));

        TNode *token = GetTok (trans);
        if (token->data != hash)
        {
            SyntaxErr ("expected: %s; got: %.*s\n", strtoken, token->len, token->declared);
            free (req_str);
            return REQUIRE_FAIL;
        }

        MovePtr (trans);
    }

    free (req_str);
    return 0;
}

TNode *CreateID (const char *id)
{
    int len     = (int) strlen (id);
    TNode *node = CreateNode (SimpleHash (id, len), TYPE_ID, id);
    node->len   = len;
    return node;
}

TNode *GetSt (Trans *trans, const char *end_cond, TNode **st_end)
{
    if (!CheckTok (trans, end_cond))
    {
        TNode *curr = ST (NULL, GetOP (trans));
        curr->left  = GetSt (trans, end_cond, st_end);

        if (st_end && !curr->left)
        {
            *st_end = curr;
        }

        // CreateNodeImage (curr, "tmp.png");
        return curr;
    }

    return NULL;
}

TNode *GetG (Trans *trans)
{
    Require (trans, "Купил мужик шляпу");

    TNode *root = ST (NULL, GetOP (trans));
    root->left = GetSt (trans, "А");

    Require (trans, "А она ему как раз");
    return root;
}

TNode *GetN (Trans *trans)
{
    TNode *node = GetTok (trans);

    if (node->type != TYPE_CONST)
    {
        SyntaxErr ("Invalid type: expected CONST, got %.*s (%d)\n", node->len, node->declared, node->type);
        return NULL;
    }
    MovePtr (trans);

    return node;
}

TNode *GetP (Trans *trans)
{
    TNode *token = GetTok (trans);

    if (HASH_EQ (token, BIBA)) // Left bracket '('
    {
        MovePtr (trans);
        TNode *node = GetE (trans);

        Require (trans, "Боба"); // Right bracket ')'
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

        for (int id = 0; id < trans->IdsNum; id++)
        {
            if (trans->IdsArr[id].hash == token->data)
            {
                MovePtr (trans);
                token->type = TYPE_VAR;
                return token;
            }
        }

        SyntaxErr ("Using an undeclared variable: %.*s\n",
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
    TNode *val = GetP (trans);

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
    TNode *val = GetPow (trans);

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

TNode *GetE (Trans *trans)
{
    TNode *val = GetT (trans);

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
            curr->right       = GetPow (trans);
        }

        val = action;
    }

    return val;
}

TNode *GetFunc (Trans *trans)
{
    Require (trans, "Господа , а не сыграть ли нам в новую игру .");
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

    Require (trans, "называется .");
    val->left       = CreateID ("function");
    val->left->left = name;
    Require (trans, "Правила очень просты :");

    TNode *curr = val->left;
    curr->right = CreateID ("parameter");
    curr        = curr->right;
    curr->right = GetTok (trans);

    Id local_var                   = {};
    local_var.hash                 = curr->right->data;
    trans->IdsArr[trans->IdsNum++] = local_var;

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

        local_var.hash                 = curr->right->data;
        trans->IdsArr[trans->IdsNum++] = local_var;

        if (curr->right->type != TYPE_ID)
        {
            SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
            DestructNode (val);
            return NULL;
        }
        MovePtr (trans);
    }
    MovePtr (trans);

    val->right = ST (NULL, GetOP (trans));
    curr       = val->right;

    TNode *last_st = NULL;
    curr->left = GetSt (trans, "Козырная", &last_st);
    MovePtr (trans);

    if (!CheckTok (trans, ","))
    {
        TNode *ret = CreateID ("return");
        ret->right = GetE (trans);
        last_st->left = ret;
    }

    Require (trans, ", господа .");

    for (int curr_id = initIds; curr_id < trans->IdsNum; curr_id++)
    {
        trans->IdsArr[curr_id] = {};
    }

    return val;
}

TNode *GetCall (Trans *trans)
{
    Require (trans, "Анекдот : Заходят как - то в бар");

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

    Require (trans, "А бармен им говорит :");
    val->right->left = GetTok (trans);
    if (val->right->left->type != TYPE_ID)
    {
        SyntaxErr ("Invalid type: expected id; got %d\n", curr->right->type);
        DestructNode (val);
        return NULL;
    }
    MovePtr (trans);
    Require (trans, ".");

    return val;
}

TNode *GetIF (Trans *trans)
{
    Require (trans, "Кто прочитал");

    TNode *val = CreateID ("if");

    val->right  = GetE (trans);
    val->left   = CreateID ("decision");
    TNode *curr = val->left;
    Require (trans, "тот сдохнет .");

    curr->right = GetSt (trans, "Ставь");
    Require (trans, "Ставь лайк");

    curr->left = GetSt (trans, "и");
    Require (trans, "и можешь считать , что не читал .");

    return val;
}

TNode *GetWhile (Trans *trans)
{
    Require (trans, "В дверь постучали");
    TNode *val  = CreateID ("while");
    val->right  = GetE (trans);
    Require (trans, "раз .");
    val->left   = GetSt (trans, "Дверь");

    Require (trans, "Дверь отвалилась .");

    return val;
}

TNode *GetOP (Trans *trans)
{
    TNode *val = NULL;
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

    Require (trans, ".");
    return val;
}

TNode *Assn (Trans *trans)
{
    Require (trans, "Этим");

    TNode *var = GetTok (trans);
    MovePtr (trans);

    Require (trans, "был");

    TNode *value = GetE (trans);

    for (int id = 0; id < trans->IdsNum; id++)
    {
        if (trans->IdsArr[id].hash == var->data)
        {
            var->type   = TYPE_VAR;
            TNode *node = CreateID ("=");
            node->type  = TYPE_OP;
            node->left  = var;
            node->right = value;
            return node;
        }
    }

    SyntaxErr ("Using an undeclared variable: %.*s\n", var->len, var->declared);
    return NULL;
}

TNode *GetDec (Trans *trans)
{
    if (Require (trans, "\"")) return NULL;

    TNode *idtok = GetTok (trans);
    MovePtr (trans);

    if (Require (trans, "\" - подумал Штирлиц .")) return NULL;

    for (int id = 0; id < trans->IdsNum; id++)
    {
        if (trans->IdsArr[id].hash == idtok->data)
        {
            SyntaxErr ("Re-declaration of variable: %.*s\n",
                       idtok->len, idtok->declared);
            return NULL;
        }
    }

    Id new_id    = {};
    new_id.hash  = idtok->data;

    trans->IdsArr[trans->IdsNum++] = new_id;

    idtok->type     = TYPE_VAR;
    TNode *val      = CreateNode (SimpleHash ("=", 1), TYPE_OP, idtok->declared, idtok);
    val->right      = CreateNode (0, TYPE_CONST);
    val->right->len = 1;

    return val;
}

void FreeTransTree (TNode *root, TNode **nodes, int nodesNum)
{
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
