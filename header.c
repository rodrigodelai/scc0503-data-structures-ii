#include "header.h"

void initialize_header(Header *hdr);
void read_header_binary(FILE *bin, Header *hdr);
void write_header_binary(FILE *bin, Header *hdr);
void update_header_status(FILE *bin, char status);