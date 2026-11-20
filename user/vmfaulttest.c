//
// vmfaulttest: C2 lazy allocation 的缺页处理测试
//
// 说明：课程仓库（upstream: erenphy/ouc-os-vm）的 c0-lab / c1-lab 两个分支
// 都没有发布 user/vmfaulttest.c；本文件按实验手册 C2 §2.5 的四项要求自行补写，
// 输出格式对齐手册的标准输出范例。
//
// 四项：
//   [1] basic lazy allocation —— sbrklazy 一页，写首尾，读回校验
//   [2] multiple page faults  —— 连续碰多页，每页各触发一次缺页
//   [3] cross-page access     —— 跨页连续读写，验证页边界两侧都补上了
//   [4] invalid page fault    —— 访问 >= sz 的地址，子进程必须被 kill
//
#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int failed = 0;

static void
report(char *name, int ok)
{
  printf("%s... %s\n", name, ok ? "PASS" : "FAIL");
  if(!ok)
    failed = 1;
}

// [1] sbrklazy 一页，写首尾两个字节，读回校验。
// 同时检查 sbrk(0) 确实前进了 PGSIZE（虚拟空间涨了，但还没给物理页）。
static void
test_basic(void)
{
  uint64 before = (uint64)sbrk(0);
  char *p = sbrklazy(PGSIZE);

  if(p == (char*)SBRK_ERROR){
    report("[1] basic lazy allocation", 0);
    return;
  }

  int ok = ((uint64)sbrk(0) == before + PGSIZE);

  p[0] = 'A';
  p[PGSIZE-1] = 'Z';
  ok = ok && (p[0] == 'A') && (p[PGSIZE-1] == 'Z');

  report("[1] basic lazy allocation", ok);
}

// [2] 一次 sbrklazy 8 页，每页写一个标记；每页的首次写入各触发一次 fault。
static void
test_multiple(void)
{
  int npage = 8;
  char *p = sbrklazy(npage * PGSIZE);
  int i, ok = 1;

  if(p == (char*)SBRK_ERROR){
    report("[2] multiple page faults", 0);
    return;
  }

  for(i = 0; i < npage; i++)
    p[i*PGSIZE] = (char)(i + 1);

  for(i = 0; i < npage; i++)
    if(p[i*PGSIZE] != (char)(i + 1))
      ok = 0;

  report("[2] multiple page faults", ok);
}

// [3] 跨页连续读写：从页中间开始，写满 4 页，最后一个字节落在页尾之后。
// 只要任何一页没补上，中途就会 fault 失败或被 kill。
static void
test_cross_page(void)
{
  int npage = 4;
  int n = npage * PGSIZE;
  int off = PGSIZE / 2;          // 故意不从页边界开始
  char *p = sbrklazy(n + off);
  int i, ok = 1;

  if(p == (char*)SBRK_ERROR){
    report("[3] cross-page access", 0);
    return;
  }

  for(i = 0; i < n; i++)
    p[off + i] = (char)(i & 0xff);

  for(i = 0; i < n; i++)
    if(p[off + i] != (char)(i & 0xff))
      ok = 0;

  report("[3] cross-page access", ok);
}

// [4] 非法缺页：子进程写 sbrk(0) 返回的地址（== sz，从未分配），
// vmfault() 必须判定非法并 kill 它；父进程检查 wait 状态。
static void
test_invalid(void)
{
  printf("[4] invalid page fault...\n");

  // 先把 sz 对齐到页边界：这样 sz 本身是一页的页首，而这一页
  // 一定从未被映射（前面所有 sbrklazy 区域都止于 sz 之前）。
  // 否则 sz 落在某页中间时，该页可能已经被前面的测试补过了，
  // 写 sz 就不会 fault，测试会误判。
  uint64 sz = (uint64)sbrk(0);
  if(sz % PGSIZE)
    sbrklazy(PGSIZE - (sz % PGSIZE));
  sz = (uint64)sbrk(0);

  int pid = fork();
  if(pid < 0){
    report("    fork for invalid test", 0);
    return;
  }

  if(pid == 0){
    char *p = (char*)sz;         // == sz，页对齐，从未映射
    p[0] = 'x';
    exit(0);                     // 不应该走到这里
  }

  int status = -1;
  wait(&status);
  printf("%s\n", status != 0 ? "PASS" : "FAIL");
  if(status == 0)
    failed = 1;
}

int
main(void)
{
  printf("===OUC===c2: vmfault test ===\n");

  test_basic();
  test_multiple();
  test_cross_page();
  test_invalid();

  printf("=== c2 vmfault test finished ===\n");
  exit(failed);
}
