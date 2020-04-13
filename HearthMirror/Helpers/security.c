//
//  security.c
//  HearthMirror
//
//  Created by Istvan Fehervari on 26/12/2016.
//  Copyright © 2016 com.ifehervari. All rights reserved.
//

#include "security.h"

#ifdef __APPLE__

#define EXPORT __attribute__((visibility("default")))

#include <Security/Authorization.h>
#include <Security/SecBase.h>
#include <CoreFoundation/CoreFoundation.h>

EXPORT int acquireTaskportRight()
{
    OSStatus stat;
    AuthorizationItem taskport_item[] = {{"system.privilege.taskport:"}};
    AuthorizationRights rights = {1, taskport_item};
    AuthorizationRef author;
    CFStringRef stat_msg;

    AuthorizationFlags auth_flags = kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize | kAuthorizationFlagInteractionAllowed | ( 1 << 5);
    
    stat = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,auth_flags,&author);
    stat_msg = SecCopyErrorMessageString(stat,NULL);
    CFShow(stat_msg);
    CFRelease(stat_msg);

    if (stat != errAuthorizationSuccess)
    {
        AuthorizationFree(author, auth_flags);
        return 0;
    }
    
    stat = AuthorizationCopyRights( author, &rights, kAuthorizationEmptyEnvironment, auth_flags, NULL/*&out_rights*/);
    if (stat != errAuthorizationSuccess)
    {
        AuthorizationFree(author, auth_flags);
        printf("fail");
        return 1;
    }
    AuthorizationFree(author, auth_flags);
    return 0;
}
#endif
