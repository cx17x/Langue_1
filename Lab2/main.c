#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include <ctype.h>
#include <tree_sitter/api.h>
#include "flow.h"

static void dot_escape(FILE *out, const char *s) {
  for (; *s; ++s) {
    unsigned char ch = (unsigned char)*s;
    if (ch == '"' || ch == '\\') fprintf(out, "\\%c", ch);
    else if (ch == '\n') fputs("\\n", out);
    else if (ch == '\r') fputs("\\r", out);
    else fputc(ch, out);
  }
}

// объявление функции языка из Lab1 grammar
const TSLanguage *tree_sitter_v2lang_test(void);

static char *read_file(const char *path, size_t *out_len) {
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
  long n = ftell(f);
  if (n < 0) { fclose(f); return NULL; }
  if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
  char *buf = malloc((size_t)n + 1);
  if (!buf) { fclose(f); return NULL; }
  size_t r = fread(buf,1,(size_t)n,f);
  fclose(f);
  buf[r] = '\0';
  if (out_len) *out_len = r;
  return buf;
}



static void print_node_text(const char *source, TSNode node, char *out, size_t out_len) {
  uint32_t s = ts_node_start_byte(node);
  uint32_t e = ts_node_end_byte(node);
  size_t len = (e > s) ? (e - s) : 0;
  if (len >= out_len) len = out_len - 1;
  if (len > 0) memcpy(out, source + s, len);
  out[len] = '\0';
}

// Найти идентификатор имени функции внутри funcSignature
static void get_func_name(const char *source, TSNode func_node, char *out, size_t out_len) {
  uint32_t child_count = ts_node_child_count(func_node);
  for (uint32_t i=0;i<child_count;i++){
    TSNode c = ts_node_child(func_node, i);
    const char *type = ts_node_type(c);
    if (strcmp(type, "funcSignature") == 0) {
      // найти ребёнка identifier
      uint32_t cc = ts_node_child_count(c);
      for (uint32_t j=0;j<cc;j++){
        TSNode g = ts_node_child(c,j);
        if (strcmp(ts_node_type(g), "identifier") == 0) {
          print_node_text(source, g, out, out_len);
          return;
        }
      }
    }
  }
  // fallback: empty
  out[0] = '\0';
}

static void get_func_signature_text(const char *source, TSNode func_node, char *out, size_t out_len) {
  out[0] = '\0';
  uint32_t child_count = ts_node_child_count(func_node);
  for (uint32_t i=0;i<child_count;i++){
    TSNode c = ts_node_child(func_node, i);
    if (strcmp(ts_node_type(c), "funcSignature") == 0) {
      print_node_text(source, c, out, out_len);
      return;
    }
  }
}

typedef struct Pair {
  char *caller;
  char *callee;
  int count;
} Pair;

static int name_in_list(char **list, int count, const char *name) {
  for (int i=0;i<count;i++) if (strcmp(list[i], name) == 0) return 1;
  return 0;
}

static void scan_line_for_calls(const char *line, void (*cb)(const char*, void*), void *userdata) {
  if (!line || !cb) return;
  const char *p = line;
  while ((p = strstr(p, "Call(")) != NULL) {
    p += 5;
    while (*p && isspace((unsigned char)*p)) p++;
    const char *start = p;
    while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
    size_t len = (size_t)(p - start);
    if (len > 0) {
      char buf[128];
      if (len >= sizeof(buf)) len = sizeof(buf) - 1;
      memcpy(buf, start, len);
      buf[len] = '\0';
      cb(buf, userdata);
    }
  }
}

static int find_call_edge(Pair *pairs, int pair_n, const char *caller, const char *callee) {
  for (int i=0;i<pair_n;i++) {
    if (strcmp(pairs[i].caller, caller)==0 && strcmp(pairs[i].callee, callee)==0) return i;
  }
  return -1;
}

static void add_call_edge(Pair **pairs, int *pair_n, int *pair_cap, const char *caller, const char *callee) {
  int idx = find_call_edge(*pairs, *pair_n, caller, callee);
  if (idx >= 0) {
    (*pairs)[idx].count++;
    return;
  }
  if (*pair_n + 1 > *pair_cap) {
    *pair_cap = (*pair_cap==0)?16:(*pair_cap*2);
    *pairs = realloc(*pairs, sizeof(Pair) * (*pair_cap));
  }
  (*pairs)[*pair_n].caller = strdup(caller);
  (*pairs)[*pair_n].callee = strdup(callee);
  (*pairs)[*pair_n].count = 1;
  (*pair_n)++;
}

typedef struct CallCollectorCtx {
  Pair **pairs;
  int *pair_n;
  int *pair_cap;
  const char *caller;
  char **known_names;
  int known_count;
} CallCollectorCtx;

