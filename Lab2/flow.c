#include "flow.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

static void intlist_init(IntList *l) { l->a = NULL; l->n = 0; l->cap = 0; }
static void intlist_push(IntList *l, int v) {
  if (l->n + 1 > l->cap) {
    l->cap = (l->cap == 0) ? 8 : l->cap * 2;
    l->a = realloc(l->a, sizeof(int) * l->cap);
  }
  l->a[l->n++] = v;
}
static void intlist_free(IntList *l) { free(l->a); l->a = NULL; l->n = l->cap = 0; }

static void textlist_init(TextList *t) { t->lines = NULL; t->n_lines = 0; t->cap_lines = 0; }
static void textlist_add_owned(TextList *t, char *line) {
  if (!line) return;
  if (t->n_lines + 1 > t->cap_lines) {
    t->cap_lines = (t->cap_lines == 0) ? 4 : t->cap_lines * 2;
    t->lines = realloc(t->lines, sizeof(char*) * t->cap_lines);
  }
  t->lines[t->n_lines++] = line;
}
static void textlist_add(TextList *t, const char *line) {
  if (!line) return;
  textlist_add_owned(t, strdup(line));
}
static void textlist_clear(TextList *t) {
  for (int i=0;i<t->n_lines;i++) free(t->lines[i]);
  free(t->lines);
  t->lines = NULL; t->n_lines = t->cap_lines = 0;
}

CFG *cfg_new(void) {
  CFG *c = malloc(sizeof(CFG));
  c->nodes = NULL; c->n_nodes = 0; c->cap_nodes = 0;
  return c;
}

void cfg_free(CFG *c) {
  if (!c) return;
  for (int i=0;i<c->n_nodes;i++) {
    free(c->nodes[i].label);
    free(c->nodes[i].role);
    textlist_clear(&c->nodes[i].ops);
    if (c->nodes[i].succ_labels) {
      for (int j=0;j<c->nodes[i].succ.n;j++) free(c->nodes[i].succ_labels[j]);
      free(c->nodes[i].succ_labels);
    }
    intlist_free(&c->nodes[i].succ);
  }
  free(c->nodes);
  free(c);
}

int cfg_add_node(CFG *c, const char *role) {
  if (c->n_nodes + 1 > c->cap_nodes) {
    c->cap_nodes = (c->cap_nodes == 0) ? 8 : c->cap_nodes * 2;
    c->nodes = realloc(c->nodes, sizeof(CFGNode) * c->cap_nodes);
  }
  int id = c->n_nodes++;
  c->nodes[id].id = id;
  const char *r = role ? role : "block";
  c->nodes[id].role = strdup(r);
  c->nodes[id].label = strdup(r);
  intlist_init(&c->nodes[id].succ);
  c->nodes[id].succ_labels = NULL;
  textlist_init(&c->nodes[id].ops);
  return id;
}

void cfg_add_edge(CFG *c, int from, int to, const char *label) {
  if (!c) return;
  if (from < 0 || from >= c->n_nodes) return;
  if (to < 0 || to >= c->n_nodes) return;
  CFGNode *n = &c->nodes[from];
  if (n->succ.n + 1 > n->succ.cap) {
    int newcap = (n->succ.cap == 0) ? 4 : n->succ.cap * 2;
    n->succ.a = realloc(n->succ.a, sizeof(int) * newcap);
    n->succ_labels = realloc(n->succ_labels, sizeof(char*) * newcap);
    n->succ.cap = newcap;
  }
  n->succ.a[n->succ.n] = to;
  n->succ_labels[n->succ.n] = label ? strdup(label) : NULL;
  n->succ.n++;
}

void cfg_node_add_line(CFG *c, int node_id, const char *line) {
  if (!c || node_id < 0 || node_id >= c->n_nodes) return;
  textlist_add(&c->nodes[node_id].ops, line);
}

void cfg_node_add_line_owned(CFG *c, int node_id, char *line) {
  if (!c || node_id < 0 || node_id >= c->n_nodes) { free(line); return; }
  textlist_add_owned(&c->nodes[node_id].ops, line);
}

static void dot_escape(FILE *f, const char *s) {
  for (; *s; ++s) {
    unsigned char ch = (unsigned char)*s;
    if (ch == '"' || ch == '\\') fprintf(f, "\\%c", ch);
    else if (ch == '\n') fputs("\\n", f);
    else if (ch == '\r') fputs("\\r", f);
    else fputc(ch, f);
  }
}

