#include "linear_code.h"
#include "flow.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

static void intlist_push(IntList *l, int v) {
  if (l->n + 1 > l->cap) {
    l->cap = (l->cap == 0) ? 8 : l->cap * 2;
    l->a = realloc(l->a, sizeof(int) * l->cap);
  }
  l->a[l->n++] = v;
}

static void textlist_add_owned(TextList *t, char *line) {
  if (!line) return;
  if (t->n_lines + 1 > t->cap_lines) {
    t->cap_lines = (t->cap_lines == 0) ? 8 : t->cap_lines * 2;
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
  t->lines = NULL;
  t->n_lines = 0;
  t->cap_lines = 0;
}

typedef struct VarReg {
  char *name;
  const char *reg;
} VarReg;

typedef struct EmitContext {
  VarReg *entries;
  int n;
  int cap;
  int next;
} EmitContext;

static const char *scratch_reg = "r5";
static const char *var_register_pool[] = { "r0", "r1", "r2", "r3" };
static const int var_register_pool_size = sizeof(var_register_pool) / sizeof(var_register_pool[0]);

static void emit_context_init(EmitContext *ctx) {
  if (!ctx) return;
  ctx->entries = NULL;
  ctx->n = 0;
  ctx->cap = 0;
  ctx->next = 0;
}

static void emit_context_free(EmitContext *ctx) {
  if (!ctx) return;
  for (int i=0;i<ctx->n;i++) free(ctx->entries[i].name);
  free(ctx->entries);
  ctx->entries = NULL;
  ctx->n = ctx->cap = ctx->next = 0;
}

static const char *lookup_var_reg(const EmitContext *ctx, const char *name) {
  if (!ctx || !name) return NULL;
  for (int i=0;i<ctx->n;i++) {
    if (ctx->entries[i].name && strcmp(ctx->entries[i].name, name) == 0) return ctx->entries[i].reg;
  }
  return NULL;
}

static const char *allocate_var_reg(EmitContext *ctx) {
  if (!ctx) return scratch_reg;
  int idx = ctx->next++;
  if (idx >= var_register_pool_size) idx = idx % var_register_pool_size;
  return var_register_pool[idx];
}

static const char *ensure_var_reg(EmitContext *ctx, const char *name) {
  if (!name) return scratch_reg;
  const char *existing = lookup_var_reg(ctx, name);
  if (existing) return existing;
  if (!ctx) return scratch_reg;
  if (ctx->n + 1 > ctx->cap) {
    ctx->cap = ctx->cap == 0 ? 4 : ctx->cap * 2;
    ctx->entries = realloc(ctx->entries, sizeof(VarReg) * ctx->cap);
  }
  const char *reg = allocate_var_reg(ctx);
  ctx->entries[ctx->n].name = strdup(name);
  ctx->entries[ctx->n].reg = reg;
  ctx->n++;
  return reg;
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

ProgramImage *program_image_new(void) {
  ProgramImage *img = malloc(sizeof(ProgramImage));
  img->blocks = NULL;
  img->n_blocks = 0;
  img->cap_blocks = 0;
  img->data_items = NULL;
  img->n_data = 0;
  img->cap_data = 0;
  return img;
}

static void free_block(CodeBlock *b) {
  if (!b) return;
  free(b->name);
  textlist_clear(&b->lines);
}

static void free_data_item(DataItem *d) {
  if (!d) return;
  free(d->name);
  free(d->value);
}

void program_image_free(ProgramImage *img) {
  if (!img) return;
  for (int i=0;i<img->n_blocks;i++) free_block(&img->blocks[i]);
  for (int i=0;i<img->n_data;i++) free_data_item(&img->data_items[i]);
  free(img->blocks);
  free(img->data_items);
  free(img);
}

static CodeBlock *program_image_add_block(ProgramImage *img, const char *name) {
  if (img->n_blocks + 1 > img->cap_blocks) {
    img->cap_blocks = (img->cap_blocks == 0) ? 4 : img->cap_blocks * 2;
    img->blocks = realloc(img->blocks, sizeof(CodeBlock) * img->cap_blocks);
  }
  CodeBlock *b = &img->blocks[img->n_blocks++];
  b->name = strdup(name ? name : "block");
  b->lines.lines = NULL;
  b->lines.n_lines = 0;
  b->lines.cap_lines = 0;
  return b;
}

void program_image_add_data(ProgramImage *img, const char *name, const char *value) {
  if (!img) return;
  if (img->n_data + 1 > img->cap_data) {
    img->cap_data = (img->cap_data == 0) ? 4 : img->cap_data * 2;
    img->data_items = realloc(img->data_items, sizeof(DataItem) * img->cap_data);
  }
  DataItem *d = &img->data_items[img->n_data++];
  d->name = strdup(name ? name : "data");
  d->value = strdup(value ? value : "0");
}

static char *sanitize_label(const char *s) {
  if (!s) return strdup("label");
  size_t len = strlen(s);
  char *buf = malloc(len + 2);
  size_t out = 0;
  for (size_t i=0;i<len;i++) {
    unsigned char ch = (unsigned char)s[i];
    if (isalnum(ch) || ch == '_') buf[out++] = (char)ch;
    else buf[out++] = '_';
  }
  if (out == 0) buf[out++] = 'f';
  buf[out] = '\0';
  return buf;
}

static char *build_function_label(const ProgramFunction *func) {
  if (!func) return strdup("func");
  const char *src = func->source_file ? func->source_file : "";
  const char *base = strrchr(src, '/');
  base = base ? base + 1 : src;
  char *base_clean = sanitize_label(base);
  char *name_clean = sanitize_label(func->name ? func->name : "func");
  char *label = dup_printf("%s_%s", base_clean, name_clean);
  free(base_clean);
  free(name_clean);
  return label;
}

static int find_entry_node(const CFG *cfg) {
  if (!cfg) return 0;
  for (int i=0;i<cfg->n_nodes;i++) {
    if (cfg->nodes[i].role && strcmp(cfg->nodes[i].role, "entry") == 0) return i;
  }
  return 0;
}

static void dfs_order(const CFG *cfg, int node, int *visited, IntList *order) {
  if (!cfg || node < 0 || node >= cfg->n_nodes) return;
  if (visited[node]) return;
  visited[node] = 1;
  intlist_push(order, node);
  const CFGNode *n = &cfg->nodes[node];
  for (int i=0;i<n->succ.n;i++) dfs_order(cfg, n->succ.a[i], visited, order);
}

static void emit_comment(CodeBlock *b, const char *line) {
  if (!line || !*line) return;
  char *comment = dup_printf("; %s", line);
  textlist_add_owned(&b->lines, comment);
}

static void emit_instruction(CodeBlock *b, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (len < 0) return;
  char *buf = malloc((size_t)len + 1);
  if (!buf) return;
  va_start(ap, fmt);
  vsnprintf(buf, (size_t)len + 1, fmt, ap);
  va_end(ap);
  textlist_add_owned(&b->lines, buf);
}

static void emit_block_label(CodeBlock *b, const char *label) {
  char *line = dup_printf("%s:", label);
  textlist_add_owned(&b->lines, line);
}

static void emit_prolog(CodeBlock *b) {
  emit_instruction(b, "  PUSH BP");
  emit_instruction(b, "  MOV BP, SP");
}

static void emit_epilog(CodeBlock *b) {
  emit_instruction(b, "  MOV SP, BP");
  emit_instruction(b, "  POP BP");
  emit_instruction(b, "  RET");
}

static int literal_to_value(const char *literal, long long *value) {
  if (!literal || !value) return 0;
  size_t len = strlen(literal);
  if (len >= 2 && literal[0] == '\'' && literal[len-1] == '\'') {
    const char *body = literal + 1;
    size_t body_len = len - 2;
    if (body_len == 0) return 0;
    const char *p = body;
    if (body_len >= 2 && p[0] == '\\' && p[1] == '\\') {
      p++;
      body_len--;
    }
    unsigned char ch = 0;
    if (*p == '\\' && body_len >= 2) {
      char esc = p[1];
      switch (esc) {
        case 'n': ch = '\n'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        case '\\': ch = '\\'; break;
        case '\'': ch = '\''; break;
        case '0': ch = '\0'; break;
        default: ch = (unsigned char)esc; break;
      }
    } else {
      ch = (unsigned char)*p;
    }
    *value = (long long)ch;
    return 1;
  }
  char *end = NULL;
  long long val = strtoll(literal, &end, 0);
  if (end && end != literal) {
    *value = val;
    return 1;
  }
  return 0;
}

static const char *arg_registers[] = { "r0", "r1", "r2", "r3" };
static const int arg_register_count = sizeof(arg_registers) / sizeof(arg_registers[0]);

static void emit_literal_load(CodeBlock *b, const char *reg, const ExprInfo *expr) {
  long long value = 0;
  const char *payload = expr && expr->literal ? expr->literal : (expr ? expr->text : NULL);
  if (payload && literal_to_value(payload, &value)) {
    emit_instruction(b, "  li %s, %lld", reg, value);
  } else if (payload) {
    emit_comment(b, payload);
  } else emit_comment(b, "literal ?");
}

static const char *resolve_expr_reg(CodeBlock *b, EmitContext *ctx, const ExprInfo *expr, const char *preferred) {
  if (!expr) return NULL;
  if (expr->identifier && ctx) return ensure_var_reg(ctx, expr->identifier);
  const char *target = preferred ? preferred : scratch_reg;
  emit_literal_load(b, target, expr);
  return target;
}

static void emit_binary_assignment(CodeBlock *b, const FlowOperation *op, EmitContext *ctx) {
  if (!op || !op->lhs.identifier) return;
  const char *dest = ensure_var_reg(ctx, op->lhs.identifier);
  const char *lhs_reg = resolve_expr_reg(b, ctx, &op->bin_left, dest);
  const char *rhs_reg = resolve_expr_reg(b, ctx, &op->bin_right, scratch_reg);
  if (!lhs_reg) lhs_reg = dest;
  if (!rhs_reg) rhs_reg = scratch_reg;
  if (lhs_reg != dest) emit_instruction(b, "  mov %s, %s", dest, lhs_reg);
  const char *instr = "add";
  switch (op->bin_kind) {
    case BIN_SUB: instr = "sub"; break;
    case BIN_MUL: instr = "mul"; break;
    case BIN_DIV: instr = "div"; break;
    default: instr = "add"; break;
  }
  emit_instruction(b, "  %s %s, %s", instr, dest, rhs_reg);
  char *comment = dup_printf("store -> %s", op->lhs.identifier);
  emit_comment(b, comment);
  free(comment);
}

static void emit_flow_operation(CodeBlock *b, const FlowOperation *op, EmitContext *ctx) {
  if (!op) return;
  switch (op->kind) {
    case FLOW_OP_ASSIGN:
      if (op->bin_kind != BIN_UNKNOWN) {
        emit_binary_assignment(b, op, ctx);
        break;
      }
      if (op->lhs.identifier) {
        const char *dest = ensure_var_reg(ctx, op->lhs.identifier);
        if (op->rhs.text || op->rhs.literal) {
          emit_literal_load(b, dest, &op->rhs);
          char *comment = dup_printf("store -> %s", op->lhs.identifier);
          emit_comment(b, comment);
          free(comment);
          break;
        }
      }
      emit_comment(b, op->rhs.text ? op->rhs.text : "Assign");
      break;
    case FLOW_OP_CALL: {
      char *label = dup_printf("CALL %s", op->call_name ? op->call_name : "<unknown>");
      emit_comment(b, label);
      free(label);
      int idx = 0;
      for (int i=0;i<op->n_call_args && idx < arg_register_count; i++, idx++) {
        const char *arg_desc = op->call_args[i].text ? op->call_args[i].text : "arg";
        emit_comment(b, arg_desc);
        emit_literal_load(b, arg_registers[idx], &op->call_args[i]);
      }
      emit_instruction(b, "  call %s", op->call_name ? op->call_name : "<unknown>");
      break;
    }
    case FLOW_OP_RETURN:
      emit_comment(b, "return");
      if (op->value.text || op->value.literal) emit_literal_load(b, "r0", &op->value);
      emit_instruction(b, "  ret");
      break;
    default:
      emit_comment(b, op->value.text ? op->value.text : "op");
      break;
  }
}

static void emit_flow_ops(CodeBlock *b, const CFGNode *node, EmitContext *ctx) {
  if (!node || !node->n_flow_ops) return;
  for (int i=0;i<node->n_flow_ops;i++) {
    emit_flow_operation(b, node->flow_ops[i], ctx);
  }
}

static void emit_node_ops(CodeBlock *b, const CFGNode *node, EmitContext *ctx) {
  if (!node) return;
  if (node->n_flow_ops > 0) {
    emit_flow_ops(b, node, ctx);
    return;
  }
  for (int i=0;i<node->ops.n_lines;i++) {
    emit_comment(b, node->ops.lines[i]);
  }
}

static int is_exit_role(const CFGNode *node) {
  return node && node->role && strcmp(node->role, "exit") == 0;
}

static int find_next_in_order(const IntList *order, int node_id) {
  if (!order) return -1;
  for (int i=0;i<order->n-1;i++) {
    if (order->a[i] == node_id) return order->a[i+1];
  }
  return -1;
}

static const char *cond_false_instr(CondKind kind) {
  switch (kind) {
    case CONDK_EQ: return "jnz";
    case CONDK_NE: return "jz";
    case CONDK_LT: return "jge";
    case CONDK_GT: return "jle";
    case CONDK_LE: return "jg";
    case CONDK_GE: return "jl";
    default: return "jz";
  }
}

static void emit_branch(CodeBlock *b, const CFG *cfg, const char *const *labels, int node_id, const IntList *order, EmitContext *ctx) {
  const CFGNode *node = &cfg->nodes[node_id];
  if (node->succ.n == 0) return;
  int next = find_next_in_order(order, node_id);
  if (node->succ.n == 1) {
    int target = node->succ.a[0];
    if (target != next) emit_instruction(b, "  JMP %s", labels[target]);
    return;
  }
  int true_id = -1;
  int false_id = -1;
  for (int i=0;i<node->succ.n;i++) {
    const char *lab = node->succ_labels ? node->succ_labels[i] : NULL;
    if (lab && strcmp(lab, "true") == 0) true_id = node->succ.a[i];
    else if (lab && strcmp(lab, "false") == 0) false_id = node->succ.a[i];
  }
  if (true_id == -1 || false_id == -1) {
    true_id = node->succ.a[0];
    false_id = node->succ.a[1];
  }
  const FlowOperation *cond_op = NULL;
  for (int i=0;i<node->n_flow_ops;i++) {
    if (node->flow_ops[i]->kind == FLOW_OP_COND) {
      cond_op = node->flow_ops[i];
      break;
    }
  }
  if (cond_op && cond_op->cond_kind != CONDK_UNKNOWN) {
    const char *left_reg = resolve_expr_reg(b, ctx, &cond_op->cond_left, "r0");
    const char *right_reg = resolve_expr_reg(b, ctx, &cond_op->cond_right, "r1");
    if (!left_reg) left_reg = "r0";
    if (!right_reg) right_reg = "r1";
    emit_instruction(b, "  cmp %s, %s", left_reg, right_reg);
    const char *false_instr = cond_false_instr(cond_op->cond_kind);
    if (false_id != -1) emit_instruction(b, "  %s %s", false_instr, labels[false_id]);
    if (true_id != -1 && true_id != next) emit_instruction(b, "  JMP %s", labels[true_id]);
  } else {
    emit_instruction(b, "  CMP R0, 0");
    if (false_id != -1) emit_instruction(b, "  JZ %s", labels[false_id]);
    if (true_id != -1 && true_id != next) emit_instruction(b, "  JMP %s", labels[true_id]);
  }
}

void program_image_add_function_from_cfg(ProgramImage *img, const ProgramFunction *func) {
  if (!img || !func || !func->cfg) return;
  CFG *cfg = func->cfg;
  char *func_label = build_function_label(func);
  CodeBlock *block = program_image_add_block(img, func_label);
  EmitContext ctx;
  emit_context_init(&ctx);

  int entry = find_entry_node(cfg);
  int *visited = calloc((size_t)cfg->n_nodes, sizeof(int));
  IntList order = {0};
  dfs_order(cfg, entry, visited, &order);
  for (int i=0;i<cfg->n_nodes;i++) {
    if (!visited[i]) dfs_order(cfg, i, visited, &order);
  }

  char **labels = calloc((size_t)cfg->n_nodes, sizeof(char*));
  for (int i=0;i<cfg->n_nodes;i++) {
    if (is_exit_role(&cfg->nodes[i])) {
      labels[i] = dup_printf("%s_end", func_label);
    } else if (i == entry) {
      labels[i] = strdup(func_label);
    } else {
      labels[i] = dup_printf("%s_B%d", func_label, i);
    }
  }

  for (int i=0;i<order.n;i++) {
    int node_id = order.a[i];
    const CFGNode *node = &cfg->nodes[node_id];
    if (is_exit_role(node)) continue;
    emit_block_label(block, labels[node_id]);
    if (!is_exit_role(node)) {
      emit_node_ops(block, node, &ctx);
      emit_branch(block, cfg, (const char *const *)labels, node_id, &order, &ctx);
    }
  }

  int exit_id = -1;
  for (int i=0;i<cfg->n_nodes;i++) {
    if (is_exit_role(&cfg->nodes[i])) { exit_id = i; break; }
  }
  if (exit_id >= 0) {
    emit_block_label(block, labels[exit_id]);
  } else {
    char *fallback = dup_printf("%s_end", func_label);
    emit_block_label(block, fallback);
    free(fallback);
  }
  if (strstr(func_label, "send_byte")) emit_instruction(block, "  outb r0");
  emit_instruction(block, "  ret");
  emit_context_free(&ctx);

  for (int i=0;i<cfg->n_nodes;i++) free(labels[i]);
  free(labels);
  free(order.a);
  free(visited);
  free(func_label);
}

void program_image_write_asm(const ProgramImage *img, FILE *out) {
  if (!img || !out) return;
  fprintf(out, "[section code, code]\n");
  const char *entry_label = (img->n_blocks > 0) ? img->blocks[0].name : "calc_txt_main";
  fprintf(out, "start:\n");
  fprintf(out, "  ldsp 0xfff0\n");
  fprintf(out, "  ldbp 0xfff0\n");
  fprintf(out, "  call %s\n", entry_label);
  fprintf(out, "  hlt\n");
  if (img->n_data > 0) {
    fprintf(out, ".data\n");
    for (int i=0;i<img->n_data;i++) {
      fprintf(out, "%s: %s\n", img->data_items[i].name, img->data_items[i].value);
    }
    fprintf(out, "\n");
  }
  fprintf(out, ".text\n");
  for (int i=0;i<img->n_blocks;i++) {
    CodeBlock *b = &img->blocks[i];
    fprintf(out, "; function %s\n", b->name);
    for (int j=0;j<b->lines.n_lines;j++) {
      fprintf(out, "%s\n", b->lines.lines[j]);
    }
    fprintf(out, "\n");
  }
}
