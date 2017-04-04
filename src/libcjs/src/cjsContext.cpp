#include <libcjs/cjs/cjsContext.h>
#include <libcjs/cjs/cjsObject.h>
#include "cjsClass_cjs.h"

static cjsStatus	cjsStatus_instance;

static 
JSClass _global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
/* === global func =======
#define	CJS_GLOBAL_FUNC( F ) \
	axStatus cjs_##F( cjsContext *cjs, uintN argc, jsval *argv ); \
	static	JSBool	_cjs_wrapper_##F( JSContext *cx, uintN argc, jsval *argv ) { \
		cjsContext* cjs = cjsContext::getInstance(cx); \
		axStatus st = cjs_##F( cjs, argc, argv ); \
		if( !st ) { \
			ax_log("JS GLOBAL FUNC {?} Error {?}", axFUNC_NAME, st ); \
			return JS_FALSE; \
		} \
		return JS_TRUE; \
	} \
	axStatus cjs_##F( cjsContext *cjs, uintN argc, jsval *argv ) 
//--- End CJS_GLOBAL_FUNC

CJS_GLOBAL_FUNC( print ) {
	CJS_FIXED_ARGV( 0, axTempStringA, str );
	cjs->onPrint( str );
	return 0;
}

JSFunctionSpec cjs_global_funcs[] = {
	CJS_FUNC_SPEC( print,   1, JSFUN_GENERIC_NATIVE )
	JS_FS_END
};

*/

cjsContext::cjsContext(){
	js_cx_ = NULL;
	js_rt_ = NULL;
	js_global_ = NULL;
	js_cjs_  = NULL;
	currentScriptFilename_ = NULL;
	config_ = NULL;
	abort_ = 0;
	loaded_modules_.setTableSize( 32 );
}

cjsContext::~cjsContext() {
	destroy();
}

cjsModule*	cjsContext::findLoadedModule ( const char* name ) {
	uint32_t	h = ax_string_hash( name );

	LoadedModule* p = loaded_modules_.getListHead( h );
	for( ; p; p=p->next() ) {
		if( p->name.equals(name) ) return p->mod;
	}
	return NULL;
}

axStatus cjsContext::loadModule( cjsModule* &out, const char* name ) {

	cjsModule* loaded = findLoadedModule( name );
	if( loaded ) {
//		ax_log( "module {?} already loaded", name );
		out = loaded;
		return 0;
	}

    axStatus	st;
    axAutoPtr< LoadedModule >	m;
    st = m.newObject();		if( !st ) return st;

	if( ax_strstr( name, ".." ) ) {
		//avoid load file out of module_dir_
		ax_log( "module name {?} contain '..' is not allowed", name );
		return cjsStatus::error;
	}

	st = m->name.set( name );		if( !st ) return st;

	axTempStringA	filename;
	if( config().module_dir.isEmpty() ) {
		ax_log( "module_dir cannot be empty" );
		return cjsStatus::error;
	}else{
		st = filename.format( "{?}/cjs_mod_{?}{?}", config().module_dir, name, axSO_SUFFIX );
	}
	if( !st ) return st;
		
    st = m->sharedLib.load( filename );
	if( !st ) {
		ax_log( "unable to load module file {?}", filename );
		return st;
	}

	cjsModule::NewMod	new_mod;
	st = m->sharedLib.getProc( new_mod,    "cjs_new_module" );	if( !st ) return st;

    m->mod.ref( new_mod() );
	if( ! m->mod ) return axStatus_Std::not_enough_memory;
	st = m->mod->create(this);		if( !st ) return st;
	m->mod->sharedLib = &m->sharedLib;

	out = m->mod;
	loaded_modules_.append( m.unref() );
	ax_log("load module {?} success", name );
    return 0;
}

//virtual	
void cjsContext::onPrint( const char* msg ) {
	size_t n = ax_strlen( msg );
	fwrite( msg, n, 1, stdout );
}

void cjsContext::onError	( const char* filename, unsigned line_no, const char* message ) {
	ax_print("File: {?}\nLine: {?}\n{?}\n\n", filename, line_no, message );
}

axStatus	cjsContext::eval( jsval& js_ret,const char* script, const char* filename ) {
	size_t	script_len = ax_strlen( script );
	axExternalByteArray	buf( (uint8_t*)script, script_len );
	return eval( js_ret, buf, filename );
}

