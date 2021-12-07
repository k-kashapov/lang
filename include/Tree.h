#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

//#define TREE_DOUBLE
typedef int64_t tree_elem;

const double Epsilon = 1e-5;

#ifdef TREE_DOUBLE
    #define TYPE_SPEC "%lf"
    #define IS_EQ_APPROX(a, b) (abs(a - b) < Epsilon)
#else
    #define TYPE_SPEC "%ld"
    #define IS_EQ_APPROX(a, b) (a == b)
#endif

struct TNode
{
    tree_elem data;
    int type;
    const char *declared;
    int len;
    TNode *left;
    TNode *right;
    TNode *parent;
};

struct Tree
{
    TNode *root;
};

enum TREE_EXIT_CODES
{
    OK          = 0x0000,
    BAD_PTR     = 0x0001,
    ORPHAN_NODE = 0x0002,
};

enum TYPES
{
    TYPE_CONST     = 0x001,
    TYPE_ID        = 0x002,
    TYPE_OP        = 0x003,
    TYPE_UNARY     = 0x004,
    TYPE_VAR       = 0x005,
    TYPE_STATEMENT = 0x006,
    TYPE_FUNC      = 0x007,
    TYPE_DEAD      = 0xFFF,
};

typedef void (*NodeAction) (TNode *);

Tree *CreateTree (tree_elem value);

TNode *GetRoot (Tree *tree);

TNode *CreateNode (tree_elem value,
                   int   type  = 0,    const char *declared = NULL,
                   TNode *left = NULL, TNode      *right    = NULL);

TNode *AddNodeLeft (TNode *node, tree_elem value);

TNode *AddNodeRight (TNode *node, tree_elem value);

void TreePrintLeftBracket (TNode *node);

void TreePrintRightBracket (TNode *node);

void TreeNodePrint (TNode *node);

int GetChildrenCount (TNode *node);

int NodesEqual (TNode *first, TNode *second);

TNode *VisitNode (TNode *node, NodeAction pre, NodeAction in, NodeAction post);

int64_t TreeOk (Tree *tree);

int64_t NodeOk (TNode *node);

int DestructNode (TNode *node);

int DestructTree (Tree *tree);
