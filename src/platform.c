#include "platform.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// NOTE: Linux only
void get_exec_path(char *buf, size_t buf_size)
{
    readlink("/proc/self/exe", buf, buf_size - 1);
    buf[buf_size - 1] = '\0';
}

void get_dir_path(char *path_buf, size_t up)
{
    size_t dir_count = 0;
    for (int n = strlen(path_buf); n >= 0; n--)
    {
        if (path_buf[n] == '/')
        {
            if (dir_count == up)
            {
                path_buf[n + 1] = '\0';
                return;
            }

            dir_count++;
        }
    }
}

char *read_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *data = malloc(size + 1);
    fread(data, size, 1, f);
    data[size] = '\0';

    return data;
}
