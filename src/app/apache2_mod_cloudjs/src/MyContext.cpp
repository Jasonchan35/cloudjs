#include "MyContext.h"
#include "cjsClass_Http.h"

cjsConfig	MyContext::static_config;

MyContext::MyContext( request_rec *req ) {
	showError_ = true;
	req_ = req;
}

MyContext::~MyContext() {
}

void MyContext::onPrint( const char* msg ) {
	if( req_ ) ap_rputs( msg, req_ );
}

void MyContext::onError( const char* filename, unsigned line_no, const char* message ) {
	if( showError_ ) {
		onPrint( message );
		axTempStringA	tmp;
		tmp.format("\nFile: {?}\nLine: {?}\n", filename, line_no );	
		onPrint( tmp );
	}
	B::onError( filename, line_no, message );
}

axStatus MyContext::getPostData( axIStringA &post_data ) {
	axStatus st;
	apr_bucket_brigade *bb;
	bb = apr_brigade_create(req_->pool, req_->connection->bucket_alloc);

	bool	seen_eos = false;
	int		child_stopped_reading = 0;
	apr_status_t	ap_ret;

    const char *data;
    apr_size_t len;

    while( !seen_eos ) {
        apr_bucket *bucket;
        ap_ret = ap_get_brigade(req_->input_filters, bb, AP_MODE_READBYTES, APR_BLOCK_READ, HUGE_STRING_LEN);
        if (ap_ret != APR_SUCCESS) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        for (bucket =  APR_BRIGADE_FIRST(bb); 
			 bucket != APR_BRIGADE_SENTINEL(bb);
             bucket =  APR_BUCKET_NEXT(bucket))
        {
            if (APR_BUCKET_IS_EOS(bucket)) { 
				seen_eos = true;
                break;
            }

            // We can't do much with this.
            if ( APR_BUCKET_IS_FLUSH(bucket) ) continue;

            // If the child stopped, we still must read to EOS.
            if ( child_stopped_reading ) continue;

            // read 
            apr_bucket_read(bucket, &data, &len, APR_BLOCK_READ);
			st = post_data.appendWithLength( data, len );
            if ( ! st ) { // silly script stopped reading, soak up remaining message
                child_stopped_reading = 1;
            }
        }
        apr_brigade_cleanup(bb);
    }

    apr_brigade_cleanup(bb);
	apr_brigade_destroy(bb);

	return 0;
}

const char* MyContext::sessionId() {
	return session.id_;
}

axStatus	MyContext::decodeQuery( cjsObject &obj, const char* src ) {
	if( !src ) return 0;
	axStatus st;
	const char* p = src;
	const char* s = p;
	int ret;

	axTempStringA	value;
	axTempStringA	name;

	bool name_part = true;

	for( ;*p; ) {
		if( name_part ) {
			switch( *p ) {
				case '%': {
					st = name.appendWithLength( s, p-s );	if( !st ) return st;
					p++; ret = ax_hex_to_int( *p );			if( ret < 0 ) return axStatus_Std::error_decodeURI;
					uint8_t value = ret << 4;
					p++; ret = ax_hex_to_int( *p );			if( ret < 0 ) return axStatus_Std::error_decodeURI;
					value |= ret;
					st = name.append( (char)value );		if( !st ) return st;
					p++;
					s = p;
				}break;
				case '+': {
					st = name.appendWithLength( s, p-s );	if( !st ) return st;
					st = name.append( ' ' );				if( !st ) return st;
					p++;
					s = p;
				}break;
				case '=': {
					st = name.appendWithLength( s, p-s );	if( !st ) return st;
					p++;
					name_part = false;
					s = p;
				}break;
				default: p++;
			}
		}else{ //value part
			switch( *p ) {
				case '%': {
					st = value.appendWithLength( s, p-s );	if( !st ) return st;
					p++; ret = ax_hex_to_int( *p );			if( ret < 0 ) return axStatus_Std::error_decodeURI;
					char hex = ret << 4;
					p++; ret = ax_hex_to_int( *p );			if( ret < 0 ) return axStatus_Std::error_decodeURI;
					hex |= ret;
					st = value.append( (char)hex );		if( !st ) return st;
					p++;
					s = p;
				}break;
				case '+': {
					st = value.appendWithLength( s, p-s );	if( !st ) return st;
					st = value.append( ' ' );				if( !st ) return st;
					p++;
					s = p;
				}break;
				case '&': {
					st = value.appendWithLength( s, p-s );	if( !st ) return st;
					st = obj.setProperty( name, value );	if( !st ) return st;

					p++;
					s = p;

					name_part = true;
					name.clear();
					value.clear();
				}break;
				default: p++;
			}
		}
	}

	st = value.appendWithLength( s, p-s );	if( !st ) return st;
	st = obj.setProperty( name, value );	if( !st ) return st;
	return 0;
}