void cfg_write_dot(CFG *c, FILE *f, const char *fname) {
  fprintf(f, "digraph CFG_%s {\n", fname);
  for (int i=0;i<c->n_nodes;i++) {
    fprintf(f, "  n%d [label=\"", c->nodes[i].id);
    dot_escape(f, c->nodes[i].label);
    for (int j=0;j<c->nodes[i].ops.n_lines;j++) {
      fputs("\\n", f);
      dot_escape(f, c->nodes[i].ops.lines[j]);
    }
    fputs("\"];\n", f);
  }
  for (int i=0;i<c->n_nodes;i++) {
    for (int j=0;j<c->nodes[i].succ.n;j++) {
      char *lab = (c->nodes[i].succ_labels) ? c->nodes[i].succ_labels[j] : NULL;
      if (lab) fprintf(f, "  n%d -> n%d [label=\"%s\"];\n", i, c->nodes[i].succ.a[j], lab);
      else fprintf(f, "  n%d -> n%d;\n", i, c->nodes[i].succ.a[j]);
    }
  }
  fprintf(f, "}\n");
}

static char *dup_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (len < 0) return NULL;
  char *buf = malloc((size_t)len + 1);
  if (!buf) return NULL;
  va_start(ap, fmt);
  vsnprintf(buf, (size_t)len + 1, fmt, ap);
  va_end(ap);
  return buf;
}

static char *node_text_trimmed(const char *src, TSNode node) {
  uint32_t s = ts_node_start_byte(node);
  uint32_t e = ts_node_end_byte(node);
  if (e <= s) return strdup("");
  size_t len = (size_t)(e - s);
  while (len > 0 && isspace((unsigned char)src[s])) { s++; len--; }
  while (len > 0 && isspace((unsigned char)src[s+len-1])) len--;
  char *txt = malloc(len + 1);
  memcpy(txt, src + s, len);
  txt[len] = '\0';
  return txt;
}

static int has_operator_char(const char *s) {
  if (!s) return 0;
  while (*s) {
    if (strchr("+-*/%&|^=!<>", *s)) return 1;
    s++;
  }
  return 0;
}

static char *summarize_expr(const char *txt) {
  if (!txt || !*txt) return strdup("expr");
  size_t len = strlen(txt);
  if (len <= 18) return strdup(txt);
  if (strchr(txt, '(') && len <= 28) return strdup(txt);
  if (has_operator_char(txt)) return strdup("complex_expr");
  return strdup("expr");
}

#define MAX_IR_DEPTH 4
#define MAX_BLOCK_LINES 3

typedef void (*VarDeclEmitter)(const char *type_txt, const char *name, void *userdata);

typedef struct {
  CFG *cfg;
  int node_id;
} VarDeclNodeCtx;

static void for_each_var_decl_entry(const char *src, TSNode stmt, VarDeclEmitter cb, void *userdata) {
  if (!cb) return;
  uint32_t cc = ts_node_child_count(stmt);
  if (cc == 0) return;
  TSNode id_list = ts_node_child(stmt, 0);
  TSNode type_node = {0};
  for (uint32_t i=1;i<cc;i++) {
    TSNode child = ts_node_child(stmt, i);
    if (strcmp(ts_node_type(child), "typeRef") == 0) { type_node = child; break; }
  }
  char *type_txt = type_node.id ? node_text_trimmed(src, type_node) : strdup("auto");
  int emitted = 0;
  if (!ts_node_is_null(id_list)) {
    uint32_t ic = ts_node_child_count(id_list);
    for (uint32_t i=0;i<ic;i++) {
      TSNode child = ts_node_child(id_list, i);
      if (!ts_node_is_named(child)) continue;
      if (strcmp(ts_node_type(child), "identifier") == 0) {
        char *name = node_text_trimmed(src, child);
        cb(type_txt, name, userdata);
        free(name);
        emitted = 1;
      }
    }
  }
  if (!emitted) {
    char *fallback = node_text_trimmed(src, stmt);
    cb(type_txt, fallback, userdata);
    free(fallback);
  }
  free(type_txt);
}

static void emit_var_decl_to_textlist(const char *type_txt, const char *name, void *userdata) {
  TextList *seq = userdata;
  char *line = dup_printf("VarDecl(%s)\n  var: %s", type_txt, name);
  textlist_add_owned(seq, line);
}

static void emit_var_decl_to_node(const char *type_txt, const char *name, void *userdata) {
  VarDeclNodeCtx *ctx = userdata;
  char *line = dup_printf("VarDecl(%s)\n  var: %s", type_txt, name);
  cfg_node_add_line_owned(ctx->cfg, ctx->node_id, line);
}

