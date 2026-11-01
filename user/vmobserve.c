#include "kernel/types.h"
#include "user/user.h"

int global_var = 123;

int
main(void)
{
  int local_var = 456;

  printf("\n===OUC===c1: address analysis ===\n");
  printf("<vmobserve.c> fix address analysis\n");
  // q(≧▽≦q) by Ling Tai
  unsigned long int stack_address = (unsigned long int)(void *)&local_var;
  printf("VPN[0] : %lu\n", (stack_address >> 12) & 0x1FF);
  printf("VPN[1] : %lu\n", (stack_address >> 21) & 0x1FF);
  printf("VPN[2] : %lu\n", (stack_address >> 30) & 0x1FF);
  printf("offset : %lu\n", stack_address & 0xFFF);

  printf("\n===OUC===c1: address analysis Done ===\n");

  // calling vmprint() to print the current process's page table
  printf("\n===OUC===c1: vmprint() ===\n");
  if(vmprint() < 0){
    printf("vmprint failed\n");
    exit(1);
  }
  printf("\n===OUC===c1: vmprint() Done ===\n");



  exit(0);
}
