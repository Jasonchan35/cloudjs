#pragma once
#ifndef __cjsModule_h__
#define __cjsModule_h__

#include "cjs_define.h"

class cjsModule : public axNonCopyable {
public:
	virtual ~cjsModule() {}

	axStatus	create(  cjsContext* cx  );
	typedef		cjsModule* (*NewMod)();

	axSharedLibrary*	sharedLib;

protected:
	virtual	axStatus	onCreate( cjsContext* cx ) = 0;
};

#endif //__cjsModule_h__
