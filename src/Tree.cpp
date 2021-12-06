#include "Tree.h"

const int POISON = 0x42;

Tree *CreateTree (tree_elem value)
{
    Tree *tree = (Tree*) calloc (1, sizeof (Tree));
    if (!tree)
    {
        printf ("CREATE TREE FAILED\n");
    }

    TNode *init_node = CreateNode (value);
    tree->root = init_node;

    TreeOk (tree);

    return tree;
}

TNode *GetRoot (Tree *tree)
{
    return tree->root;
}

TNode *CreateNode (tree_elem value, int type, const char *declared, TNode *left, TNode *right)
{
    TNode *node_ptr = (TNode *) calloc (1, sizeof (TNode));
    if (!node_ptr)
    {
        printf ("MEM_ALLOC_ERR\n");
    }

    node_ptr->data     = value;
    node_ptr->left     = left;
    node_ptr->right    = right;
    node_ptr->declared = declared;
    if (left)
    {
        left->parent = node_ptr;
    }
    if (right)
    {
        right->parent = node_ptr;
    }

    node_ptr->type   = type;

    return node_ptr;
}

TNode *AddNodeLeft (TNode *node, tree_elem value)
{
    TNode *node_ptr  = CreateNode (value);
    node->left       = node_ptr;
    node_ptr->parent = node;

    return node_ptr;
}

TNode *AddNodeRight (TNode *node, tree_elem value)
{
    TNode *node_ptr = CreateNode (value);
    node->right     = node_ptr;
    node_ptr->parent = node;

    return node_ptr;
}

void TreePrintLeftBracket (TNode *node)
{
    if (node)
        printf ("(");
    return;
}

void TreePrintRightBracket (TNode *node)
{
    if (node)
        printf (")");
    return;
}

void TreeNodePrint (TNode *node)
{
    printf (TYPE_SPEC, node->data);
    return;
}

int GetChildrenCount (TNode *node)
{
    int num = 1;
    if (node->type == TYPE_UNARY) num = 3;
    if (node->left) num += GetChildrenCount (node->left);
    if (node->right) num += GetChildrenCount (node->right);
    return num;
}

int NodesEqual (TNode *first, TNode *second)
{
    if (!IS_EQ_APPROX(first->data, second->data)) return 0;

    int equal = 1;

    if (first->left)
    {
        if (second->left)
        {
            equal *= NodesEqual (first->left, second->left);
        }
        else
        {
            return 0;
        }
    }

    if (first->right)
    {
        if (second->right)
        {
            equal *= NodesEqual (first->right, second->right);
        }
        else
        {
            return 0;
        }
    }

    return equal;
}

TNode *VisitNode (TNode *node, NodeAction pre, NodeAction in, NodeAction post)
{
    if (!node) return 0;

    if (pre)
        pre(node);

    if (node->left)
    {
        VisitNode (node->left, pre, in, post);
    }

    if (in)
        in (node);
    if (node->right)
    {
        VisitNode (node->right, pre, in, post);
    }

    if (post)
        post (node);

    return 0;
}

int64_t TreeOk (Tree *tree)
{
    int64_t err = 0;

    err |= NodeOk (GetRoot (tree));

    return err;
}

int64_t NodeOk (TNode *node)
{
    int64_t err = 0;
    if (!node)
    {
        printf ("BAD DATA PTR at %p\n", node);
        err |= BAD_PTR;
    }

    if (node->parent)
    {
        if (node != node->parent->left && node != node->parent->right)
        {
            printf ("NO PARRENT-CHILD CONNECTION at %p\n", node);
            err |= ORPHAN_NODE;
        }
    }

    if (node->left)
    {
        err |= NodeOk (node->left);
    }

    if (node->right)
    {
        err |= NodeOk (node->right);
    }

    return err;
}

int DestructNode (TNode *node)
{
    if (node->left)
    {
        DestructNode (node->left);
    }
    node->left = (TNode *) POISON;

    if (node->right)
    {
        DestructNode (node->right);
    }
    node->right = (TNode *) POISON;
    if (node->type != TYPE_DEAD)
    {
        node->type = TYPE_DEAD;
        free (node);
    }
    return 0;
}

int DestructTree (Tree *tree)
{
    DestructNode (GetRoot (tree));
    free (tree);

    return 0;
}
