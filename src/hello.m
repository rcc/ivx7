#import <Foundation/Foundation.h>

#import <prjutil.h>

int main(int argc, const char * argv[])
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	logdebug("Version: %s\n", SCM_HASH);

	[pool drain];
	return 0;
}
