#include "kernel/types.h"
#include "user/user.h"

// Parse a hexadecimal string (with or without the 0x prefix).
// Same approach as the appendix of the lab manual.
static uint64
parse_hex(char *s)
{
  uint64 value = 0;

  if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    s += 2;

  while(*s){
    char c = *s;
    uint64 digit;

    if(c >= '0' && c <= '9')
      digit = c - '0';
    else if(c >= 'a' && c <= 'f')
      digit = c - 'a' + 10;
    else if(c >= 'A' && c <= 'F')
      digit = c - 'A' + 10;
    else
      break;

    value = value * 16 + digit;
    s++;
  }

  return value;
}

// Print a 64-bit value in hex, no leading zeros.
// user/printf.c's %lx is NOT usable here: its printint() keeps the
// value in a 32-bit `uint`, so 0x3fffffd000 would print as 0xFFFFD000.
static void
puthex(uint64 v)
{
  char buf[17];
  int i = 16;

  buf[16] = 0;
  do{
    buf[--i] = "0123456789abcdef"[v & 0xf];
    v >>= 4;
  }while(v != 0);

  printf("%s", &buf[i]);
}

int
main(int argc, char *argv[])
{
  uint64 va;

  if(argc != 2){
    fprintf(2, "usage: uvmlookup <virtual_address>\n");
    exit(1);
  }

  va = parse_hex(argv[1]);

  printf("=== OUC uvmlookup ===\n");
  printf("query VA : 0x");
  puthex(va);
  printf("\n");

  // The kernel does walk() + PTE2PA() and prints the rest.
  vmlookup(va);

  exit(0);
}
