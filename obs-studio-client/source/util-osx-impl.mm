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

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void UtilObjCInt::installPlugin()
{
	char ch;
	FILE *fpw;
	fpw = fopen("/tmp/log-osn-client.txt","w");

	if(fpw == NULL)
	{
		printf("Error");   
		exit(1);             
	}

	fprintf(fpw,"%c", "test - 0");

	NSDictionary *error = [NSDictionary dictionary];
	std::string pathToScript = g_server_working_dir + "/data/obs-plugins/slobs-virtual-cam/install-plugin.sh";
	std::cout << "launching: " << pathToScript.c_str() << std::endl;

	fprintf(fpw,"%c", "test - 1");
	replace(pathToScript, " ", "\\\\ ");
	std::string arg = g_server_working_dir + "/data/obs-plugins/slobs-virtual-cam";
	fprintf(fpw,"%c", "test - 2");
	replace(arg, " ", "\\\\ ");
	std::string cmd = "do shell script \"/bin/sh " + pathToScript + " " + arg + "\" with administrator privileges";

	fprintf(fpw,"%c", "test - 3");
	NSString *script = [NSString stringWithCString:cmd.c_str()
	                            encoding:[NSString defaultCStringEncoding]];
	NSAppleScript *run = [[NSAppleScript alloc]initWithSource:script];
	fprintf(fpw,"%c", "test - 4");
	[run executeAndReturnError:&error];
	fprintf(fpw,"%c", "test - 5");
	NSLog(@"errors: %@", error);
	fclose(fpw);
}

@end
