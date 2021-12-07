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
    while (isspace (*la->str))
    {
        la->str++;
    }
}

static void MovePtr (LexicAn *la)
{
    la->str++;
    SkipSpaces (la);
}

static TNode *GetLexToken (LexicAn *lan)
{
    printf ("char = %c; (%3d); ", *lan->str, *lan->str);

    const char *declared = lan->str;

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

        return CreateNode (hash, TYPE_ID, declared);
    }
    else if (*lan->str >= '0' && *lan->str <= '9')
    {
        int64_t val = 0;

        do
        {
            val = val * 10 + (*lan->str - '0');
            MovePtr (lan);
        } while (*lan->str >= '0' && *lan->str <= '9');

        return CreateNode (val, TYPE_CONST, declared);
    }
    else switch (*lan->str)
    {
        case '^': [[fallthrough]];
        case '+': [[fallthrough]];
        case '-': [[fallthrough]];
        case '*': [[fallthrough]];
        case '/': [[fallthrough]];
        case ',': [[fallthrough]];
        case ':': [[fallthrough]];
        case '\"':
            {
                char sign = *lan->str;
                MovePtr (lan);
                return CreateNode (sign, TYPE_OP, declared);
            }
        case '!': [[fallthrough]];
        case '?': [[fallthrough]];
        case '.':
            MovePtr (lan);
            return CreateNode ('.', TYPE_ID, declared);
        default:
            SyntaxErr ("Unknown snymbol: %c (%d): %s",
                       *lan->str, *lan->str, lan->str);
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
        printf ("%ld\n", tok->data);
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
        SkipSpaces (lan);
    }

    return (int) (lan->str - init_str);
}

TNode **LexicAnalysis (const char *string, int *nodesNum)
{
    TNode **nodesArr = (TNode **) calloc (INIT_NODES_NUM, sizeof (TNode *));
    LexicAn lan      = {};
    lan.str          = string;
    lan.nodesNum     = 0;
    lan.nodesArr     = nodesArr;
    lan.nodesCap     = INIT_NODES_NUM;

    int read = AnalyzeString (&lan);
    if (read < 1)
    {
        printf ("ERROR: Analysis failed!\n");
        return NULL;
    }

    *nodesNum = lan.nodesNum;

    return lan.nodesArr;
}