static char *join_textlist(const TextList *t, const char *sep) {
  if (!t || t->n_lines == 0) return strdup("");
  size_t sep_len = strlen(sep);
  size_t total = 1;
  for (int i=0;i<t->n_lines;i++) total += strlen(t->lines[i]);
  total += sep_len * (t->n_lines - 1);
  char *buf = malloc(total);
  buf[0] = '\0';
  for (int i=0;i<t->n_lines;i++) {
    if (i > 0) strcat(buf, sep);
    strcat(buf, t->lines[i]);
  }
  return buf;
}

static int extract_identifier_name(const char *src, TSNode node, char *out, size_t out_len) {
  if (ts_node_is_null(node) || !out || out_len == 0) return 0;
  const char *t = ts_node_type(node);
  if (strcmp(t, "identifier") == 0) {
    char *txt = node_text_trimmed(src, node);
    snprintf(out, out_len, "%s", txt);
    free(txt);
    return 1;
  }
  uint32_t cc = ts_node_child_count(node);
  for (uint32_t i=0;i<cc;i++) {
    if (extract_identifier_name(src, ts_node_child(node, i), out, out_len)) return 1;
  }
  return 0;
}

static char *format_expr_ir(const char *src, TSNode node, int depth);
static char *format_postfix_ir(const char *src, TSNode node, int depth);
static char *format_call_ir(const char *src, TSNode callee, TSNode args_node, int depth);
static char *format_index_ir(const char *src, TSNode base_node, TSNode args_node, int depth);
static char *format_unary_ir(const char *src, TSNode node, int depth);
static char *format_binary_chain(const char *src, TSNode node, const char *kind, int depth);

static char *format_binary_chain(const char *src, TSNode node, const char *kind, int depth) {
  uint32_t cc = ts_node_child_count(node);
  if (cc == 0) return strdup("...");
  int operands = 0;
  for (uint32_t i=0;i<cc;i++) if (ts_node_is_named(ts_node_child(node, i))) operands++;
  if (operands == 0) return strdup("...");
  if (operands == 1) {
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(node, i);
      if (!ts_node_is_named(child)) continue;
      return format_expr_ir(src, child, depth);
    }
  }
  char *acc = NULL;
  int seen = 0;
  for (uint32_t i=0;i<cc;i++) {
    TSNode child = ts_node_child(node, i);
    if (!ts_node_is_named(child)) continue;
    if (seen == 0) {
      acc = format_expr_ir(src, child, depth + 1);
      seen = 1;
      continue;
    }
    char *rhs = format_expr_ir(src, child, depth + 1);
    char *combined = dup_printf("BinaryOp(%s) { %s | %s }", kind, acc, rhs);
    free(acc); free(rhs);
    acc = combined;
    seen++;
  }
  if (!acc) acc = strdup("...");
  return acc;
}

static char *format_unary_ir(const char *src, TSNode node, int depth) {
  uint32_t cc = ts_node_child_count(node);
  if (cc == 0) return strdup("...");
  TSNode first = ts_node_child(node, 0);
  if (!ts_node_is_named(first)) {
    char *op_txt = node_text_trimmed(src, first);
    TSNode operand = (cc > 1) ? ts_node_child(node, 1) : (TSNode){0};
    char *arg = format_expr_ir(src, operand, depth + 1);
    char *line = dup_printf("UnaryOp(%s) { %s }", op_txt, arg);
    free(op_txt); free(arg);
    return line;
  }
  return format_expr_ir(src, first, depth);
}

static char *format_call_ir(const char *src, TSNode callee, TSNode args_node, int depth) {
  char name_buf[64];
  if (!extract_identifier_name(src, callee, name_buf, sizeof(name_buf))) {
    char *raw = node_text_trimmed(src, callee);
    char *short_txt = summarize_expr(raw);
    snprintf(name_buf, sizeof(name_buf), "%s", short_txt);
    free(raw); free(short_txt);
  }
  TextList args; textlist_init(&args);
  if (!ts_node_is_null(args_node)) {
    uint32_t cc = ts_node_child_count(args_node);
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(args_node, i);
      if (!ts_node_is_named(child)) continue;
      char *arg = format_expr_ir(src, child, depth + 1);
      textlist_add_owned(&args, arg);
    }
  }
  char *joined = NULL;
  if (args.n_lines > 0) joined = join_textlist(&args, " | ");
  char *line = (joined && *joined)
                 ? dup_printf("Call(%s) { %s }", name_buf, joined)
                 : dup_printf("Call(%s) { }", name_buf);
  free(joined);
  textlist_clear(&args);
  return line;
}

