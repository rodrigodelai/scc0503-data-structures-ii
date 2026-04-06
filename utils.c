#include "utils.h"

int read_integer() {
  int value;
  scanf("%d", &value);
  return value;
}

char* read_string() {
  char *str = malloc(100 * sizeof(char));
  scanf("%s", str);
  return str;
}

// Reads an optional int field from CSV (empty field → -1), consuming the delimiter
int read_optional_int_csv(FILE *csv, char delimiter) {
  int c = fgetc(csv);
  if (c == delimiter || c == '\n' || c == '\r' || c == EOF) {
    if (c == delimiter) ungetc(c, csv);
    return -1;
  }
  ungetc(c, csv);
  int val = -1;
  fscanf(csv, "%d", &val);
  return val;
}

void scan_quote_string(char *str) {
  char R;

  while ((R = getchar()) != EOF && isspace(R))
      ; // ignorar espaços, \r, \n...

  if (R == 'N' || R == 'n') { // campo NULO
      getchar();
      getchar();
      getchar();       // ignorar o "ULO" de NULO.
      strcpy(str, ""); // copia string vazia
  } else if (R == '\"') {
      if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
          strcpy(str, "");
      }
      getchar();         // ignorar aspas fechando
  } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                          // aspas! Fazer leitura normal %s então, pois deve
                          // ser algum inteiro ou algo assim...
      str[0] = R;
      scanf("%s", &str[1]);
  } else { // EOF
      strcpy(str, "");
  }
}

void fill_with_trash(char *str, int size) {
  memset(str, '$', size);
}

char* read_line_csv(FILE *csv) {
  int capacity = 32;
  int len = 0;
  int character;

  char *buffer = calloc(capacity, sizeof(char));

  if (!buffer) {
    printf("Error: failed to allocate memory in read_line_csv\n");
    return NULL;
  }

  while ((character = fgetc(csv)) != EOF && character != '\n') {
    if (character == '\r') continue;

    if (len + 1 >= capacity) {
      capacity *= 2;

      char *bigger_buffer = realloc(buffer, capacity);
      
      if (!bigger_buffer) {
        printf("Error: failed to reallocate memory in read_line_csv\n");
        printf("Buffer capacity: %d\n", capacity);
        free(buffer);
        return NULL;
      }

      buffer = bigger_buffer;
    }

    buffer[len++] = (char) character;
  }

  if (len == 0 && character == EOF) {
    free(buffer);
    return NULL;
  }

  buffer[len] = '\0';
  return buffer;
}

void binario_na_tela(const char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}
