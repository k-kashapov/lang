#include "Lang.h"
#include <wchar.h>

const char INIT_NODES_NUM = 10;

struct LexicAn
{
    const char *str;
    TNode      **nodesArr;
    int        nodesNum;
    int        nodesCap;
};

int IsAlpha (char val)
{
    for (int ch = 0; ch < AlphaNum; ch++)
    {
        if (val == NotAlpha[ch])
        {
            return 0;
        }
    }

    return 1;
}

int SyntaxErr (const char *msg, ...)
{
    printf ("SYNTAX ERROR: ");

    va_list arg = {};
    va_start (arg, msg);
    vprintf (msg, arg);
    va_end (arg);

    return 0;
}

static void SkipSpaces (LexicAn *la)
{
    while (isspace (*la->str)) la->str++;
}

static void MovePtr (LexicAn *la)
{
    la->str++;
    SkipSpaces (la);
}

static TNode *GetLexToken (LexicAn *lan)
{
    if (IsAlpha (*lan->str))
    {
        int64_t hash       = 0;
        int     bytes_read = 1;
        do
        {
            hash += *lan->str * bytes_read;
            bytes_read++;
            lan->str++;
        } while (IsAlpha (*lan->str));

        return CreateNode (hash, TYPE_ID);
    }
    else if (*lan->str >= '0' && *lan->str <= '9')
    {
        int64_t val = (int64_t) (*lan->str - '0');
        do
        {
            val = val * 10 + (*lan->str - '0');
            MovePtr (lan);
        } while (*lan->str >= '0' && *lan->str <= '9');

        return CreateNode (val, TYPE_CONST);
    }
    else switch (*lan->str)
    {
        case '-':
            MovePtr (lan);
            return CreateNode ('-', TYPE_ID);
        case '!': [[fallthrough]];
        case '?': [[fallthrough]];
        case '.':
            MovePtr (lan);
            return CreateNode ('.', TYPE_ID);
        default:
            SyntaxErr ("Unknown snymbol: %s", lan->str);
    }

    return NULL;
}

static int AnalyzeString (LexicAn *lan)
{
    long       code_len  = (long) strlen (lan->str);
    const char *init_str = lan->str;

    while (*lan->str != '\0' && lan->str - init_str < code_len)
    {
        TNode *tok = GetLexToken (lan);
        if (!tok) return -1;

        if (lan->nodesNum >= lan->nodesCap)
        {
            size_t new_size = (unsigned long) lan->nodesCap * 2 * sizeof (TNode *);
            TNode **tmp = (TNode **) realloc (lan->nodesArr, new_size);
            assert (tmp);
            lan->nodesArr = tmp;
            lan->nodesCap *= 2;
        }

        lan->nodesArr[lan->nodesNum++] = tok;
    }

    return (int) (lan->str - init_str);
}

TNode **LexicAnalysis (const char *string)
{
    TNode **nodesArr = (TNode **) calloc (INIT_NODES_NUM, sizeof (TNode *));
    LexicAn lan      = {};
    lan.str          = string;
    lan.nodesArr     = nodesArr;
    lan.nodesCap     = INIT_NODES_NUM;

    int read = AnalyzeString (&lan);
    if (read < 1)
    {
        printf ("ERROR: Analysis failed!\n");
        return NULL;
    }

    return lan.nodesArr;
}
