# `pam_http`: PAM module for HTTP authentication

[![Build Status](https://travis-ci.org/DriftNoise/pam-http.svg?branch=master)](https://travis-ci.org/DriftNoise/pam-http)

## Quick start

Make sure development files for PAM and for cURL are installed e.g. on Debian
based systems:

```
sudo apt install libpam0g-dev libcurl-dev
```

To build: type `make`.

To test: copy `pam.d/check_user` into `/etc/pam.d/check_user`.  Change the
path inside it to refer to where you just built `pam_http.so`.  Run
`check_user username`.  It will authenticate you against the URL
specified in `/etc/pam.d/check_user` --- by default,
http://localhost/~kragen/sekrit/.

## Copyright information

Copyright Kragen Sitaker, 2001.  All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## Who cares?

This lets you check passwords when users log in against an HTTP server
such as Apache.  This is handy for three reasons:

  - in any medium-sized workgroup these days, you probably have to set
    up an HTTP server with password authentication anyway; maybe you
    already have
  - HTTP, especially over SSL, is a very featureful protocol; it supports
    transparent load-balancing, authenticating against servers on the other
    side of the world, easily managing authentication in several domains
    against the same server, secure transport encryption, and server
    authentication (so you know you aren't sending your users' putative
    passwords to a spoofing server)
  - there are many more ways to get usernames and passwords for Apache
    than there are for Unix.

You just configure `pam_http` with some URL; whenever it tries to
authenticate someone's password, it fetches that URL with their username and
the supplied password; if it got 200, it authenticates the user, but if it
gets any other result code, it fails.  You use it more or less as follows;
note that option parsing isn't yet implemented:

```
auth required pam_http.so url=https://ourserver.example.com/ cafile=/usr/lib/ca-certificates.pem
```

where `cafile` is the PEM file in which to find certifying-authority
certificates.

## Bugs

- HTTP is easy to subvert for now with the `http_proxy` environment
  variable; there isn't really a good solution for this except to
  use HTTPS
- doesn't have a list of SSL CA certificates to consult
- doesn't follow redirects, although it could and probably should
- most HTTP requests result in success even without authorization;
  this means that this module is likely to fail open if misconfigured.
  One way to solve this would be to require a "failopen=1" parameter
  to allow the use of URLs that succeed even without authentication.
- I haven't audited libcURL, so I don't know if it does anything
  IDIOTIC like leave usernames and passwords hanging around (this
  code did for a while, so I wouldn't be surprised if libcURL did too)
- this doesn't provide the most helpful error messages
- libcURL insists on having a `FILE*` to write error messages to and uses
  stderr by default!  I don't think it actually does write error messages
  in this program, but it could.
- it isn't tested against SSL servers; in particular, the
  security-critical anti-spoofing functionality of SSL is untested.
