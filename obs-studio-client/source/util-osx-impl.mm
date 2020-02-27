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

void UtilObjCInt::getPermissionsStatus(bool &webcam, bool &mic)
{
	AVAuthorizationStatus camStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
	if (camStatus == AVAuthorizationStatusAuthorized)
		webcam = true;
	else
		webcam = false;

	AVAuthorizationStatus micStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
	if (micStatus == AVAuthorizationStatusAuthorized)
		mic = true;
	else
		mic = false;

	// switch(authStatus) {
	// 	case AVAuthorizationStatusAuthorized: {
	// 		std::cout << "Permissions granted" << std::endl;
	// 		return true;
	// 	}
	// 	case AVAuthorizationStatusNotDetermined:
	// 	case AVAuthorizationStatusRestricted:
	// 	default:{
	// 		std::cout << "Permissions denied" << std::endl;
	// 		[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted)
	// 		{
	// 			if(granted)
	// 			{
	// 				std::cout << "Granted" << std::endl;
	// 			}
	// 			else
	// 			{
	// 				std::cout << "Denied" << std::endl;
	// 			}
	// 		}];
	// 		return false;
	// 	}
	// }
}

void UtilObjCInt::requestPermissions(void)
{
	[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted)
	{
		if(granted)
		{
			std::cout << "Granted Webcam" << std::endl;
		}
		else
		{
			std::cout << "Denied Webcam" << std::endl;
		}
	}];

	[AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted)
	{
		if(granted)
		{
			std::cout << "Granted Mic" << std::endl;
		}
		else
		{
			std::cout << "Denied Mic" << std::endl;
		}
	}];
}

@end