static char *format_index_ir(const char *src, TSNode base_node, TSNode args_node, int depth) {
  char *base = format_expr_ir(src, base_node, depth + 1);
  char *index_str = NULL;
  if (!ts_node_is_null(args_node)) {
    TextList idx; textlist_init(&idx);
    uint32_t cc = ts_node_child_count(args_node);
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(args_node, i);
      if (!ts_node_is_named(child)) continue;
      char *arg = format_expr_ir(src, child, depth + 1);
      textlist_add_owned(&idx, arg);
    }
    if (idx.n_lines == 1) {
      index_str = idx.lines[0];
      idx.lines[0] = NULL;
    } else if (idx.n_lines > 1) {
      char *joined = join_textlist(&idx, " | ");
      index_str = dup_printf("Tuple { %s }", joined);
      free(joined);
    }
    textlist_clear(&idx);
  }
  if (!index_str) index_str = strdup("...");
  char *line = dup_printf("BinaryOp(IndexExpr) { %s | %s }", base, index_str);
  free(base); free(index_str);
  return line;
}

static char *format_postfix_ir(const char *src, TSNode node, int depth) {
  uint32_t cc = ts_node_child_count(node);
  if (cc == 0) return strdup("...");
  TSNode callee = ts_node_child(node, 0);
  for (uint32_t i=1;i<cc;i++) {
    TSNode child = ts_node_child(node, i);
    const char *ct = ts_node_type(child);
    if (strcmp(ct, "(") == 0) {
      TSNode args = {0};
      for (uint32_t j=i+1;j<cc;j++) {
        TSNode maybe = ts_node_child(node, j);
        if (strcmp(ts_node_type(maybe), "exprList") == 0) { args = maybe; break; }
      }
      return format_call_ir(src, callee, args, depth + 1);
    }
    if (strcmp(ct, "[") == 0) {
      TSNode idx = {0};
      for (uint32_t j=i+1;j<cc;j++) {
        TSNode maybe = ts_node_child(node, j);
        if (strcmp(ts_node_type(maybe), "exprList") == 0) { idx = maybe; break; }
      }
      return format_index_ir(src, callee, idx, depth + 1);
    }
  }
  return format_expr_ir(src, callee, depth);
}

static char *format_expr_ir(const char *src, TSNode node, int depth) {
  if (depth > MAX_IR_DEPTH || ts_node_is_null(node)) return strdup("...");
  const char *t = ts_node_type(node);
  if (strcmp(t, "expr") == 0 || strcmp(t, "_expr") == 0) {
    uint32_t cc = ts_node_child_count(node);
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(node, i);
      if (!ts_node_is_named(child)) continue;
      return format_expr_ir(src, child, depth);
    }
    return strdup("...");
  }
  if (strcmp(t, "identifier") == 0) {
    char *txt = node_text_trimmed(src, node);
    char *line = dup_printf("Nop(Identifier) [var:%s]", txt);
    free(txt);
    return line;
  }
  if (strcmp(t, "literal") == 0 || strcmp(t, "bool") == 0 || strcmp(t, "str") == 0 ||
      strcmp(t, "char") == 0 || strcmp(t, "hex") == 0 || strcmp(t, "bits") == 0 ||
      strcmp(t, "dec") == 0) {
    char *txt = node_text_trimmed(src, node);
    char *line = dup_printf("Nop(Literal) [const:%s]", txt);
    free(txt);
    return line;
  }
  if (strcmp(t, "primary") == 0) {
    uint32_t cc = ts_node_child_count(node);
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(node, i);
      if (!ts_node_is_named(child)) continue;
      return format_expr_ir(src, child, depth + 1);
    }
    char *txt = node_text_trimmed(src, node);
    char *line = dup_printf("Expr(%s)", txt);
    free(txt);
    return line;
  }
  if (strcmp(t, "postfix") == 0) return format_postfix_ir(src, node, depth);
  if (strcmp(t, "unary") == 0) return format_unary_ir(src, node, depth);
  if (strcmp(t, "logical_or") == 0 || strcmp(t, "logical_and") == 0)
    return format_binary_chain(src, node, "LogicExpr", depth);
  if (strcmp(t, "bitwise_or") == 0 || strcmp(t, "bitwise_xor") == 0 ||
      strcmp(t, "bitwise_and") == 0 || strcmp(t, "shift") == 0)
    return format_binary_chain(src, node, "BitwiseExpr", depth);
  if (strcmp(t, "equality") == 0 || strcmp(t, "relational") == 0)
    return format_binary_chain(src, node, "CompareExpr", depth);
  if (strcmp(t, "add") == 0)
    return format_binary_chain(src, node, "AddExpr", depth);
  if (strcmp(t, "mul") == 0)
    return format_binary_chain(src, node, "MulExpr", depth);
  char *txt = node_text_trimmed(src, node);
  char *short_txt = summarize_expr(txt);
  char *line = dup_printf("Expr(%s)", short_txt);
  free(txt); free(short_txt);
  return line;
}

