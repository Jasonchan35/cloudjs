#ifndef __cjsClass_cjs_h__
#define __cjsClass_cjs_h__

#include <libcjs/cjs/cjsClass.h>

class	IncludeOnceFile : public axHashTableNode< IncludeOnceFile > {
public:
	axStringA	filename;
	uint32_t	hashTableValue() { return ax_string_hash(filename); }
};
axHashTable< IncludeOnceFile >	includeOnceFiles;

CJS_CLASS_START( cjs )
//	enum {
//		prop_session,
//	};	

	CJS_PROP_SPEC_START
//		CJS_PROP_SPEC( session, prop_session, JSPROP_ENUMERATE )
	CJS_PROP_SPEC_END

	CJS_FUNC_SPEC_START
		CJS_FUNC_SPEC( print,		1, JSFUN_GENERIC_NATIVE )
		CJS_FUNC_SPEC( module,		1, JSFUN_GENERIC_NATIVE )
		CJS_FUNC_SPEC( include,		1, JSFUN_GENERIC_NATIVE )
		CJS_FUNC_SPEC( includeOnce, 1, JSFUN_GENERIC_NATIVE )
	CJS_FUNC_SPEC_END

	CJS_FUNC( module ) {
		CJS_FIXED_ARGV( 0, axTempStringA, name );
		cjsModule *mod;
		cjs->loadModule( mod, name );
		return 0;
	}

	CJS_FUNC( print ) {
		CJS_FIXED_ARGV( 0, axTempStringA, str );
		cjs->onPrint( str );
		return 0;
	}

	CJS_FUNC( include ) {
		CJS_FIXED_ARGV( 0, axTempStringA, filename );
		axStatus	st;

		axTempStringA	out_filename;
		if( axFilePath::isAbsolute( filename ) ) {
			st = out_filename.set( filename );	if( !st ) return st;
		}else{
			axTempStringA	dir;
			st = axFilePath::getDirName( dir, cjs->currentScriptFilename() );	if( !st ) return st;
			st = out_filename.format( "{?}{?}", dir, filename );	if( !st ) return st;
		}

		jsval js_ret;
		st = cjs->evalFile( js_ret, out_filename );			if( !st ) return st;
		return 0;
	}

	CJS_FUNC( includeOnce ) {
		CJS_FIXED_ARGV( 0, axTempStringA, filename );
		axStatus	st;

		IncludeOnceFile*	p;
		p = includeOnceFiles.getListHead( ax_string_hash(filename) );
		for( ; p; p=p->next() ) {
			if( p->filename.equals( filename ) ) {
				//ax_log("already included file {?}", filename );
				return 0;
			}
		}

		axAutoPtr< IncludeOnceFile >	a;
		st = a.newObject();					if( !st ) return st;
		st = a->filename.set( filename );	if( !st ) return st;
		includeOnceFiles.append( a.unref() );

		//---
		axTempStringA	out_filename;
		if( axFilePath::isAbsolute( filename ) ) {
			st = out_filename.set( filename );	if( !st ) return st;
		}else{
			axTempStringA	dir;
			st = axFilePath::getDirName( dir, cjs->currentScriptFilename() );	if( !st ) return st;
			st = out_filename.format( "{?}{?}", dir, filename );	if( !st ) return st;
		}

		jsval js_ret;
		st = cjs->evalFile( js_ret, out_filename );			if( !st ) return st;
		return 0;
	}

	cjsClass_cjs() { //constructor
		//includeOnceFiles.setTableSize( 32 );
	}

CJS_CLASS_END

#endif //__cjsClass_cjs_h__
