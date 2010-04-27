#import <Foundation/Foundation.h>

#import <prjutil.h>
#import <cmds.h>

#import <stdio.h>

int main(int argc, const char * argv[])
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int ret;
	int status = 0;

	logdebug("Version: %s\n", SCM_HASH);

	if((ret = run_cmds(argc-1, &argv[1], NULL)) < 0) {
		printf("An error occurred with command at position %d\n", -ret);
		status = -1;
		goto exit;
	}

exit:
	[pool drain];
	return status;
}

CMDHANDLER(version)
{
	printf("Version: %s\n", SCM_HASH);
	return 0;
}
APPCMD(version, &version, "print the version", "usage: version", NULL);