static char *format_assignment_ir(const char *src, TSNode stmt) {
  TSNode lhs = ts_node_child(stmt, 0);
  TSNode rhs = ts_node_child(stmt, 2);
  char *lhs_ir = format_expr_ir(src, lhs, 0);
  char *rhs_ir = format_expr_ir(src, rhs, 0);
  char *line = dup_printf("Assign(=)\n  lhs: %s\n  rhs: %s", lhs_ir, rhs_ir);
  free(lhs_ir); free(rhs_ir);
  return line;
}

static char *format_expr_stmt_ir(const char *src, TSNode stmt) {
  if (ts_node_child_count(stmt) == 0) return strdup("Expr");
  TSNode expr = ts_node_child(stmt, 0);
  char *expr_txt = format_expr_ir(src, expr, 0);
  char *line = dup_printf("ExprStmt\n  expr: %s", expr_txt);
  free(expr_txt);
  return line;
}

static char *format_return_line(const char *src, TSNode stmt) {
  uint32_t cc = ts_node_child_count(stmt);
  for (uint32_t i=0;i<cc;i++) {
    TSNode child = ts_node_child(stmt, i);
    if (!ts_node_is_named(child)) continue;
    char *expr_ir = format_expr_ir(src, child, 0);
    char *line = dup_printf("Return\n  value: %s", expr_ir);
    free(expr_ir);
    return line;
  }
  return strdup("Return");
}

static int is_wrapper(const char *t) {
  return strcmp(t,"statement")==0 || strcmp(t,"statements")==0 ||
         strcmp(t,"statement_list")==0 || strcmp(t,"statement_seq")==0;
}

static int is_simple_stmt(const char *t) {
  return strcmp(t,"assignment")==0 || strcmp(t,"expr_stmt")==0 || strcmp(t,"varDecl")==0;
}

typedef struct Builder {
  const char *source;
  CFG *cfg;
  int loop_exit_stack[32];
  int loop_cond_stack[32];
  int loop_depth;
  int func_exit;
} Builder;

static void push_loop(Builder *b, int cond_id, int exit_id) {
  if (b->loop_depth < 32) {
    b->loop_cond_stack[b->loop_depth] = cond_id;
    b->loop_exit_stack[b->loop_depth] = exit_id;
    b->loop_depth++;
  }
}
static void pop_loop(Builder *b) { if (b->loop_depth > 0) b->loop_depth--; }
static int current_loop_exit(Builder *b) { if (b->loop_depth==0) return -1; return b->loop_exit_stack[b->loop_depth-1]; }
static int current_loop_cond(Builder *b) { if (b->loop_depth==0) return -1; return b->loop_cond_stack[b->loop_depth-1]; }

static void process_statement(Builder *b, TSNode stmt, const char *role_hint, int *out_entry, int *out_exit);
static void append_simple_stmt_lines(Builder *b, TSNode stmt, TextList *seq);

static void append_simple_stmt_lines(Builder *b, TSNode stmt, TextList *seq) {
  const char *t = ts_node_type(stmt);
  if (strcmp(t, "assignment") == 0) {
    textlist_add_owned(seq, format_assignment_ir(b->source, stmt));
    return;
  }
  if (strcmp(t, "expr_stmt") == 0) {
    textlist_add_owned(seq, format_expr_stmt_ir(b->source, stmt));
    return;
  }
  if (strcmp(t, "varDecl") == 0) {
    for_each_var_decl_entry(b->source, stmt, emit_var_decl_to_textlist, seq);
    return;
  }
}

static void flush_seq_block(Builder *b, TextList *seq, int *first, int *last_exit, const char *first_role, int *block_count) {
  if (seq->n_lines == 0) return;
  const char *role = (*block_count == 0 && first_role) ? first_role : "block";
  int node = cfg_add_node(b->cfg, role);
  for (int i=0;i<seq->n_lines;i++) cfg_node_add_line_owned(b->cfg, node, seq->lines[i]);
  free(seq->lines); seq->lines = NULL; seq->n_lines = seq->cap_lines = 0;
  if (*first == -1) *first = node;
  if (*last_exit >= 0) cfg_add_edge(b->cfg, *last_exit, node, NULL);
  *last_exit = node;
  (*block_count)++;
}

