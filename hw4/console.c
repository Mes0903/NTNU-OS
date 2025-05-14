//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

//
// send one character to the uart.
// called by printf(), and to echo input characters,
// but not from write().
//
void
consputc(int c)
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync('\b'); uartputc_sync(' '); uartputc_sync('\b');
  } else {
    uartputc_sync(c);
  }
}

struct {
  struct spinlock lock;
  
  // input
#define INPUT_BUF_SIZE 128
  char buf[INPUT_BUF_SIZE];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons;

/* APPEND FUNCTION & VAR*/

#define HIST_SIZE   5

static char history[HIST_SIZE][INPUT_BUF_SIZE];
static int  hist_cnt = 0;    // how many valid entries (0‑5)
static int  hist_head = 0;   // next slot to overwrite
static int  hist_cur  = -1;  // -1 = not browsing, else 0‑(hist_cnt‑1)

static void
clear_line(void)
{
  // delete everything
  while (cons.e != cons.w) {
    cons.e--;
    consputc(BACKSPACE);
  }
}

//
// user write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n)
{
  int i;

  for(i = 0; i < n; i++){
    char c;
    if(either_copyin(&c, user_src, src+i, 1) == -1)
      break;
    uartputc(c);
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n)
{
  uint target;
  int c;
  char cbuf;

  target = n;
  acquire(&cons.lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){
      if(killed(myproc())){
        release(&cons.lock);
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r++ % INPUT_BUF_SIZE];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
      break;

    dst++;
    --n;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  release(&cons.lock);

  return target - n;
}

//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c)
{
  acquire(&cons.lock);

  switch(c){
  case C('W'): {                   // browse older
    if (hist_cnt == 0 || hist_cur + 1 >= hist_cnt) 
      break;

    hist_cur++;
    clear_line();

    // print the requested line and copy it into input buffer
    char *line = history[(hist_head - 1 - hist_cur + HIST_SIZE) % HIST_SIZE];
    for (int i = 0; line[i] && cons.e - cons.r < INPUT_BUF_SIZE; i++) {
      consputc(line[i]);
      cons.buf[cons.e++ % INPUT_BUF_SIZE] = line[i];
    }
    break;
  }
  case C('S'): {                   // browse newer
    if (hist_cnt == 0) 
      break;

    if(hist_cur <= -1) {          // clear whole line
      clear_line();
      break;
    }
      
    hist_cur--;
    clear_line();

    if (hist_cur == -1)
      break;                       // show empty prompt

    char *line = history[(hist_head - 1 - hist_cur + HIST_SIZE) % HIST_SIZE];
    for (int i = 0; line[i] && cons.e - cons.r < INPUT_BUF_SIZE; i++) {
      consputc(line[i]);
      cons.buf[cons.e++ % INPUT_BUF_SIZE] = line[i];
    }
    break;
  }
  case C('P'):  // Print process list.
    procdump();
    break;
  case C('U'):  // Kill line.
    while(cons.e != cons.w &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
  case C('H'): // Backspace
  case '\x7f': // Delete key
    if(cons.e != cons.w){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
  default:
    if(c != 0 && cons.e-cons.r < INPUT_BUF_SIZE){
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      consputc(c);

      // store for consumption by consoleread().
      cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;

      if(c == '\n' || c == C('D') || cons.e-cons.r == INPUT_BUF_SIZE){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        if (c == '\n') {
          // copy from last newline (or cons.w) up to current '\n'
          int len = 0;
          for (int idx = cons.w; idx < cons.e && len < INPUT_BUF_SIZE-1; idx++)
            history[hist_head][len++] = cons.buf[idx % INPUT_BUF_SIZE];
          history[hist_head][len] = '\0';

          hist_head = (hist_head + 1) % HIST_SIZE;
          if (hist_cnt < HIST_SIZE) hist_cnt++;
          hist_cur = -1;               // reset browsing state
        }

        cons.w = cons.e;
        wakeup(&cons.r);
      }
    }
    break;
  }
  
  release(&cons.lock);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  uartinit();

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
