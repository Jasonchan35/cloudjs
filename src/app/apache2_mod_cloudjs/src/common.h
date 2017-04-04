#ifndef __apache2_mod_cloudjs_common_h__
#define __apache2_mod_cloudjs_common_h__

#include <libcjs/libcjs.h>


#ifdef _MSC_VER
	#pragma comment( lib, "libapr-1.lib" )
	#pragma comment( lib, "libaprutil-1.lib" )
	#pragma comment( lib, "libhttpd.lib" )
#endif

#define SHARED_MODULE
//apr-1
#include <apr_lib.h>
#include <apr_buckets.h>
#include <apr_hooks.h>
#include <apr_uri.h>
#include <apr_poll.h>
#include <apr_thread_proc.h>
#include <apr_optional.h>

//httpd
#include <httpd.h>
#include <http_config.h>
#include <http_request.h>
#include <http_protocol.h>
#include <http_log.h>
#include <util_script.h>

#endif //__apache2_mod_cloudjs_common_h__
