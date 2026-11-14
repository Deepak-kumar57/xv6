#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#ifdef PGTBL_SOL
#include "riscv.h"
#endif
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
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
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > UTOP)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_vmprint(void)
{
  struct proc *p;

  p = myproc();
  vmprint(p->pagetable);
  return 0;
}
#endif

#ifdef LAB_PGTBL
// pgaccess(start va, number of pages, user address of bitmask).
// Sets bit i of the mask if page i was accessed since the last call,
// then clears PTE_A. Returns 0 on success, -1 on error.
// Heap pages are allocated lazily, so a page inside p->sz that has no
// PTE yet simply counts as "not accessed". Pages beyond p->sz are an error.
#define PGACCESS_MAX (PGSIZE * 8) // one kernel page of bits
uint64
sys_pgaccess(void)
{
  uint64 va, maskaddr;
  int npages;
  struct proc *p = myproc();

  argaddr(0, &va);
  argint(1, &npages);
  argaddr(2, &maskaddr);

  if (npages <= 0 || npages > PGACCESS_MAX)
    return -1;

  va = PGROUNDDOWN(va);
  if (va >= MAXVA || va + (uint64)npages * PGSIZE > p->sz)
    return -1; // range goes past the process's memory

  // kernel temporary buffer for the bitmask (too big for the stack)
  char *mask = kalloc();
  if (mask == 0)
    return -1;
  memset(mask, 0, PGSIZE);

  for (int i = 0; i < npages; i++) {
    uint64 a = va + (uint64)i * PGSIZE;
    pte_t *pte = walk(p->pagetable, a, 0);
    if (pte == 0 || (*pte & PTE_V) == 0)
      continue; // lazily-allocated page not touched yet: not accessed
    if ((*pte & PTE_U) == 0) {
      kfree(mask); // e.g. stack guard page: user can't access it
      return -1;
    }
    if (*pte & PTE_A) {
      mask[i / 8] |= (1 << (i % 8));
      *pte &= ~PTE_A; // clear it, or it stays set forever
    }
  }

  // copy only as many bytes as needed for npages bits
  int nbytes = (npages + 7) / 8;
  int r = copyout(p->pagetable, p->sz, maskaddr, mask, nbytes);
  kfree(mask);
  return r < 0 ? -1 : 0;
}
#endif

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

#ifdef LAB_LOCK
uint64
sys_cpupin(void)
{
  struct proc *p = myproc();
  int cpu;

  argint(0, &cpu);
  if (cpu < 0 || cpu >= NCPU)
    return -1;
  acquire(&p->lock);
  p->pincpu = &cpus[cpu];
  release(&p->lock);
  return 0;
}
#endif
