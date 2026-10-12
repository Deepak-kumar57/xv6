
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

void find(char *path, char *target)
{
    char buf[512];
    char *p;
    char *name;
    int fd;
    struct stat st;
    struct dirent de;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        close(fd);
        return;
    }

    name = path;
    for (p = path; *p != '\0'; p++) {
        if (*p == '/')
            name = p + 1;
    }

    if (strcmp(name, target) == 0)
        printf("%s\n", path);

    if (st.type == T_DIR) {
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            close(fd);
            return;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);

        if (p > buf && *(p - 1) != '/')
            *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;

            if (strcmp(de.name, ".") == 0 ||
                strcmp(de.name, "..") == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = '\0';

            find(buf, target);
        }
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "Usage: find path name\n");
        exit(1);
    }

    find(argv[1], argv[2]);

    exit(0);
}