axStatus	MyContext::handleRequest( request_rec* req ) {
//    ap_set_content_type(req, "text/html");
	axStatus	st;
	st = cjsClass_http::initPrototype( this );	if( !st ) return st;

	js_http_.init( this, cjsClass_http::js_proto() );

	st = loadCookie();		if( !st ) return st;
	st = loadSession();		
	if( !st ) {
		st = newSession();		if( !st ) return st;
	}

	js_http_.setProperty( "sessionId",	sessionId() );
	js_http_.setProperty( "hostname",	req->hostname );
	js_http_.setProperty( "uri",		req->uri );
	js_http_.setProperty( "method",		req->method );


	cjsObject	js_http_arg;
	st = js_http_.newProperty( js_http_arg, "arg" );			if( !st ) return st;

//--- GET
	cjsObject	js_http_get;
	js_http_.setProperty("_get", req->args );				if( !st ) return st;
	st = js_http_.newProperty( js_http_get, "get" );		if( !st ) return st;
	st = decodeQuery( js_http_get, req->args );				if( !st ) return st;
	st = decodeQuery( js_http_arg, req->args );				if( !st ) return st;

//--- POST
	if( req->method_number == M_POST ) {
		axStringA_<4096>	_post;
		_post.setCapacityIncrement( 16*1024 );
		st = getPostData( _post );							if( !st ) return st;

		cjsObject	js_http_post;
		js_http_.setProperty("_post", _post );				if( !st ) return st;
		st = js_http_.newProperty( js_http_post, "post" );	if( !st ) return st;
		st = decodeQuery( js_http_post,  _post );			if( !st ) return st;
		st = decodeQuery( js_http_arg,   _post );			if( !st ) return st;
	}
	
	jsval js_ret;
	st = evalFile( js_ret, req->filename );					if( !st ) return st;
	st = saveSession();										if( !st ) return st;
	return 0;
}

axStatus MyContext::loadCookie() {
	axStatus st;
	const char* cookie = apr_table_get( req_->headers_in, "Cookie" );
	if( !cookie ) return 0;

//	find_var( cookie, "CJS_SESSION_ID", sessionId_ );
//	ax_print("find session id = {?}\n", sessionId_ );

	st = js_http_.newProperty( js_cookie_, "cookie" );		if( !st ) return st;

	const char* p = cookie;
	const char* s;

	axTempStringA	name;
	axTempStringA	value;

	for(;;) {
		//trim space
		for( ;; p++ ) {
			if( *p == 0  ) return 0;
			if( *p != ' ') break;
		}
		s = p;

		for( ;; p++ ) {
			if( *p == 0 ) return 0;
			if( *p == '=' ) break;
		}
		st = name.setWithLength( s, p-s );		if( !st ) return st;
		p++; // skip '='
		s = p;

		for(;; p++ ) {
			if( *p == 0 || *p == ';' ) {
				st = value.setWithLength( s, p-s );		if( !st ) return st;
				//ax_print("--> cookie = [{?}] [{?}]\n", name, value );

				if( name.equals( "CJS_SESSION_ID" ) ) {
					st = session.id_.set( value );	if( !st ) return st;
				}else{
					st = js_cookie_.setProperty( name, value );		if( !st ) return st;
				}

				if( *p == 0 ) return 0;
				p++; //skip ';'

				break;
			}
		}
	}

	return 0;
}

axStatus MyContext::getSessionFilename( axIStringA &filename ) {
	axStatus st;
	axTempStringA tmpDir;
	st = axApp::getTempDir( tmpDir );										if( !st ) return st;
	st = filename.format("{?}/cloudjs/sessions/{?}", tmpDir, session.id_ );	if( !st ) return st;
	return 0;
}

