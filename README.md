# `pam_http`: PAM module for HTTP authentication

Contents: Quick start, Copyright information, Who cares?, Bugs

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
  - there are lots more ways to get usernames and passwords for Apache
    than there are for Unix; see below.

You just configure `pam_http` with some URL; whenever it tries to
  authenticate someone's password, it fetches that URL with their username
  and the supplied password; if it got 200, it authenticates the user,
  but if it gets any other result code, it fails.  You use it more or less
  as follows; note that option parsing isn't yet implemented:

```
auth required pam_http.so url=https://ourserver.example.com/ cafile=/usr/lib/ca-certificates.pem
```

cafile is the PEM file in which to find certifying-authority
certificates.

At this writing, Apache supports at least the following 26 methods of
user authentication:

`mod_auth` --- text files on a remote server
	<http://httpd.apache.org/docs/mod/mod_auth.html>
`mod_auth_anon` --- anonymous-ftp-style anonymous access
	<http://httpd.apache.org/docs/mod/mod_auth_anon.html>
`mod_auth_db` --- authenticate users with passwords stored in Berkeley DB
	files <http://httpd.apache.org/docs/mod/mod_auth_db.html>
Apache::AuthenDBI --- authenticate users with passwords stored in any
        database supported by Perl DBI (currently Adabas, Adaptive Server,
        Altera, CSV files, DB2, Empress, Microsoft SQL Server, Sybase,
        SearchServer, Illustra, Informix, Ingres, anything you have an
        ODBC driver for, Oracle, QBase, SOLID, SQLFLEX, Sybase, Unify,
        XBase, msql, MySQL, InterBase, dtF/SQL, anything you have ADO for,
        Excel, PostgreSQL, LDAP; see <http://dbi.symbolstone.org/>
        for current list) (succeeds mod_auth_dbi)
        <http://www.perldoc.com/cpan/Apache/AuthenDBI.html>
`mod_auth_dbm` --- authenticate users with passwords stored in DBM files
	<http://httpd.apache.org/docs/mod/mod_auth_dbm.html>
`mod_auth_external` --- run an external program, CGI-style, to authenticate
	users; comes with external programs that authenticate with PAM,
	RADIUS, and Sybase
	<http://www.wwnet.net/~janc/mod_auth_external.html>
`mod_auth_kerb` --- authenticate users with Kerberos
`mod_auth_ldap` --- authenticate users with LDAP
       <http://www.muquit.com/muquit/software/mod_auth_ldap/mod_auth_ldap.html>
`mod_auth_mysql` --- authenticate users with passwords stored in a MySQL
	database <http://sourceforge.net/projects/mod-auth-mysql/>
`mod_auth_nds` --- authenticate users with passwords from Novell NDS,
	using Linux ncpfs <http://users.drew.edu/~pwilson/mod_auth_nds/>
`mod_auth_samba` --- authenticate users with passwords on SMB servers
	<http://sourceforge.net/projects/modauthsamba/>
`mod_auth_sys` --- authenticate users with getpwnam, `pam_unix`-style
	(apocryphal)
`mod_auth_nis` --- presumably, authenticate users with NIS (apocryphal)
`mod_auth_dce` --- authenticate users with DCE security registry
	<http://www.csupomona.edu/~henson/www/projects/>
`mod_auth_notes` --- presumably, authenticate users with Lotus Notes
	(apocryphal)
`mod_auth_pg` --- authenticate users with passwords stored in PostgreSQL
	<http://authpg.sourceforge.net/>
`mod_auth_radius` --- authenticate users with RADIUS
	<http://www.freeradius.org/mod_auth_radius/>
`mod_auth_rdbm` --- authenticate users with DBM files on another machine
	accessed with RPC
	<http://www.webthing.com/software/AnyDBM/apache.html>
`mod_auth_yard` --- ??? apocryphal
`mod_ldap` --- child of `mod_auth_ldap`; authenticate users with LDAP
`mod_auth_tds` --- authenticate users against TDS-compliant databases like
	MS SQL Server or Sybase
	<http://ian.cwru.edu/projects/mod_auth_tds.html>
`mod_auth_unixODBC` --- authenticate users against any database accessible
	via unixODBC <http://home-4.tiscali.nl/~t794104/mod_auth_unixodbc/>
`mod_ntlm` --- authenticate users with native Windows calls on Apache 1.3
`mod_ntlm` --- authenticate users with the NT LAN Manager protocol on
	Unix <http://modntlm.soureforge.net>
`mod_auth_pam` --- authenticate users with PAM; using this turns HTTP
	into merely an RPC protocol for PAM, which is handy if you have
	PAM modules available on one machine but not on others (due to
	different OSes, different hardware, different physical locations,
	etc.) <http://pam.sourceforge.net/mod_auth_pam/>
`mod_auth_sspi` --- authenticate users with native Windows calls with Apache 2.0;
	successor to the Windows mod_ntlm
	<http://members.ozemail.com.au/~timcostello/mod_auth_sspi/>

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
