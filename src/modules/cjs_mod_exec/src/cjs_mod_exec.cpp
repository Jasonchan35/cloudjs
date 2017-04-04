#include <libcjs/libcjs.h>

CJS_CLASS_START( mod_exec )
//	enum {
//		prop_color,
//	};	
	CJS_PROP_SPEC_START
//		CJS_PROP_SPEC( color, prop_color, JSPROP_ENUMERATE )
	CJS_PROP_SPEC_END

	CJS_FUNC_SPEC_START
		CJS_FUNC_SPEC( exec,  1, JSFUN_GENERIC_NATIVE )
	CJS_FUNC_SPEC_END

	CJS_FUNC( exec ) {
		CJS_FIXED_ARGV ( 0, axTempStringA, cmd );

		axTempStringA	in_str;
		const char*		in = NULL;
		if( CJS_GET_ARGV ( 1, in_str ) ) {
			in = in_str.c_str();
		}

		axStatus st;

		int cmd_ret;
		axTempStringA	out;
		axTempStringA	err;

		st = ax_exec( cmd, cmd_ret, in, &out, &err );		if( !st ) return st;

		cjsObject js_ret;
		st = cjs->newJSObject( js_ret );	if( !st ) return st;

		st = js_ret.setProperty( "status", cmd_ret );	if( !st ) return st;
		st = js_ret.setProperty( "stdout", out );		if( !st ) return st;
		st = js_ret.setProperty( "stderr", err );		if( !st ) return st;

		CJS_RETURN( js_ret );
	}
CJS_CLASS_END


//=== Module ====
class Module : public cjsModule {
public:
	virtual	axStatus	onCreate( cjsContext* cjs ) {
		axStatus st;
		st = cjsClass_mod_exec::initPrototype( cjs );		if( !st ) return st;
		jsval js_ret;
		st = cjs->eval( js_ret, "cloud.exec = mod_exec.exec" );		if( !st ) return st;
		return 0;
	}
};

extern "C" {
	axEXPORT cjsModule*	cjs_new_module() { return new Module; }
}
