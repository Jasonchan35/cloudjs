#include "cjs_mod_database.h"
#include "cjsClass_DBConn.h"
#include "cjsClass_DBStmt.h"

static axStatus create_DBConn( axDBConn* &out, JSObject* &out_js_obj, cjsContext *cjs ) {
	axStatus st;
	cjsClass_DBConn* p;
	st = cjs->newObject( p );	if( !st ) return st;
	out = &p->db;
	out_js_obj = p->js_obj();
	return 0;
}

//=== Module ====
class Module : public cjsModule {
public:
	virtual	axStatus	onCreate( cjsContext* cjs ) {
		axStatus st;
		st = cjsClass_DBConn::initPrototype( cjs );		if( !st ) return st;
		st = cjsClass_DBStmt::initPrototype( cjs );		if( !st ) return st;
		return 0;
	}
};

extern "C" {
	axEXPORT cjsModule*	cjs_new_module() { return new Module; }
	axEXPORT void	cjs_mod_database_get_api( cjs_mod_database_api &f ) { 
		f.create_DBConn = &create_DBConn; 
	}
}
