#ifndef _MK_DEBUG_H_
#define _MK_DEBUG_H_


#define GLOBAL_DEBUG_ON			// show ALL Debug INFO...






#if defined( GLOBAL_DEBUG_ON )

#include "at_custom.h"

#define INFO( format, ... )  do { \
        char buf[1024];  \
        os_sprintf( buf, format, ## __VA_ARGS__ );	\
        at_port_print(buf);  \
    } while(0)
#else
#define INFO( format, ... )
#endif

#endif /* _MK_DEBUG_H_ */