axStatus	cjsContext::eval( jsval& js_ret,const axIByteArray& buf, const char* filename ) {
//	ax_print("cjsContext::eval filename={?}\n", filename );
	if( buf.size() <= 0 ) return -1;
	axStatus	st;

	unsigned buf_len;
	st = ax_safe_assign( buf_len, buf.size() );		if( !st ) return st;

	const char* old_filename = currentScriptFilename_;
	currentScriptFilename_ = filename;

		const char* script = (const char*)buf.ptr();
		if( buf_len >= 3 ) {
			//skip UTF-8 BOM
			if( script[0] == '\xEF' && script[1] == '\xBB' && script[2] == '\xBF' ) {
				script  += 3;
				buf_len -= 3;
			}
		}

		JSBool ok = JS_EvaluateScript( js_cx_, js_global_, script, buf_len, filename, 1, &js_ret );
	currentScriptFilename_ = old_filename;

	if( !ok ) {
		if( JS_IsExceptionPending(js_cx_) ) {
			JS_ReportPendingException(js_cx_);
			JS_ClearPendingException(js_cx_);
		}
		return -1;
	}

	return 0;
}

axStatus	cjsContext::evalFile( jsval& js_ret, const char* filename ) {
	axStatus	st;
#if 1 //using Memory Mapping
	axMemMap	buf;
	st = buf.openRead( filename );		
#else
	axByteArray_<1024>	buf;
	st = axFile::loadFile( filename, buf );		
#endif
	if( !st ) {
		switch( st.code() ) {
			case axStatus_Std::File_not_found:		reportError("File Not Found [{?}]",     filename );	break;
			case axStatus_Std::File_access_denied:	reportError("File Access Denied [{?}]", filename );	break;
		}
		return st;
	}
	if( buf.size() == 0 ) return 0;
	return eval( js_ret, buf, filename );
}

static	
void cjsContext_errorReportCallback(JSContext *cx, const char *message, JSErrorReport *report) {
	cjsContext* cjs = cjsContext::getInstance( cx );
	cjs->onError( report->filename, report->lineno, message );
}

JSBool cjsContext::operationCallback_( JSContext *cx ) {
	cjsContext* cjs = cjsContext::getInstance( cx );

	double limit = (double) cjs->config().script_time_limit;
	if( limit <= 0 ) {
		double t = cjs->running_time_.get();
		if( t >= limit ) {
			cjs->reportError( "Time Out\n" );
			return JS_FALSE;
		}
	}
	if( cjs->abort_.cmpset(1,0) ) {
		return JS_FALSE;
	}
	return JS_TRUE;
}

void cjsContext::abort() {
	abort_ = 1;
	JS_TriggerOperationCallback( js_cx() );
}

void cjsContext::_heartBeat() {
	JS_TriggerOperationCallback( js_cx() );
}

void	cjsContext::resetRunningTime() {
	running_time_.reset();
}

axStatus	cjsContext::create ( const cjsConfig & static_config ) {
	axStatus st;
	destroy();

	config_ = & static_config;

	resetRunningTime();

    js_rt_ = JS_NewRuntime(8L * 1024L * 1024L);
    if (js_rt_ == NULL)
		return -1;

    js_cx_ = JS_NewContext(js_rt_, 8192);
    if (js_cx_ == NULL) 
		return -2;

	JS_SetContextPrivate( js_cx_, this );

	uint32_t opt = JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT;

	if( config_->strict_mode ) opt |= JSOPTION_STRICT;

    JS_SetOptions(js_cx_,  opt );
    JS_SetVersion(js_cx_, JSVERSION_LATEST);

    JS_SetErrorReporter		( js_cx_, cjsContext_errorReportCallback );
	JS_SetOperationCallback ( js_cx_, cjsContext::operationCallback_ );
	
    /* Create the global object in a new compartment. */
    js_global_ = JS_NewCompartmentAndGlobalObject(js_cx_, &_global_class, NULL);
    if( js_global_ == NULL ) return -3;

    /* Populate the global object with the standard globals,
       like Object and Array. */
    if( ! JS_InitStandardClasses(js_cx_, js_global_) ) return -3;

//	if( ! JS_DefineFunctions(js_cx_, js_global_, cjs_global_funcs ) )  return -4;

	st = cjsClass_cjs::initPrototype( this );		if( !st ) return st;
	js_cjs_ = cjsClass_cjs::js_proto();

//	jsval js_ret;
//	st = eval( js_ret, "print = cloud.print" );		if( !st ) return st;

	const axArray< axStringA > &scriptFiles = config().load_script;

	jsval js_ret;
	for( axSize i=0; i<scriptFiles.size(); i++ ) {
		const axStringA &scriptFile = scriptFiles[i];
		if( scriptFile.isEmpty() ) continue;

		ax_log("load_script {?}", scriptFile );
		evalFile( js_ret, scriptFile );
	}
	return 0;
}

