#include "Lang.h"

static FILE *Base_file = NULL;

void OpenBaseFile (const char *name)
{
    Base_file = fopen (name, "wt");
    if (!Base_file)
    {
        printf ("Cannot open Base_file, name = %s\n", name);
        return;
    }

    return;
}

void CloseBaseFile (void)
{
    if (Base_file)
    {
        fclose (Base_file);
        Base_file = NULL;
    }
    return;
}

static void PrintNodeToBase (TNode *node)
{
    switch (node->type)
    {
        case TYPE_VAR:
            fprintf (Base_file, "\"%.*s\"", node->len, node->declared);
            break;
        case TYPE_FUNC: [[fallthrough]];
        case TYPE_ID:
            fprintf (Base_file, "%.*s", node->len, node->declared);
            break;
        case TYPE_STATEMENT:
            fprintf (Base_file, "statement");
            break;
        case TYPE_OP:    [[fallthrough]];
        case TYPE_UNARY:
            {
                int64_t data = node->data;
                fprintf (Base_file, "%s", (char *)&data);
            }
            break;
        case TYPE_CONST:
            fprintf (Base_file, "%ld", node->data);
            break;
        default:
            printf ("Graph build (%d): Invalid node type: %d, node %p\n",
                    __LINE__, node->type, node);
    }

    return;
}

static void BasePrintLBracket (TNode *node)
{
    if (node)
    {
        fprintf (Base_file, "(");
    }
}

static void BasePrintRBracket (TNode *node)
{
    if (node)
    {
        fprintf (Base_file, ")");
    }
}

int SaveNode (TNode *node, const char *name)
{
    OpenBaseFile (name);
    VisitNode (node, BasePrintLBracket, PrintNodeToBase, BasePrintRBracket);
    CloseBaseFile();

    char command[100] = "mousepad ./";
    strcat (command, name);
    system (command);

    return OK;
}