static void process_block(Builder *b, TSNode block_node, const char *first_role, int *out_entry, int *out_exit) {
  TextList seq; textlist_init(&seq);
  int first = -1, last = -1, block_count = 0;
  uint32_t cc = ts_node_child_count(block_node);
  for (uint32_t i=0;i<cc;i++) {
    TSNode child = ts_node_child(block_node, i);
    const char *t = ts_node_type(child);
    if (strcmp(t,"begin")==0 || strcmp(t,"end")==0 || strcmp(t,";")==0) continue;
    if (is_wrapper(t)) {
      uint32_t wc = ts_node_child_count(child);
      for (uint32_t j=0;j<wc;j++) {
        TSNode wchild = ts_node_child(child, j);
        if (!ts_node_is_named(wchild)) continue;
        const char *wt = ts_node_type(wchild);
        if (is_simple_stmt(wt)) {
          append_simple_stmt_lines(b, wchild, &seq);
          if (seq.n_lines >= MAX_BLOCK_LINES) flush_seq_block(b, &seq, &first, &last, first_role, &block_count);
        } else {
          flush_seq_block(b, &seq, &first, &last, first_role, &block_count);
          int se=-1,sx=-1;
          process_statement(b, wchild, NULL, &se, &sx);
          if (se >= 0) {
            if (first == -1) first = se;
            if (last >= 0) cfg_add_edge(b->cfg, last, se, NULL);
          }
          if (sx == -1) last = -2;
          else last = sx;
        }
      }
      continue;
    }
    if (is_simple_stmt(t)) {
      append_simple_stmt_lines(b, child, &seq);
      if (seq.n_lines >= MAX_BLOCK_LINES) flush_seq_block(b, &seq, &first, &last, first_role, &block_count);
      continue;
    }
    flush_seq_block(b, &seq, &first, &last, first_role, &block_count);
    int se=-1,sx=-1;
    process_statement(b, child, NULL, &se, &sx);
    if (se >= 0) {
      if (first == -1) first = se;
      if (last >= 0) cfg_add_edge(b->cfg, last, se, NULL);
    }
    if (sx == -1) last = -2;
    else last = sx;
  }
  flush_seq_block(b, &seq, &first, &last, first_role, &block_count);
  textlist_clear(&seq);
  if (first == -1) {
    first = cfg_add_node(b->cfg, first_role ? first_role : "block");
    cfg_node_add_line(b->cfg, first, "empty");
    last = first;
  }
  if (last == -2) last = -1;
  *out_entry = first;
  *out_exit = last;
}

