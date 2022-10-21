#pragma once
#include <stdlib.h>

void get_exec_path(char *buf, size_t buf_size);

// NOTE: modifies input buffer
void get_dir_path(char *path_buf, size_t up);

char *read_file(const char *path);
