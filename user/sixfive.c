
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
    char separators[] = " -\r\t\n./,";
    char buf[1];
    char num[32];
    int fd;
    int n;
    int i;
    int len;
    int value;
    int overflow;

    for (int arg = 1; arg < argc; arg++) {
        fd = open(argv[arg], O_RDONLY);

        if (fd < 0) {
            fprintf(2, "sixfive: cannot open %s\n", argv[arg]);
            continue;
        }

        len = 0;
        value = 0;
        overflow = 0;

        while ((n = read(fd, buf, 1)) > 0) {
            char c = buf[0];
            int separator = 0;

            for (i = 0; separators[i] != '\0'; i++) {
                if (c == separators[i]) {
                    separator = 1;
                    break;
                }
            }

            if (c >= '0' && c <= '9') {
                if (len < sizeof(num) - 1) {
                    num[len++] = c;
                } else {
                    overflow = 1;
                }

                value = value * 10 + (c - '0');
            } else if (separator) {
                if (len > 0) {
                    if (!overflow && (value % 5 == 0 || value % 6 == 0)) {
                        num[len] = '\0';
                        printf("%s\n", num);
                    }

                    len = 0;
                    value = 0;
                    overflow = 0;
                }
            } else {
                len = 0;
                value = 0;
                overflow = 0;
            }
        }

        if (len > 0 && !overflow && (value % 5 == 0 || value % 6 == 0)) {
            num[len] = '\0';
            printf("%s\n", num);
        }

        close(fd);
    }

    if (argc == 1) {
        len = 0;
        value = 0;
        overflow = 0;

        while ((n = read(0, buf, 1)) > 0) {
            char c = buf[0];
            int separator = 0;

            for (i = 0; separators[i] != '\0'; i++) {
                if (c == separators[i]) {
                    separator = 1;
                    break;
                }
            }

            if (c >= '0' && c <= '9') {
                if (len < sizeof(num) - 1) {
                    num[len++] = c;
                } else {
                    overflow = 1;
                }

                value = value * 10 + (c - '0');
            } else if (separator) {
                if (len > 0) {
                    if (!overflow && (value % 5 == 0 || value % 6 == 0)) {
                        num[len] = '\0';
                        printf("%s\n", num);
                    }

                    len = 0;
                    value = 0;
                    overflow = 0;
                }
            } else {
                len = 0;
                value = 0;
                overflow = 0;
            }
        }

        if (len > 0 && !overflow && (value % 5 == 0 || value % 6 == 0)) {
            num[len] = '\0';
            printf("%s\n", num);
        }
    }

    exit(0);
}
