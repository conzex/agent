/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Build with dbus support? */
/* #undef DBUS */

/* Build with systemd inhibit plugin? */
#define ENABLE_INHIBIT_PLUGIN 1

/* Enable new rpm database format? */
#define ENABLE_NDB 1

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* Enable multithreading support? */
/* #undef ENABLE_OPENMP */

/* Build with plugin support? */
/* #undef ENABLE_PLUGINS */

/* Get the group name having groupid 0 */
#define GID_0_GROUP "root"

/* Define to 1 if you have the `basename' function. */
#define HAVE_BASENAME 1

/* Define to 1 if you have the <beecrypt/api.h> header file. */
/* #undef HAVE_BEECRYPT_API_H */

/* Define to 1 if OpenSSL has BN_bn2binpad */
#define HAVE_BN2BINPAD 1

/* Define to 1 if you have the <bzlib.h> header file. */
/* #undef HAVE_BZLIB_H */

/* Have cap_compare function? */
/* #undef HAVE_CAP_COMPARE */

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define to 1 if you have the <db.h> header file. */
/* #undef HAVE_DB_H */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#define HAVE_DCGETTEXT 1

/* Define to 1 if you have the declaration of `fdatasync', and to 0 if you
   don't. */
#define HAVE_DECL_FDATASYNC 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the `dirname' function. */
#define HAVE_DIRNAME 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if OpenSSL has DSA_set0_key */
#define HAVE_DSA_SET0_KEY 1

/* Define to 1 if OpenSSL has DSA_set0_pqg */
#define HAVE_DSA_SET0_PQG 1

/* Define to 1 if OpenSSL has DSA_SIG_set0 */
#define HAVE_DSA_SIG_SET0 1

/* Define to 1 if you have the <dwarf.h> header file. */
/* #undef HAVE_DWARF_H */

/* Have dwelf_elf_begin? */
/* #undef HAVE_DWELF_ELF_BEGIN */

/* Define to 1 if you have the <elfutils/libdwelf.h> header file. */
/* #undef HAVE_ELFUTILS_LIBDWELF_H */

/* Define to 1 if OpenSSL has EVP_MD_CTX_new */
#define HAVE_EVP_MD_CTX_NEW 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the <gelf.h> header file. */
/* #undef HAVE_GELF_H */

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getauxval' function. */
/* #undef HAVE_GETAUXVAL */

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `getline' function. */
#define HAVE_GETLINE 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define as 1 if your zlib has gzseek() */
#define HAVE_GZSEEK 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `lchown' function. */
#define HAVE_LCHOWN 1

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have elfutils libdw library */
/* #undef HAVE_LIBDW */

/* Define to 1 if you have the 'elf' library (-lelf). */
/* #undef HAVE_LIBELF */

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `pthread' library (-lpthread). */
#define HAVE_LIBPTHREAD 1

/* Define to 1 if you have the `selinux' library (-lselinux). */
/* #undef HAVE_LIBSELINUX */

/* Define to 1 if you have the `thread' library (-lthread). */
/* #undef HAVE_LIBTHREAD */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `lsetxattr' function. */
/* #undef HAVE_LSETXATTR */

/* Define to 1 if you have the `lutimes' function. */
#define HAVE_LUTIMES 1

/* Define to 1 if you have the <lzma.h> header file. */
/* #undef HAVE_LZMA_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mempcpy' function. */
#define HAVE_MEMPCPY 1

/* Define to 1 if you have the `mergesort' function. */
/* #undef HAVE_MERGESORT */

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the `mremap' function. */
/* #undef HAVE_MREMAP */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <nspr.h> header file. */
/* #undef HAVE_NSPR_H */

/* Define to 1 if you have the <nss.h> header file. */
/* #undef HAVE_NSS_H */

/* Define to 1 if NSS has NSS_InitContext */
/* #undef HAVE_NSS_INITCONTEXT */

/* Define to 1 if you have the <openssl/dsa.h> header file. */
#define HAVE_OPENSSL_DSA_H 1

/* Define to 1 if you have the <openssl/evp.h> header file. */
#define HAVE_OPENSSL_EVP_H 1

/* Define to 1 if you have the <openssl/rsa.h> header file. */
#define HAVE_OPENSSL_RSA_H 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have the `realpath' function. */
#define HAVE_REALPATH 1

/* Define to 1 if you have the `regcomp' function. */
#define HAVE_REGCOMP 1

/* Define to 1 if OpenSSL has RSA_set0_key */
#define HAVE_RSA_SET0_KEY 1

/* Define to 1 if you have the `sched_getaffinity' function. */
#define HAVE_SCHED_GETAFFINITY 1

/* Define to 1 if you have the <sechash.h> header file. */
/* #undef HAVE_SECHASH_H */

/* Define to 1 if you have the `secure_getenv' function. */
/* #undef HAVE_SECURE_GETENV */

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if SELinux setexecfilecon is present */
/* #undef HAVE_SETEXECFILECON */

/* Define to 1 if you have the `setprogname' function. */
/* #undef HAVE_SETPROGNAME */

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
#define HAVE_STPCPY 1

