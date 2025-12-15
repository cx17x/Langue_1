# Сборка дерева
```
# из корня
clang ast_dump.c Lab1/src/parser.c vendor/tree-sitter/lib/src/lib.c \
  -I vendor/tree-sitter/lib/include -I vendor/tree-sitter/lib/src \
  -o ast_dump

ls -l ast_dump
./ast_dump Lab1/test.txt

```