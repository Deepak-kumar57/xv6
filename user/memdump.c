#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  for(int i=0 ; fmt[i] != '\0' ; i++)
  {
    if(fmt[i] == 'i')
    {
      if(len < 4)
      {
        printf("memdump: not enough data for '%c'\n", fmt[i]);
        return;
      }

      printf("%d\n",*(int *)data);

      data = data + 4;
      len = len - 4;
    }

    else if(fmt[i] == 'p')
    {
      if(len < 8)
      {
        printf("memdump: not enough data for '%c'\n", fmt[i]);
        return;
      }

      printf("%lx\n",*(uint64 *)data);

      data = data + 8;
      len = len - 8;
    }

    else if(fmt[i] == 'h')
    {
      if(len < 2)
      {
        printf("memdump: not enough data for '%c'\n", fmt[i]);
        return;
      }

      printf("%d\n",*(short *)data);

      data = data + 2;
      len = len - 2;
    }

    else if(fmt[i] == 'c')
    {
      if(len < 1)
      {
        printf("memdump: not enough data for '%c'\n", fmt[i]);
        return;
      }

      printf("%c\n",*data);

      data = data + 1;
      len = len - 1;
    }

    else if(fmt[i] == 's')
    {
      if(len < 8)
      {
        printf("memdump: not enough data for '%c'\n", fmt[i]);
        return;
      }

      printf("%s\n",*(char **)data);

      data = data + 8;
      len = len - 8;
    }

    else if(fmt[i] == 'S')
    {
        for(int j = 0; j < len && data[j] != '\0'; j++)
        {
            printf("%c", data[j]);
        }

        printf("\n");
        data = data + len;
        len = 0;
    }

    else 
    {
      return;
    }
  }
}
