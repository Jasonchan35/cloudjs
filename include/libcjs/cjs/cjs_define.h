#ifndef __cjs_define_h__
#define __cjs_define_h__

#include <ax/ax_core.h>

#if axOS_WIN
	#define	XP_WIN
#endif

//#define STATIC_JS_API
#include <libcjs/external/js/jsapi.h>

class cjsContext;
class cjsModule;
class cjsClass;



#endif //__cjs_define_h__
