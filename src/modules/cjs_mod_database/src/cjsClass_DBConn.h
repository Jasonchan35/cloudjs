#ifndef __cjsClass_DBConn_h__
#define __cjsClass_DBConn_h__

#include "cjsClass_DBStmt.h"

CJS_CLASS_START( DBConn )

	CJS_PROP_SPEC_START
	CJS_PROP_SPEC_END

	CJS_FUNC_SPEC_START
		CJS_FUNC_SPEC( test, 0, JSFUN_GENERIC_NATIVE )
		CJS_FUNC_SPEC( exec, 1, JSFUN_GENERIC_NATIVE )
	CJS_FUNC_SPEC_END

	CJS_FUNC( test ) {
		axPRINT_FUNC_NAME
		return JS_TRUE;
	}
	
	struct Param {
		axStringA		str;
		axByteArray		bytes;
		
		axStatus	onTake( Param & src ) { 
			str.onTake( src.str );
			bytes.onTake( src.bytes );
			return 0;
		}
	};
	
	CJS_FUNC( exec ) {
		CJS_FIXED_ARGV( 0, axTempStringA, sql );
		axStatus st;
		
		unsigned n = 0;
		if( argc > 1 ) n = argc - 1;
				
		axArray< Param >	param;
		st = param.resize( n );		if( !st ) return st;
		
		jsval	v;
		axDB_ParamList	list;
		st = list.reserve( n );		if( !st ) return st;
		for( unsigned i=0; i<n; i++ ) {
			CJS_GET_ARGV( i+1, v );
			
			switch( cjs->typeOf( v ) ) {
				case JSTYPE_NUMBER: {
					if( JSVAL_IS_INT( v ) ) {
						list << JSVAL_TO_INT(v);
					}else if( JSVAL_IS_DOUBLE(v) ) {
						list << JSVAL_TO_DOUBLE(v);
					}else{
						return axStatus_Std::DB_invalid_param_type;						
					}
				}break;
				
				case JSTYPE_BOOLEAN: {
					bool b = JSVAL_TO_BOOLEAN(v) ? true : false; 
					list << b;
				}break;
				
				case JSTYPE_STRING: {
					st = cjs->convert( param[i].str, v );	if( !st ) return st;
					list << param[i].str;
				}break;
				
				default:
					return axStatus_Std::DB_invalid_param_type;
			}			
		}
		
		cjsClass_DBStmt* p;
		st = cjs->newObject( p );						if( !st ) return st;
		st = db.exec_ParamList( p->stmt, sql, list );	if( !st ) return st;
		CJS_RETURN( p->js_obj() );
	}

	axDBConn	db;

CJS_CLASS_END

#endif //__cjsClass_DBConn_h__