static void collect_call_if_known(const char *callee, void *userdata) {
  CallCollectorCtx *ctx = userdata;
  if (!name_in_list(ctx->known_names, ctx->known_count, callee)) return;
  add_call_edge(ctx->pairs, ctx->pair_n, ctx->pair_cap, ctx->caller, callee);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <input1.v2> [input2.v2 ...] [--outdir DIR]\n", argv[0]);
    return 1;
  }

  const char *outdir = NULL;
  // collect files
  int first_files = 1;
  char **files = malloc(sizeof(char*) * (argc+1));
  int file_count = 0;
  for (int i=1;i<argc;i++){
    if (strcmp(argv[i], "--outdir") == 0 && i+1<argc) { outdir = argv[i+1]; i++; continue; }
    files[file_count++] = argv[i];
  }
  if (!outdir) outdir = ".";

  // ensure outdir exists
  struct stat st = {0};
  if (stat(outdir, &st) == -1) mkdir(outdir, 0755);

  // init parser
  TSParser *parser = ts_parser_new();
  if (!ts_parser_set_language(parser, tree_sitter_v2lang_test())) {
    fprintf(stderr, "Failed to set language\n");
    return 2;
  }

  for (int i=0;i<file_count;i++) {
    const char *path = files[i];
    size_t len = 0; char *source = read_file(path, &len);
    if (!source) { fprintf(stderr, "Cannot read %s\n", path); continue; }

    TSTree *tree = ts_parser_parse_string(parser, NULL, source, (uint32_t)len);
    if (!tree) { fprintf(stderr, "Parse failed for %s\n", path); free(source); continue; }

    TSNode root = ts_tree_root_node(tree);
    // traverse to find funcDef nodes and collect them
    TSNode *stack = malloc(sizeof(TSNode) * 16384);
    int sp = 0;
    stack[sp++] = root;

    // dynamic arrays for functions
    typedef struct FuncRecord { ProgramFunction meta; TSNode node; } FuncRecord;
    FuncRecord *funcs = NULL; int func_cap = 0; int func_n = 0;

    while (sp>0) {
      TSNode node = stack[--sp];
      const char *type = ts_node_type(node);
      if (strcmp(type, "funcDef") == 0) {
        if (func_n + 1 > func_cap) { func_cap = (func_cap==0)?8:func_cap*2; funcs = realloc(funcs, sizeof(FuncRecord)*func_cap); }
        FuncRecord *fi = &funcs[func_n++];
        char name_buf[256];
        get_func_name(source, node, name_buf, sizeof(name_buf));
        if (name_buf[0]=='\0') snprintf(name_buf, sizeof(name_buf), "<anon>");
        char sig_buf[512];
        get_func_signature_text(source, node, sig_buf, sizeof(sig_buf));
        fi->meta.name = strdup(name_buf);
        fi->meta.signature = (sig_buf[0] != '\0') ? strdup(sig_buf) : strdup(name_buf);
        fi->meta.source_file = strdup(path);
        fi->meta.cfg = NULL;
        fi->node = node;
      }
      uint32_t cc = ts_node_child_count(node);
      for (uint32_t k=0;k<cc;k++) {
        stack[sp++] = ts_node_child(node,k);
        if (sp >= 16384) { fprintf(stderr, "Node stack overflow\n"); break; }
      }
    }

    // build per-function CFGs and collect call relations
    // simple arrays for call edges: caller index -> list of callee names
    char **all_func_names = NULL; int all_fn_cap=0; int all_fn_n=0;

    // prepare per-file prefix (used to produce stable expr IDs and node prefixes)
    char *pathdup = strdup(path);
    char *base = basename(pathdup);
    char sbase[256]; size_t si = 0;
    for (size_t ii=0; ii<strlen(base) && si+1<sizeof(sbase); ii++) {
      char ch = base[ii];
      if ((ch >= 'a' && ch <= 'z') || (ch >='A' && ch<='Z') || (ch>='0' && ch<='9')) sbase[si++] = ch;
      else sbase[si++] = '_';
    }
    sbase[si] = '\0';
    char prefix[320]; snprintf(prefix, sizeof(prefix), "file_%s", sbase);

    for (int fi=0; fi<func_n; fi++) {
      // unique function name list
      char *name = funcs[fi].meta.name;
      int found = 0;
      for (int k=0;k<all_fn_n;k++) if (strcmp(all_func_names[k], name)==0) { found=1; break; }
      if (!found) {
        if (all_fn_n+1>all_fn_cap) { all_fn_cap = (all_fn_cap==0)?16:all_fn_cap*2; all_func_names = realloc(all_func_names, sizeof(char*)*all_fn_cap); }
        all_func_names[all_fn_n++] = strdup(name);
      }
      // build CFG
      CFG *cfg = NULL; char out_fname[256]; out_fname[0]='\0';
      char fnprefix[360]; snprintf(fnprefix, sizeof(fnprefix), "%s_f%d", prefix, fi);
      build_cfg_for_function(source, funcs[fi].node, &cfg, out_fname, sizeof(out_fname), fnprefix);
      funcs[fi].meta.cfg = cfg;
    }



    // prepare per-file DOT
    char outfile[1024]; snprintf(outfile, sizeof(outfile), "%s/%s.dot", outdir, base);
    FILE *of = fopen(outfile, "w");
    if (!of) { fprintf(stderr, "Cannot write %s\n", outfile); }
    else {
      fprintf(of, "digraph G {\n");
      // for each function, print subgraph with prefixed node names
      for (int fi=0; fi<func_n; fi++) {
        ProgramFunction *pf = &funcs[fi].meta;
        CFG *cfg = pf->cfg;
        if (!cfg) continue;
        fprintf(of, "  subgraph cluster_f%d {\n", fi);
        fprintf(of, "    label=\"function %s\";\n", pf->name);
        // print nodes with prefix f<fi>_n<id>
        for (int n=0;n<cfg->n_nodes;n++) {
          fprintf(of, "    %s_f%d_n%d [shape=box,label=\"", prefix, fi, cfg->nodes[n].id);
          dot_escape(of, cfg->nodes[n].label);
          for (int ln=0; ln<cfg->nodes[n].ops.n_lines; ln++) {
            fputs("\\n", of);
            dot_escape(of, cfg->nodes[n].ops.lines[ln]);
          }
          fputs("\"];\n", of);
        }
        // print edges
        for (int n=0;n<cfg->n_nodes;n++) {
          for (int j=0;j<cfg->nodes[n].succ.n;j++) {
            int to = cfg->nodes[n].succ.a[j];
            char *lab = NULL;
            if (cfg->nodes[n].succ_labels) lab = cfg->nodes[n].succ_labels[j];
            if (lab) fprintf(of, "    %s_f%d_n%d -> %s_f%d_n%d [label=\"%s\"];\n", prefix, fi, n, prefix, fi, to, lab);
            else fprintf(of, "    %s_f%d_n%d -> %s_f%d_n%d;\n", prefix, fi, n, prefix, fi, to);
          }
        }
        fprintf(of, "  }\n");
      }
      fprintf(of, "}\n");
      fclose(of);
      printf("Wrote %s\n", outfile);
    }

    // build call-graph based on Call(...) occurrences inside CFG nodes
    Pair *pairs = NULL; int pair_cap=0, pair_n=0;
    for (int fi=0; fi<func_n; fi++) {
      ProgramFunction *pf = &funcs[fi].meta;
      CFG *cfg = pf->cfg;
      if (!cfg) continue;
      CallCollectorCtx ctx = { &pairs, &pair_n, &pair_cap, pf->name, all_func_names, all_fn_n };
      for (int n=0;n<cfg->n_nodes;n++) {
        for (int ln=0; ln<cfg->nodes[n].ops.n_lines; ln++) {
          scan_line_for_calls(cfg->nodes[n].ops.lines[ln], collect_call_if_known, &ctx);
        }
      }
    }

    // write callgraph
    char callgraph_dot[1024];
    char callgraph_csv[1024];
    snprintf(callgraph_dot, sizeof(callgraph_dot), "%s/%s.callgraph.dot", outdir, base);
    snprintf(callgraph_csv, sizeof(callgraph_csv), "%s/%s.callgraph.csv", outdir, base);
    FILE *cf = fopen(callgraph_dot, "w");
    if (cf) {
      fprintf(cf, "digraph CallGraph {\n");
      // unique nodes
      for (int k=0;k<all_fn_n;k++) fprintf(cf, "  \"%s\";\n", all_func_names[k]);
      for (int p=0;p<pair_n;p++) fprintf(cf, "  \"%s\" -> \"%s\" [label=\"%d\"];\n", pairs[p].caller, pairs[p].callee, pairs[p].count);
      fprintf(cf, "}\n"); fclose(cf); printf("Wrote %s\n", callgraph_dot);

      // write CSV
      FILE *csv = fopen(callgraph_csv, "w");
      if (csv) {
        fprintf(csv, "caller,callee,count\n");
        for (int p=0;p<pair_n;p++) fprintf(csv, "%s,%s,%d\n", pairs[p].caller, pairs[p].callee, pairs[p].count);
        fclose(csv);
        printf("Wrote %s\n", callgraph_csv);
      } else fprintf(stderr, "Cannot write %s\n", callgraph_csv);
    } else fprintf(stderr, "Cannot write callgraph %s\n", callgraph_dot);

    // free pairs
    for (int p=0;p<pair_n;p++) { free(pairs[p].caller); free(pairs[p].callee); }
    free(pairs);
    for (int k=0;k<all_fn_n;k++) free(all_func_names[k]); free(all_func_names);

    // cleanup
    for (int fi=0; fi<func_n; fi++) {
      if (funcs[fi].meta.cfg) cfg_free(funcs[fi].meta.cfg);
      free(funcs[fi].meta.name);
      free(funcs[fi].meta.signature);
      free(funcs[fi].meta.source_file);
    }
    free(funcs);
    free(stack);
    ts_tree_delete(tree);
    free(source);
    free(pathdup);
  }

  ts_parser_delete(parser);
  free(files);
  return 0;
}
