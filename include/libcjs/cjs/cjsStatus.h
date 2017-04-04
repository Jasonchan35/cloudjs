#include "cjs_define.h"

class cjsStatus : public axStatus_Module {
public:
	enum {
		_start = -10999,
		#define axStatus_enum(n) n,
			#include "cjsStatus_enum.h"
		#undef axStatus_enum
	};
	virtual	const char*	c_str( int code ) {
		switch( code ) {
			#define axStatus_enum(n)   case n: return #n;
				#include "cjsStatus_enum.h"
			#undef axStatus_enum
			default: return "Unknown";
		}
	}
	cjsStatus() : axStatus_Module(_start) {}
};