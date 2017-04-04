#include <libcjs/libcjs.h>
#include <ax/axSQLite3.h>

class MyApp : public axCommandLineApp {
public:
	
	axStatus onRun() {
		axStatus st;
		st = test();
		ax_log("------ end -- return {?} ----\n" , st);
		
#if axOS_WIN
		_getch();
#endif
		return 0;
	}
	
	axStatus test() {
//		printArgs();
		axStatus st;
		axTempStringA	tmp;
		axApp::getCurrentDir( tmp );
		ax_log("getCurrentDir {?}", tmp );
				
		const char * filename = getOption( NULL );
		ax_log( "filename = {?}", filename );
		
		const char * conf_file = getOption( "conf" );
		ax_log( "conf_file = {?}", conf_file );


		cjsConfig	conf;
		conf.readFile( NULL );
		conf.script_time_limit = 0; //! no limit in console program

		cjsContext	cjs;
		st = cjs.create( conf );		if( !st ) return st;		

		jsval js_ret;
		cjs.evalFile( js_ret, filename );
		return 0;
	}
};

axCommandLineApp_program_main( "cjs", MyApp );