static void process_statement(Builder *b, TSNode stmt, const char *role_hint, int *out_entry, int *out_exit) {
  const char *t = ts_node_type(stmt);
  if (strcmp(t,"block")==0) { process_block(b, stmt, role_hint, out_entry, out_exit); return; }
  if (is_wrapper(t)) {
    int first = -1, last = -1;
    uint32_t cc = ts_node_child_count(stmt);
    for (uint32_t i=0;i<cc;i++) {
      TSNode child = ts_node_child(stmt, i);
      if (!ts_node_is_named(child)) continue;
      int se=-1,sx=-1;
      process_statement(b, child, NULL, &se, &sx);
      if (se >= 0) {
        if (first == -1) first = se;
        if (last >= 0) cfg_add_edge(b->cfg, last, se, NULL);
      }
      if (sx == -1) last = -2;
      else last = sx;
    }
    if (first == -1) {
      first = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
      cfg_node_add_line(b->cfg, first, "empty");
      last = first;
    }
    if (last == -2) last = -1;
    *out_entry = first; *out_exit = last;
    return;
  }
  if (strcmp(t,"assignment")==0) {
    char *line = format_assignment_ir(b->source, stmt);
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    cfg_node_add_line_owned(b->cfg, node, line);
    *out_entry = *out_exit = node;
    return;
  }
  if (strcmp(t,"expr_stmt")==0) {
    char *line = format_expr_stmt_ir(b->source, stmt);
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    cfg_node_add_line_owned(b->cfg, node, line);
    *out_entry = *out_exit = node;
    return;
  }
  if (strcmp(t,"varDecl")==0) {
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    VarDeclNodeCtx ctx = { b->cfg, node };
    for_each_var_decl_entry(b->source, stmt, emit_var_decl_to_node, &ctx);
    *out_entry = *out_exit = node;
    return;
  }
  if (strcmp(t,"if_statement")==0) {
    TSNode cond = ts_node_child(stmt, 1);
    TSNode then_stmt = ts_node_child(stmt, 3);
    TSNode else_stmt = (ts_node_child_count(stmt) > 4) ? ts_node_child(stmt, 5) : (TSNode){0};
    char *cond_ir = format_expr_ir(b->source, cond, 0);
    int cond_id = cfg_add_node(b->cfg, "if.cond");
    cfg_node_add_line_owned(b->cfg, cond_id, dup_printf("IfCond\n  expr: %s", cond_ir));
    free(cond_ir);

    int then_entry=-1, then_exit=-1;
    process_statement(b, then_stmt, "if.then", &then_entry, &then_exit);
    if (then_entry < 0) {
      then_entry = cfg_add_node(b->cfg, "if.then");
      cfg_node_add_line(b->cfg, then_entry, "empty");
      then_exit = then_entry;
    }
    int else_entry=-1, else_exit=-1;
    if (ts_node_is_null(else_stmt)) {
      else_entry = cfg_add_node(b->cfg, "if.else");
      cfg_node_add_line(b->cfg, else_entry, "empty");
      else_exit = else_entry;
    } else {
      process_statement(b, else_stmt, "if.else", &else_entry, &else_exit);
      if (else_entry < 0) {
        else_entry = cfg_add_node(b->cfg, "if.else");
        cfg_node_add_line(b->cfg, else_entry, "empty");
        else_exit = else_entry;
      }
    }
    int join = cfg_add_node(b->cfg, "if.join");
    cfg_node_add_line(b->cfg, join, "join");

    cfg_add_edge(b->cfg, cond_id, then_entry, "true");
    cfg_add_edge(b->cfg, cond_id, else_entry, "false");
    if (then_exit >= 0) cfg_add_edge(b->cfg, then_exit, join, NULL);
    if (else_exit >= 0) cfg_add_edge(b->cfg, else_exit, join, NULL);
    *out_entry = cond_id; *out_exit = join;
    return;
  }
  if (strcmp(t,"while_statement")==0) {
    TSNode cond = ts_node_child(stmt, 1);
    TSNode body = ts_node_child(stmt, 3);
    char *cond_ir = format_expr_ir(b->source, cond, 0);
    int cond_id = cfg_add_node(b->cfg, "while.cond");
    cfg_node_add_line_owned(b->cfg, cond_id, dup_printf("WhileCond\n  expr: %s", cond_ir));
    free(cond_ir);
    int exit_id = cfg_add_node(b->cfg, "after_while");
    cfg_node_add_line(b->cfg, exit_id, "Nop(exit)");
    push_loop(b, cond_id, exit_id);

    int body_entry=-1, body_exit=-1;
    process_statement(b, body, "while.body", &body_entry, &body_exit);
    if (body_entry < 0) {
      body_entry = cfg_add_node(b->cfg, "while.body");
      cfg_node_add_line(b->cfg, body_entry, "empty");
      body_exit = body_entry;
    }

    cfg_add_edge(b->cfg, cond_id, body_entry, "true");
    if (body_exit >= 0) cfg_add_edge(b->cfg, body_exit, cond_id, NULL);
    cfg_add_edge(b->cfg, cond_id, exit_id, "false");
    pop_loop(b);
    *out_entry = cond_id; *out_exit = exit_id;
    return;
  }
  if (strcmp(t,"do_statement")==0) {
    TSNode body = ts_node_child(stmt, 1);
    TSNode kind = ts_node_child(stmt, 2);
    TSNode cond = ts_node_child(stmt, 3);
    char *cond_ir = format_expr_ir(b->source, cond, 0);

    int body_entry=-1, body_exit=-1;
    process_statement(b, body, "while.body", &body_entry, &body_exit);
    if (body_entry < 0) {
      body_entry = cfg_add_node(b->cfg, "while.body");
      cfg_node_add_line(b->cfg, body_entry, "empty");
      body_exit = body_entry;
    }

    int cond_id = cfg_add_node(b->cfg, "while.cond");
    cfg_node_add_line_owned(b->cfg, cond_id, dup_printf("RepeatCond(%s)\n  expr: %s", ts_node_type(kind), cond_ir));
    int exit_id = cfg_add_node(b->cfg, "after_while");
    cfg_node_add_line(b->cfg, exit_id, "Nop(exit)");
    push_loop(b, cond_id, exit_id);

    if (body_exit >= 0) cfg_add_edge(b->cfg, body_exit, cond_id, NULL);
    int true_is_loop = strcmp(ts_node_type(kind), "while") == 0;
    cfg_add_edge(b->cfg, cond_id, body_entry, true_is_loop ? "true" : "false");
    cfg_add_edge(b->cfg, cond_id, exit_id, true_is_loop ? "false" : "true");
    pop_loop(b);
    free(cond_ir);
    *out_entry = body_entry; *out_exit = exit_id;
    return;
  }
  if (strcmp(t,"continue_statement")==0 || strcmp(t,"continue")==0) {
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    cfg_node_add_line(b->cfg, node, "Nop(continue)");
    int cond = current_loop_cond(b);
    if (cond != -1) cfg_add_edge(b->cfg, node, cond, NULL);
    *out_entry = node; *out_exit = -1; return;
  }
  if (strcmp(t,"break_statement")==0) {
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    cfg_node_add_line(b->cfg, node, "Nop(break)");
    int exit = current_loop_exit(b);
    if (exit != -1) cfg_add_edge(b->cfg, node, exit, NULL);
    *out_entry = node; *out_exit = -1; return;
  }
  if (strcmp(t,"return_statement")==0 || strcmp(t,"return")==0) {
    int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
    cfg_node_add_line_owned(b->cfg, node, format_return_line(b->source, stmt));
    if (b->func_exit != -1) cfg_add_edge(b->cfg, node, b->func_exit, NULL);
    *out_entry = node; *out_exit = -1; return;
  }
  int node = cfg_add_node(b->cfg, role_hint ? role_hint : "block");
  cfg_node_add_line_owned(b->cfg, node, dup_printf("%s", t));
  *out_entry = *out_exit = node;
}

