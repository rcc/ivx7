#import "AppMgr.h"
#import <logging.h>

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

@end
