#include "util-osx-impl.h"
#include <iostream>

@implementation UtilImplObj

UtilObjCInt::UtilObjCInt(void)
    : self(NULL)
{   }

UtilObjCInt::~UtilObjCInt(void)
{
    [(id)self dealloc];
}

void UtilObjCInt::init(void)
{
    self = [[UtilImplObj alloc] init];
}

std::string UtilObjCInt::getDefaultVideoSavePath(void)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSURL *url = [fm URLForDirectory:NSMoviesDirectory
				inDomain:NSUserDomainMask
		       appropriateForURL:nil
				  create:true
				   error:nil];

	if (!url)
		return getenv("HOME");

	return url.path.fileSystemRepresentation;
}

void UtilObjCInt::createApplication(void)
{
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app run];
    }
}

void UtilObjCInt::terminateApplication(void)
{
	dispatch_sync(dispatch_get_main_queue(), ^{
		[NSApp stop:nil];
    });
}

bool UtilObjCInt::getPermissionsStatus(void)
{
	AVAuthorizationStatus authStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

	switch(authStatus) {
		case AVAuthorizationStatusAuthorized: {
			std::cout << "Permissions granted" << std::endl;
			return true;
		}
		case AVAuthorizationStatusNotDetermined:
		case AVAuthorizationStatusRestricted:
		default:{
			std::cout << "Permissions denied" << std::endl;
			[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted)
			{
				if(granted)
				{
					std::cout << "Granted" << std::endl;
				}
				else
				{
					std::cout << "Denied" << std::endl;
				}
			}];
			return false;
		}
	}
}

@end