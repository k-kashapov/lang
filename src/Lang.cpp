#include "Lang.h"

int SyntaxErr (const char *msg, ...)
{
    printf ("SYNTAX ERROR: ");

    va_list arg = {};
    va_start (arg, msg);
    vprintf (msg, arg);
    va_end (arg);

    return 0;
}

void SkipSpaces (Trans *trans)
{
    while (isspace (*trans->s)) trans->s++;
}

void MovePtr (Trans *trans)
{
    trans->s++;
    SkipSpaces (trans);
}

int Require (Trans *trans, const char* req)
{
    SkipSpaces (trans);

    for (; *req != '\0'; req++)
    {
        if (*trans->s == *req)
        {
            trans->s++;
        }
        else
        {
            SyntaxErr ("expected: %s; got: %s", req, trans->s);
            return -1;
        }
    }

    SkipSpaces (trans);
    return 0;
}

int GetN (Trans *trans)
{
    int val = 0;

    SEMANTIC
    (
        if (*trans->s < '0' || *trans->s > '9')
        {
            SyntaxErr ("expected: number; got: %s", trans->s);
        }

        while (*trans->s >= '0' && *trans->s <= '9')
        {
            val = val * 10 + (*trans->s - '0');
            trans->s++;
        }
    )

    return val;
}

int GetP (Trans *trans)
{
    if (*trans->s == '(')
    {
        MovePtr (trans);
        int val = GetE (trans);
        Require (trans, ")");
        return val;
    }
    else if (isalpha (*trans->s))
    {
        int hash = GetId (trans);
        SEMANTIC
        (
            for (int id = 0; id < trans->IdsNum; id++)
            {
                if (trans->IdsArr[id].hash == hash)
                {
                    return trans->IdsArr[id].value;
                }
            }
        )

        SyntaxErr ("expected: ; got: %s", trans->s);
        return -1;
    }
    else
    {
        return GetN (trans);
    }
}

int GetT (Trans *trans)
{
    int val = GetP (trans);
    SkipSpaces (trans);

    while (*trans->s == '*' || *trans->s == '/')
    {
        char op = *trans->s;
        MovePtr (trans);
        int rVal = GetP (trans);
        SEMANTIC
        (
            if (op == '*') val *= rVal;
            else val /= rVal;
        )
    }

    return val;
}

int GetE (Trans *trans)
{
    int val = GetT (trans);
    SkipSpaces (trans);

    while (*trans->s == '+' || *trans->s == '-')
    {
        char op = *trans->s;
        MovePtr (trans);
        int rVal = GetT (trans);
        SEMANTIC
        (
            if (op == '+') val += rVal;
            else val -= rVal;
        )
    }

    return val;
}

int GetOP (Trans *trans)
{
    Require (trans, "-");
    int val = Assn (trans);
    Require (trans, "!");
    return val;
}

int GetG (Trans *trans)
{
    Require (trans, "You are my friend!");
    int val = GetOP (trans);
    while (*trans->s == '-')
    {
        val = GetOP (trans);
    }
    Require (trans, "Dattebayo!");
    return val;
}

int GetId (Trans *trans)
{
    int hash  = 0;
    int char_read = 0;

    if (!isalpha (*trans->s))
    {
        SyntaxErr ("expected: letter; got: %s", trans->s);
    }

    for (; isalpha (*trans->s); char_read++)
    {
        SEMANTIC
        (
            hash += *trans->s * char_read;
        )
        trans->s++;
    }

    return hash;
}

int Assn (Trans *trans)
{
    int hash = GetId (trans);
    Require (trans, "=");
    int rVal = GetE (trans);

    for (int id = 0; id < trans->IdsNum; id++)
    {
        if (trans->IdsArr[id].hash == hash)
        {
            trans->IdsArr[id].value = rVal;
            return rVal;
        }
    }

    Id new_id    = {};
    new_id.hash  = hash;
    new_id.value = rVal;

    trans->IdsArr[trans->IdsNum++] = new_id;

    return rVal;
}
