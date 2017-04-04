#ifndef __cjsMyContext_h__
#define __cjsMyContext_h__

#include "common.h"


class MyContext : public cjsContext, public axDListNode<MyContext> {
    typedef cjsContext  B;
public:
	MyContext( request_rec *req );
	virtual ~MyContext();

    virtual void		onPrint	( const char* msg );
	virtual void		onError	( const char* filename, unsigned line_no, const char* message );

			axStatus	handleRequest( request_rec* req );
			bool		isShowError	() const { return showError_; }

			axStatus	loadCookie	();
			axStatus	newSession	();
			axStatus	loadSession	();
			axStatus	saveSession	();
			const char*	sessionId	();

	static	axStatus	cleanIdleSessions();

			axStatus	getSessionFilename( axIStringA &filename );
			axStatus	getPostData	( axIStringA &post_data );

			axStatus	decodeQuery	( cjsObject &obj, const char* sz );

	static	cjsConfig	static_config;
private:
    request_rec*	req_;
	cjsObject		js_http_;
	cjsObject		js_cookie_;
	bool			showError_;

	struct Session {
		axStringA		id_;
		axFile			file_;
		axStringA		filename_;
		axFileLock		fileLock_;
	};
	Session	session;
};


#endif //__cjsMyContext_h__
