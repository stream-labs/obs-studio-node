#include "util-osx-impl.h"

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
    NSLog(@"getDefaultVideoSavePath");
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

@end