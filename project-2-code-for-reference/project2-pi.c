#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int c;
	system("/bin/stty raw -echo");
	while ((c = getchar()) != '.')
		putchar(c);
	system("/bin/stty cooked echo");

	return 0;
}
