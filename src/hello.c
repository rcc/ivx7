#include <stdio.h>

#include <prjutil.h>

int main(void)
{
	pdebug("%s : Debug Prints are ON\n", __FILE__);
	printf("Hello, World\n");
	return 0;
}
