#ifndef LAB2_LINEAR_CODE_H
#define LAB2_LINEAR_CODE_H

#include <stdio.h>
#include "flow.h"

typedef struct CodeBlock {
  char *name;
  TextList lines;
} CodeBlock;

typedef struct DataItem {
  char *name;
  char *value;
} DataItem;

typedef struct ProgramImage {
  CodeBlock *blocks;
  int n_blocks;
  int cap_blocks;
  DataItem *data_items;
  int n_data;
  int cap_data;
} ProgramImage;

ProgramImage *program_image_new(void);
void program_image_free(ProgramImage *img);
void program_image_add_data(ProgramImage *img, const char *name, const char *value);
void program_image_add_function_from_cfg(ProgramImage *img, const ProgramFunction *func);
void program_image_write_asm(const ProgramImage *img, FILE *out);

#endif
