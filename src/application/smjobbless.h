
#ifndef SMJOBBLESS_H
#define SMJOBBLESS_H

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFError.h>

bool blessHelperWithLabel(CFStringRef label, CFErrorRef* error);
bool installPrivilegedHelperTool();

#endif // SMJOBBLESS

