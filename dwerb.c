#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <termios.h>
#include <unistd.h>

/**** defines ****/
// taking a parameter k and using a BITWISE AND operation to set it to the
// Ctrl+parameter key
//
#define CTRL_KEY(k) ((k) & 0x1f)

/**** data ****/
struct termios orig_termios;

/**** terminal ****/
// error handling, gives out what char gave the error
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
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
  // disabling OPOST disables the \n to \r\n translation and so instead of
  // printing
  /*
   Expected     Output
   a            a
   b              b
   c                c
  A carriage return or \r would fix this and this is implemented in printf
  */

  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  // VMIN is the minimum number of bytes needed before read() returns a value
  raw.c_cc[VMIN] = 0;
  // VTIME is the maximum time before read() returns something
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/**** output ****/
void editorDrawRows() {
  int y;
  // harcoding terminal size to 24 come back and change
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
  // clearing screen
  write(STDOUT_FILENO, "\x1b[2J", 4);
  // repositioning cursor
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}
char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

/*** input ***/
void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}

/**** init ****/
int main() {
  enableRawMode();
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return 0;
}
