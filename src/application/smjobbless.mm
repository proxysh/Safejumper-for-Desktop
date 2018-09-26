
#include <QtGui>
#include <ServiceManagement/ServiceManagement.h>
#include <Security/Security.h>
#include <Security/Authorization.h>
#include <Security/Security.h>
#include <Security/SecCertificate.h>
#include <Security/SecCode.h>
#include <Security/SecStaticCode.h>
#include <Security/SecCodeHost.h>
#include <Security/SecRequirement.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFError.h>

#include "common.h"
#include "smjobbless.h"

// this function was adapted from the SMJobBless example
bool blessHelperWithLabel(CFStringRef label, CFErrorRef* error)
{
    bool result = false;

    AuthorizationItem authItem		= { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
    AuthorizationRights authRights	= { 1, &authItem };
    AuthorizationFlags flags		= kAuthorizationFlagDefaults |
                                        kAuthorizationFlagInteractionAllowed |
                                        kAuthorizationFlagPreAuthorize |
                                        kAuthorizationFlagExtendRights;
    AuthorizationRef authRef = NULL;

    /* Obtain the right to install privileged helper tools (kPRIVILEGED_HELPER_LABEL). */
    OSStatus status = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);
    if (status != errAuthorizationSuccess)
    {
        qCritical() << QObject::tr("Failed to create AuthorizationRef, return code %1").arg( (long)status);
    } else
    {
        /* This does all the work of verifying the helper tool against the application
         * and vice-versa. Once verification has passed, the embedded launchd.plist
         * is extracted and placed in /Library/LaunchDaemons and then loaded. The
         * executable is placed in /Library/PrivilegedHelperTools.
         */
        result = SMJobBless(kSMDomainSystemLaunchd, label, authRef, error);

        AuthorizationFree(authRef, kAuthorizationFlagDefaults);
    }

    return result;
}

// this function is adapted from https://bitbucket.org/sinbad/privilegedhelperexample
bool installPrivilegedHelperTool()
{
    // This uses SMJobBless to install a tool in /Library/PrivilegedHelperTools which is
    // run by launchd instead of us, with elevated privileges. This can then be used to do
    // things like install utilities in /usr/local/bin or run another app with admin rights

    // We do this rather than AuthorizationExecuteWithPrivileges because that's deprecated in 10.7
    // The SMJobBless approach is more secure because both ends are validated via code signing
    // which is enforced by launchd - ie only tools signed with the right cert can be installed, and
    // only apps signed with the right cert can install it.

    // Although the launchd approach is primarily associated with daemons, it can be used for one-off
    // tools too. We effectively invoke the privileged helper by talking to it over a private Unix socket
    // (since we can't launch it directly). We still need to be careful about that invocation because
    // the SMJobBless structure doesn't validate that the caller at runtime is the right application.
    // However, the privilegehelper validates the signature of the calling app and the command to execute.

    CFErrorRef  error = NULL;
    CFDictionaryRef installedHelperJobData = SMJobCopyDictionary(kSMDomainSystemLaunchd, CFSTR(kHELPER_LABEL));
    bool needToInstall = true;

    if (installedHelperJobData)
    {
        // This code vVerify wheather or not the PrivilegedHelper is installed as a privileged helper tool
        CFArrayRef arguments = (CFArrayRef)CFDictionaryGetValue (installedHelperJobData, CFSTR("ProgramArguments"));
        CFStringRef installedPath = (CFStringRef)CFArrayGetValueAtIndex(arguments, 0);
        CFURLRef installedPathURL = CFURLCreateWithString(kCFAllocatorDefault, installedPath, NULL);
        CFDictionaryRef installedInfoPlist = CFBundleCopyInfoDictionaryForURL(installedPathURL);
        CFStringRef installedBundleVersion = (CFStringRef)CFDictionaryGetValue (installedInfoPlist, CFSTR("CFBundleVersion"));
        int installedVersion = CFStringGetIntValue(installedBundleVersion);

        qInfo() << "installedVersion: " << (long)installedVersion;

        CFBundleRef appBundle = CFBundleGetMainBundle();
        CFURLRef appBundleURL = CFBundleCopyBundleURL(appBundle);

        qInfo() << "appBundleURL: " << appBundleURL;

        CFStringRef helperToolPath = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Contents/Library/LaunchServices/%@"), CFSTR(kHELPER_LABEL));
        CFURLRef currentHelperToolURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, appBundleURL, helperToolPath, false);

        qInfo() << "currentHelperToolURL: " << currentHelperToolURL;

        CFDictionaryRef currentInfoPlist = CFBundleCopyInfoDictionaryForURL(currentHelperToolURL);
        CFStringRef currentBundleVersion = (CFStringRef)CFDictionaryGetValue (currentInfoPlist, CFSTR("CFBundleVersion"));
        int currentVersion = CFStringGetIntValue(currentBundleVersion);

        qInfo() << "currentVersion: " << currentVersion;

        if ( currentVersion == installedVersion )
        {
            SecRequirementRef requirement;
            OSStatus stErr;
            CFStringRef reqStr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("identifier %@ and certificate leaf[subject.CN] = \"%@\""),
                                                          CFSTR(kHELPER_LABEL), CFSTR(kSigningCertCommonName));

            stErr = SecRequirementCreateWithString(reqStr, kSecCSDefaultFlags, &requirement );

            if ( stErr == noErr )
            {
                SecStaticCodeRef staticCodeRef;

                stErr = SecStaticCodeCreateWithPath( installedPathURL, kSecCSDefaultFlags, &staticCodeRef );

                if ( stErr == noErr )
                {
                    stErr = SecStaticCodeCheckValidity( staticCodeRef, kSecCSDefaultFlags, requirement );

                    needToInstall = false;
                }
            }

            CFRelease(reqStr);
        }

        CFRelease(helperToolPath);
        CFRelease(installedPathURL);
        CFRelease(appBundleURL);
        CFRelease(currentHelperToolURL);
        CFRelease(currentInfoPlist);
        CFRelease(installedInfoPlist);
        CFRelease(installedHelperJobData);
    }


    // When the PrivilegedHelper is not installed, we proceed to install it, using the blessHelperWithLabel function
    if (needToInstall)
    {
        if (!blessHelperWithLabel(CFSTR(kHELPER_LABEL), &error))
        {
            if (error) {
                CFStringRef cfErrDescription = CFErrorCopyDescription(error);
                const char *szErrDesc = CFStringGetCStringPtr(cfErrDescription, CFStringGetSystemEncoding());
                qCritical() << "Failed to install privileged helper: " << szErrDesc << error;
            } else {
                qCritical() << "Failed to install helper, but no error set, user probably hit cancel";
            }
            return false;
        }
        else
            qInfo() << "Privileged helper installed.";
    }
    else
        qInfo() << "Privileged helper already available, not installing.";

    return true;
}

