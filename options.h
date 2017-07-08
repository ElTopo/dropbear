#ifndef DROPBEAR_OPTIONS_H
#define DROPBEAR_OPTIONS_H

/* 
            > > > Don't edit this file any more! < < <
            
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
#define DROPBEAR_TERMUX_PWLOGIN

/* override default options */
#ifdef DROPBEAR_DEFPORT
#undef DROPBEAR_DEFPORT
#endif
#define DROPBEAR_DEFPORT "22222"

#ifdef DROPBEAR_SVR_PASSWORD_AUTH 
#undef DROPBEAR_SVR_PASSWORD_AUTH 
#endif
#define DROPBEAR_SVR_PASSWORD_AUTH 1

#ifdef DSS_PRIV_FILENAME
#undef DSS_PRIV_FILENAME
#endif
#define DSS_PRIV_FILENAME "/data/data/com.termux/files/usr/etc/dropbear/dropbear_dss_host_key"

#ifdef RSA_PRIV_FILENAME
#undef RSA_PRIV_FILENAME
#endif
#define RSA_PRIV_FILENAME "/data/data/com.termux/files/usr/etc/dropbear/dropbear_rsa_host_key"

#ifdef ECDSA_PRIV_FILENAME
#undef ECDSA_PRIV_FILENAME
#endif
#define ECDSA_PRIV_FILENAME "/data/data/com.termux/files/usr/etc/dropbear/dropbear_ecdsa_host_key"

/* The MOTD file path */
#ifdef MOTD_FILENAME
#undef MOTD_FILENAME
#endif
#define MOTD_FILENAME "/data/data/com.termux/files/usr/etc/motd"

/* The default file to store the daemon's process ID, for shutdown
   scripts etc. This can be overridden with the -P flag */
#ifdef DROPBEAR_PIDFILE
#undef DROPBEAR_PIDFILE
#endif
#define DROPBEAR_PIDFILE "/data/data/com.termux/files/usr/var/run/dropbear.pid"

/* The command to invoke for xauth when using X11 forwarding.
 * "-q" for quiet */
#ifdef XAUTH_COMMAND
#undef XAUTH_COMMAND
#endif
#define XAUTH_COMMAND "/data/data/com.termux/files/usr/bin/xauth -q"

/* if you want to enable running an sftp server (such as the one included with
 * OpenSSH), set the path below. If the path isn't defined, sftp will not
 * be enabled */
#ifdef SFTPSERVER_PATH
#undef SFTPSERVER_PATH
#endif
#define SFTPSERVER_PATH "/data/data/com.termux/files/usr/libexec/sftp-server"

/* This is used by the scp binary when used as a client binary. If you're
 * not using the Dropbear client, you'll need to change it */
#ifdef DROPBEAR_PATH_SSH_PROGRAM
#undef DROPBEAR_PATH_SSH_PROGRAM
#endif
#define DROPBEAR_PATH_SSH_PROGRAM "/data/data/com.termux/files/usr/bin/dbclient"

/* The default path. This will often get replaced by the shell */
#ifdef DEFAULT_PATH
#undef DEFAULT_PATH
#endif
#define DEFAULT_PATH "/data/data/com.termux/files/usr/bin:/system/bin"

#endif /* DROPBEAR_OPTIONS_H */
