#pragma once
#include <stdlib.h>

// NOTE: modifies input buffer
void get_dir_path(char *path_buf, size_t up);
char *read_file(const char *path);
