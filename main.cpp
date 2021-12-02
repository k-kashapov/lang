#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "files.h"

const int MAX_STR_LEN = 100;
char *s               = NULL;
int SynErr            = 0;

struct Id
{
    int64_t hash;
    int     value;
};

Id  IdsArr[10] = {};
int IdsNum     = 0;

#define SEMANTIC(cmd) cmd

int GetN();
int GetP();
int GetT();
int GetE();
int GetId();
int Assn();
int Require (char req);
int GetG();

#define SYNTAX_ERR()                                                            \
{                                                                               \
    printf ("%s: SYNTAX ERROR: %s\n", __FUNCTION__, s);                         \
    SynErr = 1;                                                                 \
    return -1;                                                                  \
}

int Require (char req)
{
    if (*s == req)
    {
        s++;
    }
    else
    {
        SYNTAX_ERR();
        return -1;
    }
    return 0;
}

int GetN()
{
    int val = 0;

    SEMANTIC
    (
        if (*s < '0' || *s > '9')
        {
            SYNTAX_ERR();
        }

        while (*s >= '0' && *s <= '9')
        {
            val = val * 10 + (*s - '0');
            s++;
        }
    )

    return val;
}

int GetP()
{
    if (*s == '(')
    {
        s++;
        int val = GetE();
        Require (')');
        return val;
    }
    else if (isalpha(*s))
    {
        return GetId();
    }
    else
    {
        return GetN();
    }
}

int GetT()
{
    int val = GetP();
    while (*s == '*' || *s == '/')
    {
        char op = *s;
        s++;
        int rVal = GetP();
        SEMANTIC
        (
            if (op == '*') val *= rVal;
            else val /= rVal;
        )
    }

    return val;
}

int GetE()
{
    int val = GetT();
    while (*s == '+' || *s == '-')
    {
        char op = *s;
        s++;
        int rVal = GetT();
        SEMANTIC
        (
            if (op == '+') val += rVal;
            else val -= rVal;
        )
    }

    return val;
}

int GetG()
{
    int val = Assn();
    Require ('$');
    return val;
}

int GetId()
{
    int hash  = 0;
    int char_read = 0;

    if (!isalpha (*s))
    {
        SYNTAX_ERR();
    }

    for (; isalpha (*s); char_read++)
    {
        SEMANTIC
        (
            hash += *s << ((unsigned long) char_read % sizeof (int));
        )
        s++;
    }

    SEMANTIC
    (
        for (int id = 0; id < IdsNum; id++)
        {
            if (IdsArr[id].hash == hash)
            {
                return IdsArr[id].value;
            }
        }
    )

    return -1;
}

int Assn()
{
    int hash = GetId();
    printf ("hash is %d\n", hash);
    Require ('=');
    printf ("next = %s\n", s);
    int rVal = GetE();
    printf ("rVal is %d\n", rVal);

    for (int id = 0; id < IdsNum; id++)
    {
        if (IdsArr[id].hash == hash)
        {
            IdsArr[id].value = rVal;
            return rVal;
        }
    }

    Id new_id    = {};
    new_id.hash  = hash;
    new_id.value = rVal;

    IdsArr[IdsNum++] = new_id;

    return rVal;
}

int main (int argc, const char **argv)
{
    Config io_config = {};
    GetArgs (argc, argv, &io_config);

    File_info code_info = {};

    long lines = read_all_lines (&code_info, io_config.input_file);
    if (lines < 1)
    {
        printf ("NO lines read!\n");
        return -1;
    }

    int val = 0;

    for (long line = 0; line < code_info.lines_num; line++)
    {
        s = code_info.strs[line]->text;
        val = GetG();
    }

    printf ("res = %d\n", val);

    free_info (&code_info);

    return 0;
}
