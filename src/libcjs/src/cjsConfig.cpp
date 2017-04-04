#include <libcjs/cjs/cjsConfig.h>

cjsConfig::cjsConfig() {
	session_timeout = 15 * 60;
	script_time_limit = 900;
	strict_mode = true;
}

axStatus cjsConfig::readFile( const char* filename ) {
	axStatus st;

	if( filename == NULL ) {
		#if axOS_WIN
			filename = "c:/cloudjs/cloudjs.ini";
		#else
			filename = "/etc/cloudjs.ini";
		#endif
	}

	st = config_filename.set( filename );	if( !st ) return st;

	ax_log("read config file {?}", filename );

	st = ini.readFile( filename );		
	if( !st ) {
		ax_log("Error load cloudjs config file {?}", filename );
		return st;
	}
		
	ini.getValue( session_timeout,	"cloudjs",	"session_timeout" );
	ini.getValue( module_dir,		"cloudjs",	"module_dir" );
	ini.getValueArray( load_script,	"cloudjs",	"load_script" );
	ini.getValue( strict_mode,		"cloudjs",	"strict_mode" );

	return 0;
}
