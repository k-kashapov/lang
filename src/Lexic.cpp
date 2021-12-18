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
    LogMsg ("char = %c%c; (%3d); ", lan->str[0], lan->str[1], *lan->str);

    const char *declared = lan->str;

    if (IsAlpha (*lan->str))
    {
        int64_t hash       = 0;
        int     bytes_read = 1;
        do
        {
            LogMsg ("%c", *lan->str);
            hash += *lan->str * bytes_read;
            bytes_read++;
            lan->str++;
        } while (IsAlpha (*lan->str));

        TNode *node = CreateNode (hash, TYPE_ID, declared);
        node->len   = bytes_read - 1;

        return node;
    }
    else if (*lan->str >= '0' && *lan->str <= '9')
    {
        int64_t val = 0;
        int len     = 0;

        do
        {
            len++;
            val = val * 10 + (*lan->str - '0');
            MovePtr (lan);
        } while (*lan->str >= '0' && *lan->str <= '9');

        TNode *node = CreateNode (val, TYPE_CONST, declared);
        node->len   = len;
        return node;
    }
    else switch (*lan->str)
    {
        case '!': [[fallthrough]];
        case '>': [[fallthrough]];
        case '=': [[fallthrough]];
        case '<':
            if (lan->str[1] == '=')
            {
                TNode *node = CreateNode (*lan->str + 2 * '=', TYPE_OP, declared);
                node->len = 2;
                lan->str++;
                MovePtr (lan);
                return node;
            }
            [[fallthrough]];
        case '^': [[fallthrough]];
        case '+': [[fallthrough]];
        case '-': [[fallthrough]];
        case '*': [[fallthrough]];
        case '/': [[fallthrough]];
        case ',': [[fallthrough]];
        case ':': [[fallthrough]];
        case ')': [[fallthrough]];
        case '(': [[fallthrough]];
        case '\"':
            {
                TNode *node = CreateNode (*lan->str, TYPE_OP, declared);
                node->len = 1;
                MovePtr (lan);
                return node;
            }
        case '?': [[fallthrough]];
        case '.':
            {
                TNode *node = CreateNode ('.', TYPE_ID, declared);
                node->len = 1;
                MovePtr (lan);
                return node;
            }
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
        LogMsg (" %ld\n", tok->data);
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
