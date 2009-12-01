#include <stdio.h>

#include <prjutil.h>

int main(void)
{
	pdebug("Version: %s\n", SCM_HASH);
	printf("Hello, World\n");
	return 0;
}
