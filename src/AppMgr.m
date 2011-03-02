#import "AppMgr.h"
#import <logging.h>

#import <CoreFoundation/CoreFoundation.h>

@implementation AppMgr

- (id) init
{
	logverbose("enter\n");
	self = [super init];
	return self;
}

- (void) dealloc
{
	logverbose("enter\n");
	[super dealloc];
}

- (void) startRunLoop
{
	CFRunLoopRun();
}

- (void) stopRunLoop
{
	CFRunLoopStop(CFRunLoopGetMain());
}

@end
