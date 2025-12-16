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

typedef struct CFGNode {
  int id;
  char *label; /* printable label */
  char *role;  /* logical role: entry, if.then, ... */
  IntList succ;
  char **succ_labels;
  TextList ops; /* textual statements */
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

// write dot
void cfg_write_dot(CFG *c, FILE *f, const char *fname);

// build CFG for function node; returns 0 on success
int build_cfg_for_function(const char *source, TSNode func_node, CFG **out_cfg, char *out_fname, size_t fname_len, const char *file_prefix);

#endif
