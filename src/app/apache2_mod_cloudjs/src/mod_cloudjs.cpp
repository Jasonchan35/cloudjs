#include "MyContext.h"
#include "cjsClass_Http.h"

static const char* cloudjs_config_filename = NULL;

class	WatchDog : public axTimerThread {
	typedef axTimerThread	B;
public:
	enum {
		kInterval = 5, //seconds
		kCleanSessionsInterval = 60, // seconds
	};

	WatchDog() {
		axPRINT_FUNC_NAME
		cleanSessionsTime_ = 0 ;
		MyContext::static_config.readFile( cloudjs_config_filename );
		MyContext::cleanIdleSessions();
	}
	
	~WatchDog() {
		destroy();
	}

	virtual	void	onTimer() {
		MTData	mtd( mtd_ );
		MyContext*	p = mtd->ctx_list.head();
		for( ; p; p=p->next() ) {
			p->_heartBeat();
		}

		cleanSessionsTime_ += kInterval;
		if( cleanSessionsTime_ >= kCleanSessionsInterval ) {
			cleanSessionsTime_ = 0;
			MyContext::cleanIdleSessions();
		}
	}

	struct	_MTData {
		axDList<MyContext>	ctx_list;
	};
	typedef	axMutexProtected<_MTData>	MTData;
	MTData::Data	mtd_;
	
	static	WatchDog* getInstance() {
		static	WatchDog o;
		return  &o;
	}

	int	cleanSessionsTime_;
};

static apr_status_t cloudjs_handler( request_rec *req ) {
//    ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, req, "cloudjs-script");
//	ax_print( "Thread[{?}] cloudjs_handler\n", axSystem::getCurrentThreadID() );

//	axTempStringA	tmp;
//	axApp::getCurrentDir( tmp );
//	ax_print("getCurrentDir {?}\n", tmp );	
	
	axStatus st;
    MyContext cjs( req );

	st = cjs.create( MyContext::static_config );		if( !st ) return HTTP_INTERNAL_SERVER_ERROR;
			
	{	WatchDog::MTData	mtd( WatchDog::getInstance()->mtd_ );
		mtd->ctx_list.append( &cjs );
	}
	st = cjs.handleRequest( req );
	{	WatchDog::MTData	mtd( WatchDog::getInstance()->mtd_ );
		mtd->ctx_list.remove( &cjs );
	}

	if( !cjs.isShowError() && !st ) {
		switch( st.code() ) {
			case axStatus_Std::File_not_found:		return HTTP_NOT_FOUND;
			case axStatus_Std::File_access_denied:	return HTTP_FORBIDDEN;
		}
		return HTTP_INTERNAL_SERVER_ERROR;
	}else{
		return OK;
	}
}

static apr_status_t mod_cloudjs_handler_hook (request_rec *req) {
//	axPRINT_FUNC_NAME;
//	ax_print( "   request_rec->handle = {?}\n", req->handler );
	if( req->handler && ax_strcmp( req->handler, "cloudjs" ) == 0 ) {
		return cloudjs_handler(req);
	}
    return DECLINED; //no erroneous condition exists, but the module declines to handle the phase; the server tries to find another.
}

void mod_cloudjs_child_init_hook( apr_pool_t *pchild, server_rec *s ) {
	ax_log("Thread[{?}] mod_cloudjs_child init", axSystem::getCurrentThreadID() );
	WatchDog::getInstance()->create ( WatchDog::kInterval );
}


static void register_hooks (apr_pool_t *pool) {
	ap_hook_handler		( mod_cloudjs_handler_hook,		NULL, NULL, APR_HOOK_MIDDLE );
	ap_hook_child_init	( mod_cloudjs_child_init_hook,	NULL, NULL, APR_HOOK_MIDDLE );
}

static const char* set_cloudjs_ini(cmd_parms *cmd, void *dummy, const char *arg) {
	cloudjs_config_filename = arg;
	return NULL;
}

//==============
extern "C" {

static const command_rec cmd_rec[] = {
    AP_INIT_TAKE1("cloudjs_ini", (cmd_func)set_cloudjs_ini, NULL, RSRC_CONF, "CloudJS ini file path"),
    { NULL }
};

module AP_MODULE_DECLARE_DATA cloudjs_module = {
    // Only one callback function is provided.  Real
    // modules will need to declare callback functions for
    // server/directory configuration, configuration merging
    // and other tasks.
    STANDARD20_MODULE_STUFF,
    NULL, // mod_create_dir_conf,
    NULL, // mod_merge_dir_conf,
    NULL, // mod_create_server_conf,
    NULL, // mod_merge_server_conf,
    cmd_rec,
    register_hooks
};

} //extern "C"

