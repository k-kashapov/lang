#ifndef LANG_H
#define LANG_H

#include "Logs.h"
#include <stdarg.h>
#include "files.h"
#include "Tree.h"

const int INIT_IDS_NUM = 10;

const char NotAlpha[] = "0123456789:-+!?*/ \"\n\t\r(){}[]^\\,.=";
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

#define SERVICE_HASH(val) SimpleHash (val, strlen (val))
const int64_t ServiceNodes[] =
{
    SERVICE_HASH ("Биба"),
    SERVICE_HASH ("Боба"),
    SERVICE_HASH ("define"),
    SERVICE_HASH ("function"),
    SERVICE_HASH ("parameter"),
    SERVICE_HASH ("call"),
    SERVICE_HASH ("if"),
    SERVICE_HASH ("decision"),
    SERVICE_HASH ("while"),
    SERVICE_HASH ("return")
};
#undef SERVICE_HASH

enum ServiceHash
{
    BIBA = 0,
    BOBA,
    DEF,
    FUNC,
    PARAM,
    CALL,
    IF,
    DECISION,
    WHILE,
    RET,
};

#define TRANS_IDS &trans->IdsArr, &trans->IdsNum

#define HASH_EQ(node, hash_num) (node->data == ServiceNodes[hash_num])
#define ST(l, r)                CreateNode (0, TYPE_STATEMENT, NULL, l, r)
#define IDEXISTS(target)        (FindId (TRANS_IDS, target) >= 0)

const int UnaryNum = sizeof (UnaryFuncs) / sizeof (int64_t);

TNode *BuildTreeFromBase (Config *io_config, const char **buffer);

int IsAlpha (char val);

TNode *CreateID (const char *id);

TNode *GetSt (Trans *trans, const char *end_cond);

TNode *GetN (Trans *trans);

TNode *GetP (Trans *trans);

TNode *GetT (Trans *trans);

TNode *GetE (Trans *trans);

TNode *GetPow (Trans *trans);

TNode *Assn (Trans *trans);

TNode *GetG (Trans *trans);

TNode *GetFunc (Trans *trans);

TNode *GetCall (Trans *trans);

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

void OpenBaseFile (const char *name);

int SaveNode (TNode *node, const char *name);

void CloseBaseFile (void);

int Translate (TNode *root, const char *name);

void FreeTransTree (TNode *root, TNode **nodes, int nodesNum);

int AddId (Id **IdsArr, int *IdsNum, int64_t hash);

int FindId (Id **IdsArr, int *IdsNum, int64_t hash);

int RmId (Id **IdsArr, int *IdsNum, int num = 1);

#endif
