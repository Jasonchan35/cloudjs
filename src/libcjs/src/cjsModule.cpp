#include <libcjs/cjs/cjsModule.h>

axStatus	cjsModule::create( cjsContext* cx ) {
	axStatus	st;
	st = onCreate(cx);
	return 0;
}