void	cjsContext::destroy() {
	if( js_cx_ )	{ 
		JS_DestroyContext(js_cx_);	
		js_cx_ = NULL;
		js_cjs_ = NULL;
	}
	if( js_rt_ )	{ 
		JS_DestroyRuntime(js_rt_);	js_rt_ = NULL; 
	    JS_ShutDown();
	}
	loaded_modules_.clear();
	config_ = NULL;
}

axStatus cjsContext::reportError_ArgList( const char* fmt, const axStringFormat::ArgList &list ) {
	axStatus st;
	axStringA_<1024>	tmp;
	st = tmp.format_ArgList( fmt, list );		if( !st ) return st;
	JS_ReportError( js_cx(), tmp );
	return 0;
}

axStatus cjsContext::newJSObject( cjsObject & out, JSObject* parent ) {
	JSObject* j;
	j = JS_ConstructObject( js_cx_, NULL, NULL, parent );
	if( ! j ) return cjsStatus::error_newJSObject;
	out.init( this, j );
	return 0;
}

axStatus	cjsContext::convert( axIStringA & dst, jsval src ) {
	axStatus st;
	
	JSString* jstr = JS_ValueToString( js_cx_, src );
	size_t n = JS_GetStringEncodingLength( js_cx_, jstr );

	st = dst.resize( n );					if( !st ) return st;
	char* p = dst._getInternalBufferPtr();	if( !p  ) return -1;
	JS_EncodeStringToBuffer( jstr, p, n );
	return 0;
}

axStatus	cjsContext::convert( jsval & dst, const char * src ) {
	JSString * o = JS_NewStringCopyZ( js_cx_, src );
	if( !o ) return -1;
	dst = STRING_TO_JSVAL(o);
	return 0;
}


template<class T> inline
axStatus	cjsContext::_convert_int( T & dst, jsval src ) {
	int32_t v;
	if( ! JS_ValueToInt32( js_cx_, src, &v ) ) return -1;
	dst = v;
	return 0;
}

//---
axStatus	cjsContext::convert	( jsval & dst,	jsval     src ) { dst = src; return 0; }
axStatus	cjsContext::convert	( jsval & dst,	int8_t    src ) { dst = INT_TO_JSVAL( src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	int16_t   src ) { dst = INT_TO_JSVAL( src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	int32_t   src ) { dst = INT_TO_JSVAL( src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	int64_t   src ) { dst = DOUBLE_TO_JSVAL ( (double)src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	float     src ) { dst = DOUBLE_TO_JSVAL ( (double)src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	double    src ) { dst = DOUBLE_TO_JSVAL ( src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	bool      src ) { dst = BOOLEAN_TO_JSVAL( src?1:0 ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	JSObject* src ) { dst = OBJECT_TO_JSVAL ( src ); return 0; }
axStatus	cjsContext::convert	( jsval & dst,	const cjsObject & src ) { dst = OBJECT_TO_JSVAL ( src.js_obj() ); return 0; }

//---
axStatus	cjsContext::convert( int8_t  & dst, jsval src ) { return _convert_int(dst,src); }
axStatus	cjsContext::convert( int16_t & dst, jsval src ) { return _convert_int(dst,src); }
axStatus	cjsContext::convert( int32_t & dst, jsval src ) { return _convert_int(dst,src); }
axStatus	cjsContext::convert( int64_t & dst, jsval src ) { return _convert_int(dst,src); }

axStatus	cjsContext::convert( float & dst, jsval src ) {
	double d;
	if( ! JS_ValueToNumber( js_cx_, src, &d ) ) return -1;
	dst = (float)d;
	return 0;
}

axStatus	cjsContext::convert( double & dst, jsval src ) {
	if( ! JS_ValueToNumber( js_cx_, src, &dst ) ) return -1;
	return 0;
}

axStatus	cjsContext::convert( bool & dst, jsval src ) {
	JSBool	b;
	if( ! JS_ValueToBoolean( js_cx_, src, &b ) ) return -1;
	dst = b ? true : false;
	return 0;
}
