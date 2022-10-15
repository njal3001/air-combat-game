#include "platform.h"
#include <unistd.h>
#include <string.h>

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
