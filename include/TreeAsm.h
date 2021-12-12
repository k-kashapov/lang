#include "Lang.h"

static FILE *AsmFile   = NULL;
static int  IdsNum     = 0;
static Id   *IdsArr    = NULL;
static int  FreeOffset = 0;
static int  Tabs       = 0;

static int  NodeToAsm   (TNode *node);
static int  PrintSt     (TNode *node);
static int  PrintVar    (TNode *node);
static int  PrintID     (TNode *node);
static int  PrintDefine (TNode *node);
static int  PrintConst  (TNode *node);
static int  PrintOP     (TNode *node);
static int  PrintRet    (TNode *node);
static void PrintA      (const char *msg, ...);

const char *RES  = "rx"; /* a register for calculations result
                           i.e. function's return value        */

const char *FREE = "fx"; /* a register that holds free space
                            begin position                   */
#define CURR    node
#define LEFT    node->left
#define RIGHT   node->right
#define DATA    node->data
#define DECL    node->declared
#define LEN     node->len
#define TYPE    node->type
#define IDS     IdsArr
#define IDNUM   IdsNum
#define ASM_IDS &IDS, &IDNUM
#define SAVE()  PrintA ("pop %s\n", RES)
