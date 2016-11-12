#ifdef WINDOWS
#include <conio.h>
#else
#ifndef MYCONIO
#define MYCONIO

/*
 *  kbhit()  --  a keyboard lookahead monitor, returns amount of chars available to read. Allows for nonblocking input
 */
int kbhit();

/*
 *  nonblock(bool, bool)  --  Change canonical mode and echo state
 */
void nonblock(bool nb, bool echo);

char getch();		//Just for portability with windows

#endif
#endif
