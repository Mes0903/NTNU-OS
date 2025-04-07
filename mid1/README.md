# 1. (25 points)

For each of the following statements, answer $T$ if you think it is true; otherwise, answer $F$.  
No need to show your reasoning here.

1. **(a) (5 points)** In OS terminology, by *returning-from-trap* the system will raise the privilege level from user mode to kernel mode.  
2. **(b) (5 points)** A *page fault* will occur when the OS cannot perform address translation from the TLB.  
3. **(c) (5 points)** When the OS performs a context switch from process $A$ to process $B$, the OS would save its kernel register values into the PCB of process $B$.  
4. **(d) (5 points)** The design of a multi-level feedback queue prevents job starvation by resetting a job’s priority to the top.  
5. **(e) (5 points)** When a user-space program prints out the memory address of one of its local variables, the printed address is a virtual address.

# 2. (10 points)

Suppose we have four virtual pages (labeled 0, 1, 2, 3) and two empty physical frames.  
Consider the following VPN access sequence:  

$$
1 → 0 → 2 → 1 → 0 → 2 → 0 → 3
$$

How many page faults will occur if the system uses the LRU replacement policy?

## 3. (15 points)

Consider the following three jobs. Using the RR scheduling policy (scheduling quantum = 3), what would be the average turnaround time of these three?

- **Job A:** arrival time = 0; job length = 6  
- **Job B:** arrival time = 1; job length = 6  
- **Job C:** arrival time = 2; job length = 6

## 4. (15 points)

Given a 512-byte address space and a 4KB physical memory, use the following segment registers information to translate the virtual address `0x16c` to its physical address. Write your answer in hexadecimal, or write “segmentation fault” if you think the translation is illegal. No need to consider the size of the information beginning at each address.

| Segment | Base   | Size (byte) | Grows Positive? |
|---------|--------|-------------|-----------------|
| $Code_{00}$  | `0x723`  | 100         | 1               |
| $Heap_{01}$  | `0x921`  | 100         | 1               |
| $Stack_{11}$ | `0xb20`  | 30          | 0               |

## 5. (15 points)

Consider the following C function. Assuming no failure, how many `"foo "` strings will this part of the code print?

```c
// ...
for (int i = 0; i < 2; i++)
{
    int rc = fork();
    if (rc == 0) {
        fork();
        printf("foo ");
    }
    fork();
}
// ...
```

## 6. (5 points)

Pick all the correct statement(s) from the following four, or write **NONE** if you think none of them is correct. Points will be given only if you have made the correct judgment on every statement. Watch out for the logic.

1. A zombie process is a parent process whose children processes all have terminated and did not call a `wait()` for it.  
2. A zombie process is a child process whose parent process has terminated and did not call a `wait()` for the child.  
3. A process becomes a zombie if it has finished its execution but its original parent process is still running and has not called a `wait()`.  
4. A process becomes a zombie if its original parent process has terminated before the process has finished its execution.

## 7. (15 points)

Perform an address translation using a two-level page table. Some assumptions:

- The page size is 32 bytes.
- Physical memory consists of 128 pages. A physical address requires 12 bits.
- The address space for a process has 1024 virtual pages. Assume that there is only one process to be considered.

The PTE is 8 bits and the format is `VALID PFN6 ... PFN0`, starting from the MSB.  
The PDE is 8 bits and the format is `VALID PT6 ... PT0`, starting from the MSB.

Figures 1 and 2 show a hexadecimal dump of some pages of memory. In each page dump, the first byte is shown on the left; the last byte, on the right.

Suppose that the page table directory (aka the page directory) is held in page 118 (decimal).  
Translate the virtual address `0x5224` into the corresponding physical address. Write your answer in hexadecimal.

> 原諒我這我實在不想打

![alt text](image.png)

![alt text](image-1.png)