#include <libcjs/cjs/cjsClass.h> 

class	cjs_mod_database_api {
public:
	axStatus (*create_DBConn)( axDBConn* &out, JSObject* &out_js_obj, cjsContext *cjs );
};


inline axStatus cjs_mod_database_require( cjsContext* cjs, cjs_mod_database_api &api ) {
	axStatus st;
	cjsModule* mod;
	st = cjs->loadModule( mod, "database" );				if( !st ) return st;

	void (*get_api)( cjs_mod_database_api &f );

	st = mod->sharedLib->getProc( get_api, "cjs_mod_database_get_api" );	if( !st ) return st;
	get_api( api );
	return 0;
}

