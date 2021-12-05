#include <stdarg.h>
#include "files.h"
#include "Tree.h"

#define SEMANTIC(cmd) cmd

const int INIT_IDS_NUM = 10;

const char NotAlpha[] = "0123456789-!? \n\t\r(){}[]^\\,.";
const int  AlphaNum   = sizeof (NotAlpha) / sizeof (char);

struct Id
{
    int64_t hash;
    int     value;
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

void SkipSpaces (Trans *trans);

void MovePtr (Trans *trans);

int CheckString (Trans *trans, const char *str);

int GetN (Trans *trans);

int GetP (Trans *trans);

int GetT (Trans *trans);

int GetE (Trans *trans);

int GetId (Trans *trans);

int Assn (Trans *trans);

int Require (Trans *trans, const char* req);

int GetG (Trans *trans);

int GetOP (Trans *trans);

int SyntaxErr (const char *msg, ...);

int GetDec (Trans *trans);

TNode *ReadToken (const char *str);

TNode **LexicAnalysis (const char *string);
