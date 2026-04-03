/*
 * Local wrapper for the internal mbedTLS pk_wrap header.
 *
 * CubeIDE imports in this repo do not always propagate the
 * ARM_Security/library include path into the generated makefiles, while
 * several application sources include "pk_wrap.h" directly. Keep a shim in an
 * existing application include path so both CubeIDE and make-based builds
 * resolve the header consistently.
 */

#ifndef APP_LOCAL_PK_WRAP_H
#define APP_LOCAL_PK_WRAP_H

#include "../../../Middlewares/Third_Party/ARM_Security/library/pk_wrap.h"

#endif /* APP_LOCAL_PK_WRAP_H */
