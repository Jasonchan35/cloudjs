#ifndef __cjs_Context_h__
#define __cjs_Context_h__

#include "cjs_define.h"
#include "cjsModule.h"
#include "cjsConfig.h"
#include "cjsStatus.h"

class cjsObject;

class cjsContext : public axNonCopyable {
public:
	cjsContext();
	virtual	~cjsContext();

			axStatus	create		( const cjsConfig & static_config );
			void		destroy		();

			axStatus	loadModule	( cjsModule* &out, const char* name );

	virtual	void		onPrint		( const char* msg  );
	virtual void		onError		( const char* filename, unsigned line_no, const char* message );

			axStatus	eval		( jsval& js_ret, const char* script,		const char* filename = NULL );
			axStatus	eval		( jsval& js_ret, const axIByteArray& buf,	const char* filename = NULL );
			axStatus	evalFile	( jsval& js_ret, const char* filename );

			void		abort		();

			void		_heartBeat		();	//!< for time limit check
			void		resetRunningTime();

			//-- to jsval
			axStatus	convert		( jsval & dst,	jsval    src );
			axStatus	convert		( jsval & dst,	int8_t   src );
			axStatus	convert		( jsval & dst,	int16_t  src );
			axStatus	convert		( jsval & dst,	int32_t  src );
			axStatus	convert		( jsval & dst,	int64_t  src );
			axStatus	convert		( jsval & dst,	float    src );
			axStatus	convert		( jsval & dst,	double   src );
			axStatus	convert		( jsval & dst,	bool     src );
			axStatus	convert		( jsval & dst,	const char * src );
			axStatus	convert		( jsval & dst,	JSObject* src );
			axStatus	convert		( jsval & dst,	const cjsObject & src );

			//-- from jsval
			axStatus	convert		( axIStringA & dst,	jsval src );
			axStatus	convert		( int8_t     & dst,	jsval src );
			axStatus	convert		( int16_t    & dst,	jsval src );
			axStatus	convert		( int32_t    & dst,	jsval src );
			axStatus	convert		( int64_t    & dst,	jsval src );
			axStatus	convert		( float      & dst,	jsval src );
			axStatus	convert		( double     & dst,	jsval src );
			axStatus	convert		( bool       & dst,	jsval src );

			//-- from jsid
			axStatus	convertJSId	( jsval & dst, jsid src );
	template<class T>
			axStatus	convertJSId	( T     & dst, jsid src );


	template<class T>	axStatus	_convert_int( T & dst, jsval src );

						JSType		typeOf		( jsval & v );
			
						axStatus	newJSObject	( cjsObject & out, JSObject* parent = NULL );
	template<class T>	axStatus	newObject	( T* &out ) { return T::newObject( out, this ); }

		const char*		currentScriptFilename () const		{ return currentScriptFilename_; }

			JSContext*	js_cx		() { return js_cx_; }
			JSObject*	js_global	() { return js_global_; }
			JSObject*	js_cjs		() { return js_cjs_; }

			axStatus 	reportError_ArgList( const char* fmt, const axStringFormat::ArgList &list );			
			axExpandArgList1( axStatus, reportError, const char*, const axStringFormat_Arg &, axStringFormat_ArgList, reportError_ArgList );

	static	cjsContext*	getInstance	( JSContext* cx ) { return (cjsContext*) JS_GetContextPrivate( cx ); }

			cjsModule*	findLoadedModule ( const char* name );

	const	cjsConfig&	config() { return *config_; }

	template<class T> axStatus getFuncArg( uintN argc, jsval* argv, uintN index, T & value );

private:
	const	cjsConfig*	config_;

	static	JSBool		operationCallback_( JSContext *cx );

	class LoadedModule : public axHashTableNode< LoadedModule > {
	public:
		axSharedLibrary			sharedLib;
		axAutoPtr<cjsModule>	mod;
		axStringA				name;

		uint32_t	hashTableValue() { return ax_string_hash(name); }
	};
	axHashTable< LoadedModule >	loaded_modules_;

    JSRuntime*		js_rt_;
    JSContext*		js_cx_;
    JSObject*		js_global_;
	JSObject*		js_cjs_;

	const char*		currentScriptFilename_;
	int				openedFileCount_;

	axStopWatch		running_time_;
	axAtomicInt		abort_;
	axStringA		module_dir_;
};

template<class T> inline
axStatus	cjsContext::getFuncArg( uintN argc, jsval* argv, uintN index, T & value ) {
	axStatus st;
	if( index >= argc ) { 
		return cjsStatus::invalid_func_arg_index; 
	}
	st = convert( value, JS_ARGV( js_cx(), argv )[index] );
	if( !st ) {
		ax_log("cannot convert arg({?})", index ); 
		return cjsStatus::invalid_func_arg_index; 
	}
	return 0;
}

inline
JSType		cjsContext::typeOf( jsval & v ) {
	return JS_TypeOfValue( js_cx_, v);
}


inline	axStatus	cjsContext::convertJSId	( jsval &dst, jsid src ) { if( ! JS_IdToValue( js_cx_, src, &dst ) ) return -1; return 0; }

template<class T> inline
axStatus  cjsContext::convertJSId ( T & dst, jsid src ) {
	axStatus st;
	jsval jv;
	st = convertJSId( jv, src );	if( !st ) return st;
	return convert( dst, jv );
}



#endif //__cjs_Context_h__