static void finalize_cfg_labels(CFG *cfg) {
  if (!cfg) return;
  for (int i=0;i<cfg->n_nodes;i++) {
    CFGNode *n = &cfg->nodes[i];
    free(n->label);
    const char *role = n->role ? n->role : "block";
    n->label = dup_printf("B%d (%s)", n->id, role);
  }
}

int build_cfg_for_function(const char *source, TSNode func_node, CFG **out_cfg, char *out_fname, size_t fname_len, const char *file_prefix) {
  (void)file_prefix;
  uint32_t cc = ts_node_child_count(func_node);
  char fname[256]; fname[0] = '\0';
  for (uint32_t i=0;i<cc;i++) {
    TSNode c = ts_node_child(func_node, i);
    if (strcmp(ts_node_type(c),"funcSignature") == 0) {
      uint32_t gc = ts_node_child_count(c);
      for (uint32_t j=0;j<gc;j++) {
        TSNode g = ts_node_child(c, j);
        if (strcmp(ts_node_type(g),"identifier")==0) {
          uint32_t s = ts_node_start_byte(g);
          uint32_t e = ts_node_end_byte(g);
          size_t len = (e>s)?(size_t)(e-s):0;
          if (len >= sizeof(fname)) len = sizeof(fname)-1;
          memcpy(fname, source + s, len);
          fname[len] = '\0';
          break;
        }
      }
    }
  }
  if (fname[0] == '\0') strcpy(fname, "<anon>");
  if (out_fname && fname_len>0) snprintf(out_fname, fname_len, "%s", fname);

  Builder b = {0};
  b.source = source;
  b.cfg = cfg_new();
  b.loop_depth = 0;
  b.func_exit = cfg_add_node(b.cfg, "exit");
  cfg_node_add_line(b.cfg, b.func_exit, "Nop(exit)");

  TSNode body = {0};
  for (uint32_t i=0;i<cc;i++) {
    TSNode c = ts_node_child(func_node, i);
    if (strcmp(ts_node_type(c),"body")==0) { body = c; break; }
  }
  if (ts_node_is_null(body)) {
    int entry = cfg_add_node(b.cfg, "entry");
    cfg_node_add_line(b.cfg, entry, "empty");
    cfg_add_edge(b.cfg, entry, b.func_exit, NULL);
    finalize_cfg_labels(b.cfg);
    *out_cfg = b.cfg;
    return 0;
  }
  TSNode block = {0};
  uint32_t bc = ts_node_child_count(body);
  for (uint32_t i=0;i<bc;i++) {
    TSNode c = ts_node_child(body, i);
    if (strcmp(ts_node_type(c),"block")==0) { block = c; break; }
  }
  if (ts_node_is_null(block)) {
    int entry = cfg_add_node(b.cfg, "entry");
    cfg_node_add_line(b.cfg, entry, "empty");
    cfg_add_edge(b.cfg, entry, b.func_exit, NULL);
    finalize_cfg_labels(b.cfg);
    *out_cfg = b.cfg;
    return 0;
  }
  int entry=-1, exit=-1;
  process_block(&b, block, "entry", &entry, &exit);
  if (entry < 0) {
    entry = cfg_add_node(b.cfg, "entry");
    cfg_node_add_line(b.cfg, entry, "empty");
    exit = entry;
  }
  if (exit >= 0) cfg_add_edge(b.cfg, exit, b.func_exit, NULL);
  else cfg_add_edge(b.cfg, entry, b.func_exit, NULL);
  finalize_cfg_labels(b.cfg);
  *out_cfg = b.cfg;
  return 0;
}
