/**** includes ****/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/**** data ****/
struct termios orig_termios;

/**** terminal ****/
// error handling, gives out what char gave the error
void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    die("tcgetattr");
  }

  atexit(disableRawMode);
  struct termios raw = orig_termios;
  // ICANON helps read input byte by byte instead of line by line
  // ISIG prevents Ctrl+z SIGTSTP and Ctrl+c which gives out SIGINT
  // IXON is to prevent Ctrl+s and Ctrl+q to stop and continue transmission to
  // the terminal
  // IEXETEN Prevents ctrl+v which makes the terminal wait while you type
  // another character

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  // disabling OPOST disables the /n to /r/n translation and so instead of
  // printing
  /*
   Expected     Output
   a            a
   b              b
   c                c
  A carriage return would fix this and this is implemented in printf
  */

  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  // VMIN is the minimum number of bytes needed before read() returns a value
  raw.c_cc[VMIN] = 0;
  // VTIME is the maximum number of time before read() returns something
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/**** init ****/
int main() {
  enableRawMode();
  while (1) {
    char c = '\0';
    // exits when you have q in the stream
    if (read(STDIN_FILENO, &c, 1) == -1) {
      die("tcsetattr");
    };

    // if a control character return the ASCII value
    if (iscntrl(c)) {
      // do a carriage return \r in printf to fix the lack of OPOST
      printf("%d\r\n", c);
      // else return ascii value and the character
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') {
      break;
    }
  }

  return 0;
}
