#include <libcjs/libcjs.h>
#include <ax/axMySQL.h>

#include "../../cjs_mod_database/src/cjs_mod_database.h"

cjs_mod_database_api mod_database;

CJS_CLASS_START( MySQL )
//	enum {
//		prop_color,
//	};	
	CJS_PROP_SPEC_START
//		CJS_PROP_SPEC( color, prop_color, JSPROP_ENUMERATE )
	CJS_PROP_SPEC_END

	CJS_FUNC_SPEC_START
		CJS_FUNC_SPEC( connect,  1, JSFUN_GENERIC_NATIVE )
	CJS_FUNC_SPEC_END

	CJS_FUNC( connect ) {
		CJS_FIXED_ARGV( 0, axTempStringA, dbname );
		CJS_FIXED_ARGV( 1, axTempStringA, user );
		CJS_FIXED_ARGV( 2, axTempStringA, password );
		CJS_FIXED_ARGV( 3, axTempStringA, host );

		uint32_t port = 0;
		int _port;
		if( CJS_GET_ARGV(4,_port) ) {
			port = _port;
		}

		axStatus st;		
		axDBConn* p;
		JSObject* o;		
		st = mod_database.create_DBConn( p, o, cjs );					if( !st ) return st;
		st = axMySQL_connect( *p, dbname, user, password, host, port );		if( !st ) return st;
		CJS_RETURN( o );
	}
CJS_CLASS_END


//=== Module ====
class Module : public cjsModule {
public:
	virtual	axStatus	onCreate( cjsContext* cjs ) {
		axStatus st;
		st = cjs_mod_database_require( cjs, mod_database );		if( !st ) return st;
		st = cjsClass_MySQL::initPrototype( cjs );				if( !st ) return st;
		return 0;
	}
};

extern "C" {
	axEXPORT cjsModule*	cjs_new_module() { return new Module; }
}
