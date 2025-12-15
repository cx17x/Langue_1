// ast_dump.c — вывод AST в формате DOT (Graphviz)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

// объявление функции языка (из grammar.js → name: 'v2lang_test')
const TSLanguage *tree_sitter_v2lang_test(void);

// читает весь файл в память
static char* read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); return NULL; }
    if (fseek(f, 0, SEEK_END) != 0) { perror("fseek"); fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { perror("ftell"); fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { perror("fseek"); fclose(f); return NULL; }

    char *buf = (char*)malloc((size_t)n + 1);
    if (!buf) { perror("malloc"); fclose(f); return NULL; }

    size_t rd = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

// экранируем кавычки и странные символы для DOT-лейбла
static void print_label_escaped(FILE *out, const char *s) {
    fputc('"', out);
    for (const char *p = s; *p; ++p) {
        if (*p == '"' || *p == '\\')
            fprintf(out, "\\%c", *p);
        else if (*p == '\n')
            fprintf(out, "\\n");
        else
            fputc(*p, out);
    }
    fputc('"', out);
}

// рекурсивный обход AST и генерация узлов/рёбер
static void dump_node_dot(FILE *out, TSNode node, int parent_id, int *next_id) {
    int my_id = (*next_id)++;

    const char *type = ts_node_type(node);

    // вывод вершины
    fprintf(out, "  n%d [label=", my_id);
    print_label_escaped(out, type);
    fprintf(out, "];\n");

    // если есть родитель — вывод ребра parent -> this
    if (parent_id >= 0) {
        fprintf(out, "  n%d -> n%d;\n", parent_id, my_id);
    }

    // обходим детей
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        dump_node_dot(out, child, my_id, next_id);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <input.v2> <output.dot>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    size_t len = 0;
    char *source = read_file(input_path, &len);
    if (!source) return 2;

    TSParser *parser = ts_parser_new();
    if (!ts_parser_set_language(parser, tree_sitter_v2lang_test())) {
        fprintf(stderr, "Failed to set language.\n");
        free(source);
        ts_parser_delete(parser);
        return 3;
    }

    TSTree *tree = ts_parser_parse_string(parser, NULL, source, (uint32_t)len);
    if (!tree) {
        fprintf(stderr, "Parse failed (null tree).\n");
        free(source);
        ts_parser_delete(parser);
        return 4;
    }

    FILE *out = fopen(output_path, "w");
    if (!out) {
        perror("fopen output");
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        free(source);
        return 5;
    }

    // начало DOT-графа
    fprintf(out, "digraph AST {\n");
    int next_id = 0;
    TSNode root = ts_tree_root_node(tree);
    dump_node_dot(out, root, -1, &next_id);
    fprintf(out, "}\n");

    fclose(out);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    free(source);
    return 0;
}
