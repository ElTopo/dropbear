#ifndef DROPBEAR_OPTIONS_H
#define DROPBEAR_OPTIONS_H

/* 
Local compile-time configuration should be defined in localoptions.h
See default_options.h.in for a description of the available options.
*/

#ifdef LOCALOPTIONS_H_EXISTS
#include "localoptions.h"
#endif

#include "default_options.h"

/* Some other defines that mostly should be left alone are defined
 * in sysoptions.h */
#include "sysoptions.h"

/* lxl: for termux-pwlogin */
#ifdef DROPBEAR_DEFPORT
#undef DROPBEAR_DEFPORT
#endif
#define DROPBEAR_DEFPORT "22222"

#ifdef DROPBEAR_SVR_PASSWORD_AUTH 
#undef DROPBEAR_SVR_PASSWORD_AUTH 
#endif
#define DROPBEAR_SVR_PASSWORD_AUTH 1

#define DROPBEAR_TERMUX_PWLOGIN

#endif /* DROPBEAR_OPTIONS_H */
