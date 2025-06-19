#include <freeDiameter/extension.h>

int test_auth_entry(char * conffile) {
    fd_log_debug("test_auth plugin loaded with conf: %s\n", conffile);
    return 0;
}

EXTENSION_ENTRY("test_auth", test_auth_entry);
