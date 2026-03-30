#include "utils.h"

int read_integer() {
  int value;
  scanf("%d", &value);
  return value;
}

void scan_quote_string(char *str);
void binario_na_tela(char *nome_arquivo_binario);
void fill_with_trash(char *str, int size);