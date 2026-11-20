#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;

  if(growproc(n) < 0)
    return -1;

  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Print the current process's page table.
uint64
sys_vmprint(void)
{
  struct proc *p = myproc();

  printf("current process: pid=%d name=%s\n",
         p->pid, p->name);

  vmprint(p->pagetable);

  return 0;
}

// Print PTE / physical page / offset / physical address for a user VA.
uint64
sys_vmlookup(void)
{
  uint64 va;
  pte_t pte;
  struct proc *p = myproc();

  argaddr(0, &va);

  printf("VA : 0x%lx\n", va);

  pte = vmlookup(p->pagetable, va);
  if(pte == 0){
    printf("VA 0x%lx: not mapped\n", va);
    return 0;
  }

  printf("PTE  : 0x%lx\n", pte);
  printf("PA page : 0x%lx\n", PTE2PA(pte));
  printf("offset : 0x%lx\n", va & 0xFFF);
  printf("PA : 0x%lx\n", PTE2PA(pte) + (va & 0xFFF));
  return 0;
}