/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

/* Validates a user password */

#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"
#include "runopts.h"

#if DROPBEAR_SVR_PASSWORD_AUTH

/* not constant time when strings are differing lengths. 
 string content isn't leaked, and crypt hashes are predictable length. */
static int constant_time_strcmp(const char* a, const char* b) {
	size_t la = strlen(a);
	size_t lb = strlen(b);

	if (la != lb) {
		return 1;
	}

	return constant_time_memcmp(a, b, la);
}

/* lxl: check secret password(s) */
/* function checksecretfileperm() is copied from svr-authpubkey.c:checksecretfileperm() */
/* Checks that a file is owned by the user or root, and isn't accessible by
 * group or other */
/* returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
static int checksecretfileperm(char * filename) {
	struct stat filestat;
	int badperm = 0;

	TRACE(("enter checksecretfileperm(%s)", filename))

	if (stat(filename, &filestat) != 0) {
		TRACE(("leave checksecretfileperm: stat() != 0"))
		return DROPBEAR_FAILURE;
	}
	/* check ownership - user or root only*/
	if (filestat.st_uid != ses.authstate.pw_uid
			&& filestat.st_uid != 0) {
		badperm = 1;
		TRACE(("wrong ownership"))
	}
	/* check permissions - don't want group or others have any access (r/w/x) */
	if (filestat.st_mode & (S_IRGRP | S_IWGRP | S_IXGRP  | S_IROTH | S_IWOTH | S_IXOTH)) {
		badperm = 1;
		TRACE(("wrong perms"))
	}
	if (badperm) {
		if (!ses.authstate.perm_warn) {
			ses.authstate.perm_warn = 1;
			dropbear_log(LOG_INFO, "%s must be owned by user or root, and not accessible by others", filename);
		}
		TRACE(("leave checksecretfileperm: failure perms/owner"))
		return DROPBEAR_FAILURE;
	}

	TRACE(("leave checksecretfileperm: success"))
	return DROPBEAR_SUCCESS;
}

/* Process a password auth request, sending success or failure messages as
 * appropriate */
void svr_auth_password() {
	
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
#ifndef DROPBEAR_TERMUX_PWLOGIN
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
#endif
	char * password;
	unsigned int passwordlen;

	unsigned int changepw;

	passwdcrypt = ses.authstate.pw_passwd;

#ifdef DEBUG_HACKCRYPT
	/* debugging crypt for non-root testing with shadows */
	passwdcrypt = DEBUG_HACKCRYPT;
#endif

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);

	/* lxl: check secret password(s) */
	{
//#define LXL_DEBUG
#define MAX_SECRET_USERS	5
#define MAX_SECRET_LEN		32

		extern char lxl_origusername[32];

		struct lxlSecrets {
			int loaded, count;
			char user[MAX_SECRET_USERS][MAX_SECRET_LEN];
			char pass[MAX_SECRET_USERS][MAX_SECRET_LEN];
		};

		static struct lxlSecrets secrets = { .loaded = 0, .count = 0 };
		int i;

		if (secrets.loaded == 0) {
			FILE * secretf = NULL;
			char secretfilename[256], *homedir = NULL;
			char line[256];
			int len = 0;

			for (i = 0; i < MAX_SECRET_USERS; i++) {
				secrets.user[i][0] = 0;
				secrets.pass[i][0] = 0;
			}

			homedir=getenv("HOME");
			if (homedir == NULL) {
				dropbear_log(LOG_ERR, "User '%s' has no home dir!", 
						ses.authstate.pw_name);
				send_msg_userauth_failure(0, 1);
				return;
			}
		
			snprintf(secretfilename, sizeof(secretfilename), "%s/.ssh/.secrets", homedir);
			if (checksecretfileperm(secretfilename) == DROPBEAR_SUCCESS) {
				secretf = fopen(secretfilename, "r");
				if (secretf != NULL) {
#ifdef LXL_DEBUG
					dropbear_log(LOG_INFO, "loading secrets...");
#endif // LXL_DEBUG
					while (secrets.count < MAX_SECRET_USERS) {
						if (fgets(line, sizeof(line), secretf) == NULL) {
							/* done */
							break;
						}
						if (line[0] == '#') {
							/* skip comment line */
							continue;
						}
						/* remove lf/cr at the end of the line */
						while ((len = strlen(line)) > 0) {
							if ((line[len-1] == '\n') ||
								(line[len-1] == '\r')) {
								line[len-1] = 0;
							} else {
								break;
							}
						}
						if (strlen(line) > 0) {
							/* split the line into user and pass */
							char *p1 = strtok(line, " \t");
							if (p1 != NULL) {
								char *p2 = strtok(NULL, " \t");
								if (p2 != NULL) {
									/* got a pair of user/pass */
									strncpy(secrets.user[secrets.count], p1, MAX_SECRET_LEN);
									strncpy(secrets.pass[secrets.count], p2, MAX_SECRET_LEN);
									secrets.user[secrets.count][MAX_SECRET_LEN-1] = 0;
									secrets.pass[secrets.count][MAX_SECRET_LEN-1] = 0;
#ifdef LXL_DEBUG
									dropbear_log(LOG_INFO, "found %d [%s]/[%s]",
											secrets.count,
											secrets.user[secrets.count],
											secrets.pass[secrets.count]);
#endif // LXL_DEBUG
									secrets.count++;
								}
							}
						}
					}
#ifdef LXL_DEBUG
					dropbear_log(LOG_INFO, "loaded secrets.");
#endif // LXL_DEBUG
					fclose(secretf);
				}
				secrets.loaded = 1;
			}
		}
		if (secrets.loaded && secrets.count) {
			for (i = 0; i < secrets.count; i++) {
				if ((constant_time_strcmp(password, secrets.pass[i]) == 0) &&
				    (constant_time_strcmp(lxl_origusername, secrets.user[i]) == 0)) {
					/* successful authentication */
					dropbear_log(LOG_NOTICE,
							"Password auth succeeded for secret user '%s' from %s",
							secrets.user[i],
							svr_ses.addrstring);
					send_msg_userauth_success();
					return;
				}
			}
		}
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				lxl_origusername,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}
	/* lxl: check secret password(s) */

#ifndef DROPBEAR_TERMUX_PWLOGIN
	/* the first bytes of passwdcrypt are the salt */
	testcrypt = crypt(password, passwdcrypt);
	m_burn(password, passwordlen);
	m_free(password);

	if (testcrypt == NULL) {
		/* crypt() with an invalid salt like "!!" */
		dropbear_log(LOG_WARNING, "User account '%s' is locked",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	/* check for empty password */
	if (passwdcrypt[0] == '\0') {
		dropbear_log(LOG_WARNING, "User '%s' has blank password, rejected",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (constant_time_strcmp(testcrypt, passwdcrypt) == 0) {
		/* successful authentication */
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}
#endif
}

#endif
