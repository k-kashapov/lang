#include "Lang.h"

static FILE *Graph_file = NULL;

#define COLOR_CASE(value, clr)                                                  \
    case value:                                                                 \
        color = clr;                                                            \
        break
#define OP_PRINT(value, to_print)                                               \
    case value:                                                                 \
        fprintf (Graph_file, to_print);                                         \
        break;

void OpenGraphFile (const char *name)
{
    Graph_file = fopen (name, "wt");
    if (!Graph_file)
    {
        printf ("Cannot open Graph_file, name = %s\n", name);
        return;
    }

    fprintf (Graph_file, "digraph G\n{rankdir = \"TB\";\nsplines = true;\n");

    return;
}

void CloseGraphFile (void)
{
    if (Graph_file)
    {
        fprintf (Graph_file, "}\n");
        fclose (Graph_file);
        Graph_file = NULL;
    }
    return;
}

static void LinkTreeNodes (TNode *src)
{
    if (src->left)
        fprintf (Graph_file, "NODE%p:sw->NODE%p:n ["
                             "minlen = \"2\"]\n", src, src->left);
    if (src->right)
        fprintf (Graph_file, "NODE%p:se->NODE%p:n ["
                             "minlen = \"2\"]\n", src, src->right);

    return;
}

void PrintNodeDot (TNode *node)
{
    const char *shape = "rectangle";
    const char *color = "red";

    switch (node->type)
    {
        case TYPE_STATEMENT:
            color = "gold";
            shape = "octagon";
            break;
        case TYPE_SERVICE: [[fallthrough]];
        case TYPE_ID:
            color = "lawngreen";
            shape = "invhouse";
            break;
        case TYPE_CONST:
            color = "purple";
            break;
        case TYPE_VAR:
            color = "pink";
            shape = "ellipse";
            break;
        case TYPE_UNARY:
            shape = "diamond";
            color = "plum1";
            break;
        case TYPE_OP:
            shape = "diamond";
            switch (node->data)
            {
                COLOR_CASE ('^', "tan");
                COLOR_CASE ('+', "lightgreen");
                COLOR_CASE ('-', "cornflowerblue");
                COLOR_CASE ('*', "lightcyan");
                COLOR_CASE ('/', "orange");
                COLOR_CASE ('!', "darkseagreen1");
                COLOR_CASE ('=', "aqua");
                COLOR_CASE (AE,  "darkorchid1");
                COLOR_CASE (BE,  "deeppink");
                COLOR_CASE (NE,  "mediumturquoise");
                COLOR_CASE (EE,  "plum1");
                COLOR_CASE (OR,  "lavender");
                COLOR_CASE (AND, "royalblue");
                default:
                    printf ("Graph build: Invalid operation: %ld, node %p\n", node->data, node);
            }
            break;
        default:
            printf ("Graph build (%d): Invalid node type: %d, node %p\n", __LINE__, node->type, node);
    }

    fprintf (Graph_file,
                "NODE%p"
                "["
                    "shape=%s, style = \"rounded,filled\", "
                    "fillcolor=\"%s\", "
                    "label = \"",
                    node, shape, color);

    switch (node->type)
    {
        case TYPE_SERVICE: [[fallthrough]];
        case TYPE_VAR:     [[fallthrough]];
        case TYPE_ID:
            {
                fprintf (Graph_file, "%.*s", node->len, node->declared);
                break;
            }
        case TYPE_STATEMENT:
            fprintf (Graph_file, "statement");
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
                        printf ("Graph build (%d): Invalid operation: %ld, "
                                "node %p\n", __LINE__, node->data, node);
                        break;
                }
            }
            break;
        case TYPE_UNARY:
            {
                int64_t data = node->data;
                fprintf (Graph_file, "%s", (char *)&data);
            }
            break;
        case TYPE_CONST:
            fprintf (Graph_file, "%ld\n", node->data);
            break;
        default:
            printf ("Graph build (%d): Invalid node type: %d, node %p\n", __LINE__, node->type, node);
    }

    fprintf (Graph_file, "\"]\n");

    LinkTreeNodes (node);

    return;
}

int CreateNodeImage (TNode *node, const char *name)
{
    OpenGraphFile ("dotInput.dot");
    VisitNode (node, NULL, PrintNodeDot, NULL);
    CloseGraphFile();

    char dot_command[100] = "dot dotInput.dot -Tpng -o img/";
    strcat (dot_command, name);
    if (system (dot_command)) exit (-1);

    char eog_command[100] = "eog img/";
    strcat (eog_command, name);

    system (eog_command);

    return OK;
}
