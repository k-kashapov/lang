#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "files.h"
#include <stdarg.h>

#define SEMANTIC(cmd) cmd
const int MAX_STR_LEN  = 100;
const int INIT_IDS_NUM = 10;

struct Id
{
    int64_t hash;
    int     value;
};

struct Trans
{
    Id  *IdsArr;
    int IdsNum;
    char *s;
};

void SkipSpaces (Trans *trans);

void MovePtr (Trans *trans);

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
