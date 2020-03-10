#include "util-osx-impl.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <string>

#import <mach/mach.h>

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

unsigned long long UtilObjCInt::getTotalPhysicalMemory(void)
{
	return [NSProcessInfo processInfo].physicalMemory;
}

unsigned long long UtilObjCInt::getAvailableMemory(void)
{
    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;

    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);

    vm_statistics_data_t vm_stat;

    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
        return 0;

    // uint64_t mem_used = (uint64_t)(vm_stat.active_count +
    //                       vm_stat.inactive_count +
    //                       vm_stat.wire_count) * (uint64_t)pagesize;

    uint64_t mem_free = (uint64_t)vm_stat.free_count * (uint64_t)pagesize * 10;
	return mem_free;
}

std::vector<std::pair<uint32_t, uint32_t>> UtilObjCInt::getAvailableScreenResolutions(void)
{
    std::vector<std::pair<uint32_t, uint32_t>> resolutions;

    for (NSScreen* screen: [NSScreen screens]) {
        resolutions.push_back(std::make_pair(
            [screen frame].size.width,
            [screen frame].size.height
        ));
    }

    return resolutions;
}

std::string UtilObjCInt::getUserDataPath(void)
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory,
        NSUserDomainMask,
        YES);
    if ([paths count] == 0)
        return "";

    NSString* userPath = [paths objectAtIndex:0];
    return std::string([userPath UTF8String]);
}

std::string UtilObjCInt::getWorkingDirectory(void)
{
    // NSFileManager* fm = [[NSFileManager alloc] init];
    // NSString* workindDirPath = [fm currentDirectoryPath];
    NSString* workindDirPath = [[NSProcessInfo processInfo] environment][@"PWD"];
    return std::string([workindDirPath UTF8String]);
}

@end