axStatus MyContext::cleanIdleSessions() {
	if( static_config.session_timeout <= 0 ) return 0;

	axStatus st;
	axTempStringA	tmpDir;
	st = axApp::getTempDir( tmpDir );							if( !st ) return st;

	axTempStringA	path;
	st = path.format("{?}/cloudjs/sessions", tmpDir );			if( !st ) return st;

	axArray< axDir::Entry >	entries;
	st = axDir::getEntries( path, entries );					if( !st ) return st;
	
	axTimeStamp	expiredTimestamp;
	expiredTimestamp.now();

	expiredTimestamp.addSecond( -static_config.session_timeout );
	//ax_print("expire = {?}\n", expiredTimestamp );

	axDir	dir;
	axDir::Entry	entry;

	st = dir.open( path );		if( !st ) return st;

	axTimeStamp	t;
	axDateTime	dt;
	axTempStringA	fullpath;

	for(;;) {
		if( ! dir.next( entry ) ) break;
		if( entry.isDir() ) continue;

		if( ! fullpath.format( "{?}/{?}", path, entry.filename ) ) continue;
		if( ! axFileSystem::fileLastWriteTime( t, fullpath ) ) continue;

//		ax_print("filename = {?} {?}\n", fullpath, t );
		if( t < expiredTimestamp ) {
			//ax_print("---- expired ----\n");
			axFileSystem::deleteFile( fullpath );
		}
	}

	return 0;
}

axStatus MyContext::newSession() {
	axStatus st;
	axByteArray_<128>	buf;

	st = buf.appendN( (uint8_t*)req_->connection->remote_addr->ipaddr_ptr, req_->connection->remote_addr->ipaddr_len );
	if( !st ) return st;

	axTimeStamp	ts;
	ts.now();

	int64_t	i64 = (int64_t)( ts.second() * 10000 );
	st = buf.appendN( (uint8_t*)&i64, sizeof(i64) );

	uint8_t* p = buf.ptr();
	size_t   n = buf.byteSize();
	for( size_t i=0; i<n; i++ ) {
		p[i] ^= 0x5A;
	}

	st = ax_to_hex_str( session.id_, buf );		if( !st ) return st;

	st = getSessionFilename( session.filename_ );				if( !st ) return st;

	axTempStringA	path;

	axTempStringA	tempDir;
	st = axApp::getTempDir( tempDir );							if( !st ) return st;
	st = path.format( "{?}/cloudjs", tempDir );					if( !st ) return st;
	axFileSystem::makeDirectory( path );

	st = axFilePath::getDirName( path, session.filename_ );		if( !st ) return st;
	axFileSystem::makeDirectory( path );

	st = session.file_.openWrite( session.filename_, false );	if( !st ) return st;
	st = session.fileLock_.lock( session.file_, true );			if( !st ) return st;

	jsval js_ret;
	st = eval(js_ret, "http.session = {}");						if( !st ) return st;

	axTempStringA tmp;
	st = tmp.format( "CJS_SESSION_ID={?};", session.id_ );		if( !st ) return st;
	apr_table_add( req_->headers_out, "Set-Cookie", tmp );
	return 0;
}

axStatus MyContext::loadSession() {
	axStatus st;

	jsval js_ret;
	st = eval(js_ret, "http.session = {}");						if( !st ) return st;

/*
	const apr_array_header_t *t = apr_table_elts( req_->headers_in );
	const apr_table_entry_t  *e = (const apr_table_entry_t*)t->elts;
	for( int i=0; i<t->nelts; i++ ) {
		ax_print("header_in {?}:{?}\n", e[i].key, e[i].val );
	}

	const char* cookie = apr_table_get( req_->headers_in, "Cookie" );
	if( cookie ) {
		find_var( cookie, "CJS_SESSION_ID", sessionId_ );
		ax_print("find session id = {?}\n", sessionId_ );
	}
*/
	st = getSessionFilename( session.filename_ );				if( !st ) return st;
	st = session.file_.openAppend( session.filename_,false );	if( !st ) return st;
	st = session.fileLock_.lock( session.file_, true );			if( !st ) return st;

	axTempStringA	tmp;
	st = session.file_.readWholeFile( tmp );						if( !st ) return st;
	if( ! tmp.isEmpty() && ! tmp.equals("undefined") ) {
		axTempStringA	script;
		st = script.format( "http.session = JSON.parse('{?}');", tmp );	if( !st ) return st;

		eval( js_ret, script );
	}

	//get session id from http header
	//if no, then generate id
	//load session values from file
	return 0;
}

axStatus MyContext::saveSession() {
	//save session values to file
	axStatus st;
	if( ! session.file_.isValid() ) return 0;

	jsval js_ret;
	st = eval( js_ret, "JSON.stringify( http.session )" );	if( !st ) return st;
	
	axTempStringA	str;
	st = convert( str, js_ret );			if( !st ) return st;
	st = session.file_.setFileSize(0);		if( !st ) return st;
	st = session.file_.writeString( str );	if( !st ) return st;

	return 0;
}
