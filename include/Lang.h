#include <stdarg.h>
#include "files.h"
#include "Tree.h"

#define SEMANTIC(cmd) cmd

const int INIT_IDS_NUM = 10;

const char NotAlpha[] = "0123456789-+!? \"\n\t\r(){}[]^\\,.";
const int  AlphaNum   = sizeof (NotAlpha) / sizeof (char);

struct Id
{
    int64_t hash;
};

struct Trans
{
    Id    *IdsArr;
    int   IdsNum;
    TNode **s;
};

enum LANG_EXIT_CODES
{
    REQUIRE_FAIL  = 0x01,
    REDECLARATION = 0x02,
    UNDECLARED    = 0x03,
};

int IsAlpha (char val);

TNode *GetN (Trans *trans);

TNode *GetP (Trans *trans);

TNode *GetT (Trans *trans);

TNode *GetE (Trans *trans);

int GetId (Trans *trans);

TNode *Assn (Trans *trans);

TNode *GetG (Trans *trans);

int GetOP (Trans *trans);

int SyntaxErr (const char *msg, ...);

int GetDec (Trans *trans);

TNode *ReadToken (const char *str);

TNode **LexicAnalysis (const char *string);

int64_t SimpleHash (const void *data, int len);
