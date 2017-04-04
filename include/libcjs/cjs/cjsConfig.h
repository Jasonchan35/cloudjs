/* 
 * File:   cjsConfig.h
 * Author: jason
 *
 * Created on July 25, 2011, 8:13 AM
 */

#ifndef __cjsConfig_h__
#define	__cjsConfig_h__

#include "cjs_define.h"

class cjsConfig {
public:
	cjsConfig();

	axStatus readFile( const char* filename );

	axStringA			config_filename;
	axStringA			module_dir;
	axArray<axStringA>	load_script;
	int					session_timeout;
	int					script_time_limit;
	bool				strict_mode;

	axIniParser	ini;
};


#endif	//__cjsConfig_h__