/* Define to 1 if you have the `stpncpy' function. */
#define HAVE_STPNCPY 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `d_type' is a member of `struct dirent'. */
#define HAVE_STRUCT_DIRENT_D_TYPE 1

/* Define to 1 if you have the `syncfs' function. */
/* #undef HAVE_SYNCFS */

/* Define to 1 if you have the <sys/auxv.h> header file. */
/* #undef HAVE_SYS_AUXV_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/systemcfg.h> header file. */
/* #undef HAVE_SYS_SYSTEMCFG_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if you have the `unshare' function. */
#define HAVE_UNSHARE 1

/* Define to 1 if you have the `utimes' function. */
#define HAVE_UTIMES 1

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define if libzstd is available */
/* #undef #define HAVE_ZSTD */

/* Define if __progname is defined */
#define HAVE___PROGNAME 1

/* Define to 1 if you have the `__secure_getenv' function. */
/* #undef HAVE___SECURE_GETENV */

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if `major', `minor', and `makedev' are declared in <mkdev.h>.
   */
/* #undef MAJOR_IN_MKDEV */

/* Define to 1 if `major', `minor', and `makedev' are declared in
   <sysmacros.h>. */
#define MAJOR_IN_SYSMACROS 1

/* Name of package */
#define PACKAGE "rpm"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "rpm-maint@lists.rpm.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "rpm"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "rpm 4.18.2"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "rpm"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.18.2"

/* canonical vendor */
#define RPMCANONVENDOR "pc"

/* run-time variable directory */
#define RUNDIR "/run"

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Get the user name having userid 0 */
#define UID_0_USER "root"

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable general extensions on macOS.  */
#ifndef _DARWIN_C_SOURCE
# define _DARWIN_C_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable X/Open compliant socket functions that do not require linking
   with -lxnet on HP-UX 11.11.  */
#ifndef _HPUX_ALT_XOPEN_SOCKET_API
# define _HPUX_ALT_XOPEN_SOCKET_API 1
#endif
/* Identify the host operating system as Minix.
   This macro does not affect the system headers' behavior.
   A future release of Autoconf may stop defining this macro.  */
#ifndef _MINIX
/* # undef _MINIX */
#endif
/* Enable general extensions on NetBSD.
   Enable NetBSD compatibility extensions on Minix.  */
#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif
/* Enable OpenBSD compatibility extensions on NetBSD.
   Oddly enough, this does nothing on OpenBSD.  */
#ifndef _OPENBSD_SOURCE
# define _OPENBSD_SOURCE 1
#endif
/* Define to 1 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_SOURCE
/* # undef _POSIX_SOURCE */
#endif
/* Define to 2 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_1_SOURCE
/* # undef _POSIX_1_SOURCE */
#endif
/* Enable POSIX-compatible threading on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-5:2014.  */
#ifndef __STDC_WANT_IEC_60559_ATTRIBS_EXT__
# define __STDC_WANT_IEC_60559_ATTRIBS_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-1:2014.  */
#ifndef __STDC_WANT_IEC_60559_BFP_EXT__
# define __STDC_WANT_IEC_60559_BFP_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-2:2015.  */
#ifndef __STDC_WANT_IEC_60559_DFP_EXT__
# define __STDC_WANT_IEC_60559_DFP_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-4:2015.  */
#ifndef __STDC_WANT_IEC_60559_FUNCS_EXT__
# define __STDC_WANT_IEC_60559_FUNCS_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-3:2015.  */
#ifndef __STDC_WANT_IEC_60559_TYPES_EXT__
# define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TR 24731-2:2010.  */
#ifndef __STDC_WANT_LIB_EXT2__
# define __STDC_WANT_LIB_EXT2__ 1
#endif
/* Enable extensions specified by ISO/IEC 24747:2009.  */
#ifndef __STDC_WANT_MATH_SPEC_FUNCS__
# define __STDC_WANT_MATH_SPEC_FUNCS__ 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable X/Open extensions.  Define to 500 only if necessary
   to make mbstate_t available.  */
#ifndef _XOPEN_SOURCE
/* # undef _XOPEN_SOURCE */
#endif


/* Version number of package */
#define VERSION "4.18.2"

/* Build with acl support? */
/* #undef WITH_ACL */

/* libaudit support */
/* #undef WITH_AUDIT */

/* Define if BDB is available */
/* #undef WITH_BDB */

/* Build with read-only Berkeley DB */
#define WITH_BDB_RO 1

/* Build with beecrypt instead of nss3 support? */
/* #undef WITH_BEECRYPT */

/* Build with capability support? */
/* #undef WITH_CAP */

/* Build with imaevm support? */
/* #undef WITH_IMAEVM */

/* Build with selinux support? */
/* #undef WITH_SELINUX */

/* Define if SQLITE is available */
#define WITH_SQLITE 1

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* A full path to a program, possibly with arguments, that will create a
   directory and all necessary parent directories, ala 'mkdir -p' */
#define __MKDIR_P "/bin/mkdir -p"

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define as a signed integer type capable of holding a process identifier. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
