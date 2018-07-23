/*
 * pam_http: authenticate local users against an HTTP server
 *
 * See README for more documentation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PAM_SM_PASSWORD
#include <security/pam_modules.h>

/* we use libcURL */
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/* nop write function */
static size_t nop_wf(void* a, size_t x, size_t y, void* b) { return x * y; }

/* nop password ask function */
static int my_getpass(void *client, char *prompt, char *buffer, int buflen) {
  buffer[0] = '\0';
  return 0;
}

static int geturl(const char *url, const char *username, 
		  const char *password, const char *cafile)
{
  CURL *curl = curl_easy_init();
  CURLcode res = -1;
  char *userpass;
  int len = strlen(username) + strlen(password) + 2;

  if (!curl) return 0;
  userpass = malloc(len);
  if (!userpass) goto cleanup;

  sprintf(userpass, "%s:%s", username, password);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  /* discard read data */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nop_wf);
  curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);
  /* provide no progress indicator */
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  /* fail on HTTP errors */
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  /* return an empty password if none supplied instead of the default
     behavior, which is to fuck with the tty and half-assedly ask the
     user for a password from somewhere in the guts of libcURL */
  curl_easy_setopt(curl, CURLOPT_PASSWDFUNCTION, my_getpass);

  /* seed SSL randomness from somewhere; this is really problematic
     because libcurl wants to read 16 kilobytes of randomness.  (Why
     does it think it needs 131072 bits?  Does it think someone might
     spend 10^39334 universe-lifetimes to brute-force our SSL
     connection?) */
  curl_easy_setopt(curl, CURLOPT_RANDOM_FILE, "/dev/urandom");
  /* verify SSL peer's certificate */
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
  /* and also verify that its name matches the name we're calling it by */
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
  /* use this file for SSL cert verification */
  curl_easy_setopt(curl, CURLOPT_CAINFO, cafile);

  res = curl_easy_perform(curl);

  memset(userpass, '\0', len);
  free(userpass);
 cleanup:
  curl_easy_cleanup(curl);

  return res == 0;
}

/* pam arguments are normally of the form name=value.  This gets the
 * 'value' corresponding to the passed 'name' from the argument
 * list. */
static const char *getarg(const char *name, int argc, const char **argv) {
  int len = strlen(name);
  while (argc) {
    if (strlen(*argv) > len && 
	!strncmp(name, *argv, len) && 
	(*argv)[len] == '=') {
      return *argv + len + 1;  /* 1 for the = */
    }
    argc--;
    argv++;
  }
  return 0;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, 
				   int argc, const char **argv)
{
  struct pam_conv *item;
  struct pam_message msg;
  const struct pam_message *msgp;
  struct pam_response *respp;
  const char *username;
  const char *cafile = getarg("cafile", argc, argv);  /* possibly NULL */
  const char *url = getarg("url", argc, argv);
  int rv = PAM_SUCCESS;

  if (!url) return PAM_AUTH_ERR;

  msgp = &msg;

  msg.msg_style = PAM_PROMPT_ECHO_OFF;
  msg.msg = "Tell me a secret: ";

  if (pam_get_item(pamh, PAM_CONV, (const void**)&item) != PAM_SUCCESS) {
    fprintf(stderr, "Couldn't get pam_conv\n");
    return PAM_AUTH_ERR;
  }

  if (pam_get_user(pamh, &username, 0) != PAM_SUCCESS) {
    fprintf(stderr, "Couldn't get username\n");
    return PAM_AUTH_ERR;
  }

  item->conv(1, &msgp, &respp, item->appdata_ptr);

  if (!geturl(url, (char*)username, respp[0].resp, cafile))
      rv = PAM_AUTH_ERR;

  memset(respp[0].resp, '\0', strlen(respp[0].resp));
  free(respp);

  return rv;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, 
			      int argc, const char **argv)
{
  return PAM_SUCCESS;
}

#ifdef PAM_STATIC  /* untested */

struct pam_module _pam_http_modstruct = {
  "pam_http",
  pam_sm_authenticate,
  pam_sm_setcred,
  0, 0, 0, 0
};

#endif
