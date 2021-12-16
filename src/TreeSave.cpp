#include "Lang.h"

static FILE *Base_file = NULL;

#define OP_PRINT(value, to_print)                                               \
    case value:                                                                 \
        fprintf (Base_file, to_print);                                          \
        break;

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
        case TYPE_SERVICE:
            fprintf (Base_file, "%.*s", node->len, node->declared);
            break;
        case TYPE_VAR: [[fallthrough]];
        case TYPE_ID:
            fprintf (Base_file, "\'%.*s\'", node->len, node->declared);
            break;
        case TYPE_STATEMENT:
            fprintf (Base_file, "statement");
            break;
        case TYPE_OP:
            {
                switch (node->data)
                {
                    OP_PRINT ('^', "^");
                    OP_PRINT ('+', "+");
                    OP_PRINT ('-', "-");
                    OP_PRINT ('*', "*");
                    OP_PRINT ('/', "/");
                    OP_PRINT ('!', "!");
                    OP_PRINT ('=', "=");
                    OP_PRINT (AE,  ">=");
                    OP_PRINT (BE,  "<=");
                    OP_PRINT (NE,  "!=");
                    OP_PRINT (EE,  "==");
                    OP_PRINT (OR,  "||");
                    OP_PRINT (AND, "&&");
                    default:
                        printf ("Save base (%d): Invalid operation: %ld, "
                                "node %p\n", __LINE__, node->data, node);
                        break;
                }
            }
            break;
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
            printf ("Save base (%d): Invalid node type: %d, node %p\n",
                    __LINE__, node->type, node);
    }

    return;
}

static void TreeToBase (TNode *node)
{
    fprintf (Base_file, "(");

    if (node->left)
    {
        TreeToBase (node->left);
    }

    PrintNodeToBase (node);

    if (node->right)
    {
        TreeToBase (node->right);
    }

    fprintf (Base_file, ")");

    return;
}

int SaveNode (TNode *node, const char *name)
{
    OpenBaseFile (name);
    TreeToBase (node);
    CloseBaseFile();

    char command[100] = "mousepad ./";
    strcat (command, name);
    system (command);

    return OK;
}
