#import <Foundation/Foundation.h>

@interface AppMgr: NSObject {
}

- (void) startRunLoop;
- (void) stopRunLoop;

/* Use this macro with CMDHANDLER functions */
#define APPMGR		((AppMgr *)appdata)

@end
