#include "util-osx-impl.h"
#include <iostream>

std::string g_server_working_dir;

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

	m_webcam_perm = false;
	m_mic_perm    = false;
}

void UtilObjCInt::getPermissionsStatus(bool &webcam, bool &mic)
{
	NSOperatingSystemVersion OSversion = [NSProcessInfo processInfo].operatingSystemVersion;
	if (OSversion.majorVersion >= 10 && OSversion.minorVersion >= 14) {
		AVAuthorizationStatus camStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
		webcam = camStatus == AVAuthorizationStatusAuthorized;

		AVAuthorizationStatus micStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
		mic = micStatus == AVAuthorizationStatusAuthorized;

		m_webcam_perm = webcam;
		m_mic_perm    = mic;
	} else {
		webcam = true;
		mic = true;
	}
}

void UtilObjCInt::requestPermissions(void *async_cb, perms_cb cb)
{
	NSOperatingSystemVersion OSversion = [NSProcessInfo processInfo].operatingSystemVersion;
	if (OSversion.majorVersion >= 10 && OSversion.minorVersion >= 14) {
		m_async_cb = async_cb;
		[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted)
		{
			m_webcam_perm = granted;
			cb(m_async_cb, m_webcam_perm, m_mic_perm);
		}];

		[AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted)
		{
			m_mic_perm = granted;
			cb(m_async_cb, m_webcam_perm, m_mic_perm);
		}];
	}
}

void UtilObjCInt::setServerWorkingDirectoryPath(std::string path)
{
	g_server_working_dir = path;
}

void UtilObjCInt::installPlugin()
{
	NSDictionary *error = [NSDictionary dictionary];
	NSString* workindDirPath = [[NSProcessInfo processInfo] environment][@"PWD"];
	std::string pwd =  std::string([workindDirPath UTF8String]);
	std::string pathToScript = g_server_working_dir + "/data/obs-plugins/slobs-virtual-cam/install-plugin.sh";
	std::cout << "launching: " << pathToScript.c_str() << std::endl;
	std::string cmd = "do shell script \"/bin/sh " + pathToScript + " " + g_server_working_dir + "/data/obs-plugins/slobs-virtual-cam" + "\" with administrator privileges";
	NSString *script = [NSString stringWithCString:cmd.c_str()
                                   encoding:[NSString defaultCStringEncoding]];
	NSAppleScript *run = [[NSAppleScript alloc]initWithSource:script];
	[run executeAndReturnError:&error];
}

@end
