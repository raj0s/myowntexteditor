#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
// ioctl is used to grab the size of the terminal window that is currently being
// worked in
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/**** defines ****/
// taking a parameter k and using a BITWISE AND operation to set it to the
// Ctrl+parameter key
//

/**** data ****/
#define CTRL_KEY(k) ((k) & 0x1f)

// create a struct which holds the holds the state of the terminal
struct editorConfig {
  int srows;
  int scols;
  struct termios orig_termios;
};

struct editorConfig E;

/**** terminal ****/

// error handling, gives out what char gave the error
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
    die("tcgetattr");
  }

  atexit(disableRawMode);
  struct termios raw = E.orig_termios;
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

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  printf("\r\n&buf[1]: %s\r\n", &buf[1]);
  editorReadKey();
  return -1;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  } else {
    // we pass the values by setting the values to the int references of cols
    // and rows that were passed orignally in the function this helps C
    // returns multiple things along with having functions return multiple
    // values in C.
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/**** output ****/
void editorDrawRows() {
  int y;
  // harcoding terminal size to 24 come back and change
  for (y = 0; y < E.srows; y++) {
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
void initEditor() {
  if (getWindowSize(&E.srows, &E.scols) == -1)
    die("getWindowSize");
}

int main() {
  enableRawMode();
  initEditor();
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return 0;
}
