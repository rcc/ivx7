#include <prjutil.h>
#include <cmds.h>

#include <stdio.h>

int main(int argc, const char * argv[])
{
	int ret;
	int status = 0;

	pdebug("Version: %s\n", SCM_HASH);

	if((ret = run_cmds(argc-1, &argv[1], NULL)) < 0) {
		fprintf(stderr,
			"An error occurred with command at position %d\n",
			-ret);
		status = -1;
		goto exit;
	}

exit:
	return status;
}

CMDHANDLER(version)
{
	printf("Version: %s\n", SCM_HASH);
	return 0;
}
APPCMD(version, &version, "print the version", "usage: version", NULL);
