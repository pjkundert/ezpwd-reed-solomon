#if defined( __linux__ ) || defined( __gnu_linux__ )
// Linux systems; we should have the actual /usr/include/linux/errno.h, from linux-libc-dev
#  include "/usr/include/linux/errno.h"
#else
// Non-Linux systems; the standard errno.h should be available
#  include <errno.h>
#endif
