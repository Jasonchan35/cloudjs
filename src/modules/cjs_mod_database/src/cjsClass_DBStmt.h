#ifndef __cjsClass_DBStmt_h__
#define __cjsClass_DBStmt_h__

#include <libcjs/libcjs.h>

CJS_CLASS_START( DBStmt )

	CJS_PROP_SPEC_START
	CJS_PROP_SPEC_END

	CJS_FUNC_SPEC_START
		CJS_FUNC_SPEC( getRow,	0, JSFUN_GENERIC_NATIVE )
	CJS_FUNC_SPEC_END

	CJS_FUNC( getRow ) {
		axStatus st;
		cjsObject	o;
		st = cjs->newJSObject(o);	if( !st ) return st;
		
		st = stmt.fetch();
		if( st.code() == axStatus_Std::DB_no_more_row ) {
			CJS_RETURN( JS_FALSE );
		}
		
		if( !st ) return st;

		axTempStringA	tmpStr;
		const char*	colName;
		axSize n = stmt.colCount();
		for( axSize i=0; i<n; i++ ) {
			colName = stmt.getColumnName( i );	if( !colName ) return st;
			int t   = stmt.getValueType(i);
			
			switch( t ) {
				#define axTYPE_LIST( C, T )	\
					case C: { \
						T	v; \
						st = stmt.getValue( i, v );				if( !st ) return st; \
						st = o.setProperty( colName, v );		if( !st ) return st;\
					}break;
				//------
				axTYPE_LIST( axDB_c_type_int8,   int8_t  )
				axTYPE_LIST( axDB_c_type_int16,  int16_t )
				axTYPE_LIST( axDB_c_type_int32,  int32_t )
				axTYPE_LIST( axDB_c_type_int64,  int64_t )
				axTYPE_LIST( axDB_c_type_float,  float   )
				axTYPE_LIST( axDB_c_type_double, double  )
				#undef axTYPE_LIST

				case axDB_c_type_StringA: {
					st = stmt.getValue( i,tmpStr );				if( !st ) return st;
					st = o.setProperty( colName, tmpStr );		if( !st ) return st;
				}break;
			}
		}
		
		CJS_RETURN(o);
	}
	axDBStmt	stmt;
CJS_CLASS_END

#endif //__cjsClass_DBStmt_h__
