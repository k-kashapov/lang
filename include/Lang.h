#include <stdarg.h>
#include "files.h"
#include "Tree.h"

#define SEMANTIC(cmd) cmd

const int INIT_IDS_NUM = 10;

const char NotAlpha[] = "0123456789-+!?*/ \"\n\t\r(){}[]^\\,.";
const int  AlphaNum   = sizeof (NotAlpha) / sizeof (char);

struct Id
{
    int64_t hash;
};

struct FuncId
{
    int64_t hash;
    int args_num;
};

struct Trans
{
    Id     *IdsArr;
    int    IdsNum;
    TNode  **s;
    FuncId *FuncArr;
    int     FuncsNum;
};

enum LANG_EXIT_CODES
{
    REQUIRE_FAIL  = 0x01,
    REDECLARATION = 0x02,
    UNDECLARED    = 0x03,
};

int64_t SimpleHash (const void *data, int len);

const int64_t UnaryFuncs[] =
{
    SimpleHash ("sin",  3),
    SimpleHash ("cos",  3),
    SimpleHash ("diff", 4)
};

const int UnaryNum = sizeof (UnaryFuncs) / sizeof (int64_t);

int IsAlpha (char val);

TNode *GetN (Trans *trans);

TNode *GetP (Trans *trans);

TNode *GetT (Trans *trans);

TNode *GetE (Trans *trans);

TNode *GetPow (Trans *trans);

TNode *Assn (Trans *trans);

TNode *GetG (Trans *trans);

TNode *GetFunc (Trans *trans);

TNode *GetIF (Trans *trans);

TNode *GetWhile (Trans *trans);

TNode *GetOP (Trans *trans);

int SyntaxErr (const char *msg, ...);

TNode *GetDec (Trans *trans);

TNode *ReadToken (const char *str);

TNode **LexicAnalysis (const char *string, int *nodesNum);

void OpenGraphFile (const char *name);

void PrintNodeDot (TNode *node);

int CreateNodeImage (TNode *node, const char *name);

void CloseGraphFile (void);

void FreeTransTree (TNode *root, TNode **nodes, int nodesNum);
