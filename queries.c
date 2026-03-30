
#include "queries.h"

void create_from_csv(const char *csv_filename, const char *bin_filename);
void select_all(const char *bin_filename);
void select_where(const char *bin_filename, int num_criteria);
void select_by_rrn(const char *bin_filename, int rrn);