#include <stdlib.h>

int urand(int low, int high) {
  double r = rand() / (1.0 + RAND_MAX);
  int range = high - low + 1;
  return (int)((r * range) + low);
}

void randstr(char *str, int len) {
  int i;
  for (i = 0; i < len-1; i++) {
    str[i] = '!' + urand(0, 'z'-'!');
  }
  str[len-1] = '\0';
}

void randnumstr(char *str, int len) {
  int i;
  for (i = 0; i < len-1; i++) {
    str[i] = '0' + urand(0, 9);
  }
  str[len-1] = '\0';
}
