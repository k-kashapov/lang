#include "Lang.h"

int64_t SimpleHash (const void *data, int len)
{
    int64_t hash        = 0;
    const char *data_ch = (char *) data;

    for (int byte = 0; byte < len; byte++)
    {
        hash += data_ch[byte] * (byte + 1);
    }

    return hash;
}

int Require (Trans *trans, const char *req)
{
    int64_t hash = SimpleHash (token, strlen (token));
    if (!trans->s->data != hash)
    {
        SyntaxErr ("expected: %s\n", req);
        return REQUIRE_FAIL;
    }
    SkipSpaces (trans);

    return 0;
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
    Require (trans, "-tailed beast");

    return val;
}

int GetP (Trans *trans)
{
    if (*trans->s == '(')
    {
        int val = GetE (trans);
        Require (trans, ")");
        return val;
    }
    else if (isalpha (*trans->s))
    {
        char *init_s = trans->s;

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

        SyntaxErr ("Using an undeclared variable: %s\n", init_s);
        return UNDECLARED;
    }
    else
    {
        return GetN (trans);
    }
}

int GetT (Trans *trans)
{
    int val = GetP (trans);

    int mul = 0;
    int div = 0;

    while ((mul = CheckString (trans, "Kage Bunshin")) ||
           (div = CheckString (trans, "/")))
    {
        trans->s += mul + div;
        int rVal = GetP (trans);
        SEMANTIC
        (
            if (mul)
            {
                val *= rVal;
            }
            else
            {
                val /= rVal;
            }
        )
    }

    return val;
}

int GetE (Trans *trans)
{
    int val = GetT (trans);

    int add = 0;
    int sub = 0;

    while ((add = CheckString (trans, "Rasengan")) ||
           (sub = CheckString (trans, "Chidori")))
    {
        trans->s += add + sub;
        int rVal = GetT (trans);
        SEMANTIC
        (
            if (add)
            {
                val += rVal;
            }
            else
            {
                val -= rVal;
            }
        )
    }

    return val;
}

int GetOP (Trans *trans)
{
    Require (trans, "-");

    int val = 0;
    if (CheckString (trans, "Kuchiyose no Jutsu"))
    {
        val = GetDec (trans);
    }
    else
    {
        val = Assn (trans);
    }

    Require (trans, "!");
    return val;
}

int GetId (Trans *trans)
{
    int hash  = 0;
    int char_read = 1;

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
    char *init_s = trans->s;

    int hash = GetId (trans);
    int rVal = GetE (trans);
    Require (trans, "Desu");

    for (int id = 0; id < trans->IdsNum; id++)
    {
        if (trans->IdsArr[id].hash == hash)
        {
            trans->IdsArr[id].value = rVal;
            return rVal;
        }
    }

    SyntaxErr ("Using an undeclared variable: %s\n", init_s);
    return UNDECLARED;
}

int GetDec (Trans *trans)
{
    int bytes_read = CheckString (trans, "Kuchiyose no Jutsu");
    if (bytes_read)
    {
        char *init_s = trans->s;

        trans->s += bytes_read;
        int hash = GetId (trans);

        for (int id = 0; id < trans->IdsNum; id++)
        {
            if (trans->IdsArr[id].hash == hash)
            {
                SyntaxErr ("Re-declaration of variable: %s\n", init_s);
                return REDECLARATION;
            }
        }

        Id new_id    = {};
        new_id.hash  = hash;
        new_id.value = 0;

        trans->IdsArr[trans->IdsNum++] = new_id;
        return hash;
    }

    return -1;
}
