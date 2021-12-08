#include "Logs.h"
#include "Lang.h"

enum READ_ERRS
{
    CONST_READ_FAIL = 0x201,
    UNKNOWN_SYMBOL  = 0x202,
    LR_EXIST        = 0x203,
};

struct Reader
{
    const char *src;
    const char *src_start;
    TNode *curr_node;
    TNode *init_node;
};

int ProcessChar (Reader *rdr);

static int ProcessAlpha (Reader *rdr)
{
    int type = TYPE_ID;

    if (*rdr->src == '\"')
    {
        type = TYPE_VAR;
        rdr->src++;
    }

    const char *declared  = rdr->src;
    int64_t    hash       = 0;
    int        bytes_read = 0;

    while (IsAlpha (*rdr->src))
    {
        bytes_read++;
        printf ("reading char: %c\n", *rdr->src);
        hash += *rdr->src * bytes_read;
        rdr->src++;
    }

    rdr->curr_node->type = type;

    printf ("hash = %ld, simple = %ld;\n", hash, SimpleHash ("statement", 9));
    if (hash == SimpleHash ("statement", 9))
    {
        rdr->curr_node->type = TYPE_STATEMENT;
    }
    else
    {
        for (int unary = 0; unary < UnaryNum; unary++)
        {
            if (hash == UnaryFuncs[unary])
            {
                rdr->curr_node->type = TYPE_UNARY;
                break;
            }
        }
    }

    rdr->curr_node->len      = (int) (rdr->src - declared);
    rdr->curr_node->data     = hash;
    rdr->curr_node->declared = declared;

    if (type == TYPE_VAR && *rdr->src == '\"') rdr->src++; // skip \" after var name
    printf ("end variant: %.*s\n", rdr->curr_node->len, rdr->curr_node->declared);

    return ProcessChar (rdr);
}

static int ProcessNum (Reader *rdr)
{
    int64_t val    = 0;
    int bytes_read = 0;

    int read = sscanf (rdr->src, "%ld%n", &val, &bytes_read);
    if (!read)
    {
        LogErr ("Const value read err: src = %s", rdr->src);
        return CONST_READ_FAIL;
    }

    printf ("Const value read = %ld\n", val);

    rdr->curr_node->data     = val;
    rdr->curr_node->type     = TYPE_CONST;
    rdr->curr_node->declared = rdr->src;
    rdr->curr_node->len      = bytes_read;

    rdr->src += bytes_read;

    return ProcessChar (rdr);
}

static int ProcessSpecial (Reader *rdr)
{
    switch (*rdr->src)
    {
        case '(':
            if (!rdr->curr_node->left &&
                rdr->curr_node->type != TYPE_UNARY)
            {
                rdr->curr_node->left         = CreateNode (0);
                rdr->curr_node->left->parent = rdr->curr_node;
                rdr->curr_node               = rdr->curr_node->left;
            }
            else if (!rdr->curr_node->right)
            {
                rdr->curr_node->right         = CreateNode (0);
                rdr->curr_node->right->parent = rdr->curr_node;
                rdr->curr_node                = rdr->curr_node->right;
            }
            else
            {
                LogErr ("Node %p L and R already exist: %s\n", rdr->curr_node, rdr->src);
                return LR_EXIST;
            }
            break;
        case ')':
            if (!rdr->curr_node) rdr->curr_node = CreateNode (0);
            rdr->curr_node = rdr->curr_node->parent;
            break;
        case '^': [[fallthrough]];
        case '+': [[fallthrough]];
        case '-': [[fallthrough]];
        case '*': [[fallthrough]];
        case '=': [[fallthrough]];
        case '/':
            rdr->curr_node->data     = (tree_elem) (*rdr->src);
            rdr->curr_node->type     = TYPE_OP;
            rdr->curr_node->declared = rdr->src;
            rdr->curr_node->len      = 1;
            break;
        default:
            LogErr ("Unknown symbol: %c (%d)\n", *rdr->src, *rdr->src);
            return UNKNOWN_SYMBOL;
    }

    rdr->src++;

    return ProcessChar (rdr);
}

int ProcessChar (Reader *rdr)
{
    printf ("---------\ncurr char is %c (%d)\n", *rdr->src, *rdr->src);

    if (IsAlpha (*rdr->src) || *rdr->src == '\"')
    {
        $ return ProcessAlpha (rdr);
    }
    else if (*rdr->src >= '0' && *rdr->src <= '9')
    {
        $ return ProcessNum (rdr);
    }
    else
    {
        $ return ProcessSpecial (rdr);
    }
}

TNode *BuildTreeFromBase (Config *io_config, const char **buffer)
{
    char *source = read_file (io_config->input_file);
    if (!source)
    {
        LogErr ("in function: %s (%d) couldn't read source file\n",
                __FUNCTION__, __LINE__);
        return NULL;
    }

    long src_len = (long) strlen (source);
    printf ("Length = %ld\n", src_len);

    Reader rdr    = {};
    rdr.src       = source;
    rdr.src_start = source;
    rdr.curr_node = CreateNode (0, TYPE_STATEMENT);
    rdr.init_node = rdr.curr_node;

    $ int errors = ProcessChar (&rdr);
    if (errors)
    {
        LogMsg ("Errors were detected during file reading. Stopping...\n");
        CreateNodeImage (rdr.init_node, "fail.png");

        free (source);
        DestructNode (rdr.init_node);

        return NULL;
    }

    rdr.curr_node = rdr.init_node->left;
    free (rdr.init_node);

    CreateNodeImage (rdr.curr_node, "init_graph.png");

    *buffer = rdr.src_start;

    return rdr.curr_node;
}
