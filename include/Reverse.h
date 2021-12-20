#ifndef TREEANEK_H
#define TREEANEK_H

#include "Lang.h"

static FILE *AnekFile = NULL;
static int  IdsNum    = 0;
static Id   *IdsArr   = NULL;
static int  Tabs      = 0;
static Id   *ConstArr = NULL;
static int  ConstNum  = 0;

static void PrintA        (const char *msg, ...);
static int  PrintCallArgs (TNode *node);
static int  PrintCALL     (TNode *node);
static int  PrintRET      (TNode *node);
static int  PrintDEF      (TNode *node);
static int  PrintIN       (TNode *node);
static int  PrintOUT      (TNode *node);
static int  PrintNeg      (TNode *node);
static int  PrintIF       (TNode *node);
static int  PrintWHILE    (TNode *node);
static int  PrintSERV     (TNode *node);
static int  PrintOP       (TNode *node);
static int  PrintAssn     (TNode *node);
static int  PrintConst    (TNode *node);
static int  PrintID       (TNode *node);
static int  PrintVar      (TNode *node);
static int  PrintSt       (TNode *node);
static int  PrintUNARY    (TNode *node);
static int  NodeToAnek    (TNode *node);

#define CURR     node
#define LEFT     node->left
#define RIGHT    node->right
#define DATA     node->data
#define DECL     node->declared
#define LEN      node->len
#define TYPE     node->type
#define IDS      IdsArr
#define IDNUM    IdsNum

#define ANEK_IDS  &IDS,      &IDNUM
#define CONST_IDS &ConstArr, &ConstNum

#endif
