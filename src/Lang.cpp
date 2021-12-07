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
            long line_len = strchr (token->declared, '\n') - token->declared;
            SyntaxErr ("expected: %s; got: %.*s\n", strtoken, line_len, token->declared);
            free (req_str);
            return REQUIRE_FAIL;
        }

        MovePtr (trans);
    }

    free (req_str);
    return 0;
}

TNode *GetG (Trans *trans)
{
    Require (trans, "Купил мужик шляпу");

    TNode *root = ST (GetOP (trans), NULL);
    TNode *curr = root;

    while (!CheckTok (trans, "А"))
    {
        curr->right = ST (GetOP (trans), NULL);
        curr        = curr->right;
    }

    VisitNode (root, TreePrintLeftBracket, TreeNodePrint, TreePrintRightBracket);

    Require (trans, "А она ему как раз");
    return root;
}

TNode *GetN (Trans *trans)
{
    TNode *node = GetTok (trans);

    if (node->type != TYPE_CONST)
    {
        long line_len = strchr (node->declared, '\n') - node->declared;
        SyntaxErr ("Invalid type: expected CONST, got %.*s (%d)\n", line_len, node->declared, node->type);
        return NULL;
    }
    MovePtr (trans);

    return node;
}

TNode *GetP (Trans *trans)
{
    TNode *token = GetTok (trans);

    if (CheckTok (trans, "LEFT")) // Left bracket '('
    {
        MovePtr (trans);
        TNode *node = GetE (trans);

        Require (trans, "RIGHT"); // Right bracket ')'
        return node;
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

        long line_len = strchr (token->declared, '\n') - token->declared;
        SyntaxErr ("Using an undeclared variable: %.*s\n",
                   line_len,
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

        TNode *rVal   = GetPow (trans);
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
    TNode *val = CreateNode (SimpleHash ("def", 4), TYPE_FUNC, "def");

    TNode *name = GetTok (trans);
    MovePtr (trans);

    if (name->type != TYPE_ID)
    {
        long line_len = strchr (name->declared, '\n') - name->declared;
        SyntaxErr ("Invalid function type: %d at %.*s\n",
                   name->type, line_len, name->declared);
        return NULL;
    }

    Require (trans, "называется .");
    val->left   = name;
    TNode *curr = val;

    while (!CheckTok (trans, "Ржевский"))
    {
        curr->right = ST (GetOP (trans), NULL);
        curr        = curr->right;
    }

    Require (trans, "Ржевский спокойно и невозмутимо положил карты на стол .");
    return val;
}

TNode *GetIF (Trans *trans)
{
    Require (trans, "Кто прочитал");

    TNode *val = CreateNode (SimpleHash ("if", 2), TYPE_FUNC, "if");

    val->left   = ST (GetE (trans), NULL);
    TNode *curr = val->left;

    while (!CheckTok (trans, "тот"))
    {
        curr->right = ST (GetP (trans), NULL);
        curr        = curr->right;
    }

    Require (trans, "тот");

    val->right = CreateNode (SimpleHash ("TODO?", 5), TYPE_FUNC, "TODO?");
    curr       = val->right;
    curr->left = ST (GetOP (trans), NULL);
    curr       = curr->left;

    while (!CheckTok (trans, "Ставь"))
    {
        curr->right = ST (GetOP (trans), NULL);
        curr        = curr->right;
    }

    Require (trans, "Ставь лайк");

    curr = val->right;
    while (!CheckTok (trans, "и"))
    {
        curr->right = ST (GetOP (trans), NULL);
        curr        = curr->right;
    }

    Require (trans, "и можешь считать , что не читал .");

    return val;
}

TNode *GetWhile (Trans *trans)
{
    Require (trans, "В дверь постучали");
    TNode *val  = CreateNode (SimpleHash ("while", 5), TYPE_FUNC, "while");
    val->left   = ST (GetE (trans), NULL);
    TNode *curr = val->left;

    while (!CheckTok (trans, "раз"))
    {
        curr->right = ST (GetE (trans), NULL);
        curr        = curr->right;
    }

    Require (trans, "раз");

    val->right = ST (GetOP (trans), NULL);
    curr       = val->right;

    while (!CheckTok (trans, "Дверь"))
    {
        curr->right = ST (GetOP (trans), NULL);
        curr        = curr->right;
    }

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
            var->type = TYPE_VAR;
            return CreateNode ('=', TYPE_OP, var->declared, var, value);
        }
    }

    long line_len = strchr (var->declared, '\n') - var->declared;
    SyntaxErr ("Using an undeclared variable: %.*s\n", line_len, var->declared);
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
            long line_len = strchr (idtok->declared, '\n') - idtok->declared;
            SyntaxErr ("Re-declaration of variable: %.*s\n",
                       line_len, idtok->declared);
            return NULL;
        }
    }

    Id new_id    = {};
    new_id.hash  = idtok->data;

    trans->IdsArr[trans->IdsNum++] = new_id;

    idtok->type = TYPE_VAR;
    TNode *val = CreateNode (SimpleHash ("=", 1), TYPE_OP, idtok->declared, idtok);
    val->right = CreateNode (0, TYPE_CONST);

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
