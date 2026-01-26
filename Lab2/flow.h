#ifndef LAB2_FLOW_H
#define LAB2_FLOW_H

#include <stdio.h>
#include <tree_sitter/api.h>

typedef struct IntList {
  int *a;
  int n;
  int cap;
} IntList;

typedef struct TextList {
  char **lines;
  int n_lines;
  int cap_lines;
} TextList;

typedef enum FlowOpKind {
  FLOW_OP_UNKNOWN,
  FLOW_OP_ASSIGN,
  FLOW_OP_EXPR,
  FLOW_OP_CALL,
  FLOW_OP_VARDECL,
  FLOW_OP_COND,
  FLOW_OP_RETURN,
  FLOW_OP_LOOP,
} FlowOpKind;

typedef enum BinaryKind {
  BIN_UNKNOWN,
  BIN_ADD,
  BIN_SUB,
  BIN_MUL,
  BIN_DIV,
} BinaryKind;

typedef enum CondKind {
  CONDK_UNKNOWN,
  CONDK_EQ,
  CONDK_NE,
  CONDK_LT,
  CONDK_LE,
  CONDK_GT,
  CONDK_GE,
} CondKind;

typedef enum ExprKind {
  EXPRK_UNKNOWN,
  EXPRK_IDENTIFIER,
  EXPRK_LITERAL,
  EXPRK_COMPLEX,
} ExprKind;

typedef struct ExprInfo {
  ExprKind kind;
  char *text;
  char *identifier;
  char *literal;
} ExprInfo;

typedef struct FlowOperation {
  FlowOpKind kind;
  ExprInfo lhs;
  ExprInfo rhs;
  ExprInfo cond;
  ExprInfo value;
  ExprInfo bin_left;
  ExprInfo bin_right;
  ExprInfo cond_left;
  ExprInfo cond_right;
  char *call_name;
  ExprInfo *call_args;
  int n_call_args;
  BinaryKind bin_kind;
  CondKind cond_kind;
} FlowOperation;

typedef struct CFGNode {
  int id;
  char *label; /* printable label */
  char *role;  /* logical role: entry, if.then, ... */
  IntList succ;
  char **succ_labels;
  TextList ops; /* textual statements */
  FlowOperation **flow_ops;
  int n_flow_ops;
  int cap_flow_ops;
} CFGNode;

typedef struct CFG {
  CFGNode *nodes;
  int n_nodes;
  int cap_nodes;
} CFG;

typedef struct ProgramFunction {
  char *name;
  char *signature;
  char *source_file;
  CFG *cfg;
} ProgramFunction;

// create/free
CFG *cfg_new(void);
void cfg_free(CFG *c);

// add node/edge
int cfg_add_node(CFG *c, const char *role);
void cfg_add_edge(CFG *c, int from, int to, const char *label);
void cfg_node_add_line(CFG *c, int node_id, const char *line);
void cfg_node_add_line_owned(CFG *c, int node_id, char *line);
void cfg_node_add_operation(CFG *c, int node_id, FlowOperation *op);

// write dot
void cfg_write_dot(CFG *c, FILE *f, const char *fname);

// build CFG for function node; returns 0 on success
int build_cfg_for_function(const char *source, TSNode func_node, CFG **out_cfg, char *out_fname, size_t fname_len, const char *file_prefix);

#endif
