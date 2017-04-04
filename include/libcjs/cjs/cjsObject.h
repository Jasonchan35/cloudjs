#ifndef __cjsObject_h__
#define __cjsObject_h__

#include "cjs_define.h"

class cjsContext;

class cjsObject {
public:
	cjsObject();
	
	void init( cjsContext* cjs, JSObject* o );

	operator	JSObject*	()		{ return o_; }
	JSObject*	js_obj		() const	{ return o_; }

						axStatus	newProperty ( cjsObject & out, const char* name );

						axStatus	setProperty ( const char * name, jsval     value );			
	template<class T>	axStatus	setProperty ( const char * name, const T&  value );

						axStatus	getProperty ( const char * name, jsval  &  value );
	template<class T>	axStatus	getProperty ( const char * name, T      &  value );
			
						axStatus	setElement  ( int index, jsval    value );
	template<class T>	axStatus	setElement  ( int index, const T& value );

						axStatus	getElement  ( int index, jsval &  value );
	template<class T>	axStatus	getElement  ( int index, T &	  value );


private:
	cjsContext* cjs_;
	JSObject*	o_;
};


//-------

template<class T> inline
axStatus	cjsObject::setProperty ( const char * name, const T & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;

	jsval j; 
	axStatus st;
	st = cjs_->convert( j, value );	if( !st ) return st;
	st = setProperty( name, j );	if( !st ) return st;
	return 0;
}


template<class T> inline
axStatus	cjsObject::getProperty ( const char * name, T & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	jsval j; 
	axStatus st;
	st = getProperty( name, j );	if( !st ) return st;
	st = cjs_->convert( value, j );		if( !st ) return st;
	return 0;
}

template<class T> inline
axStatus	cjsObject::setElement  ( int index, const T & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	jsval j; 
	axStatus st;
	st = cjs_->convert( j, value );		if( !st ) return st;
	st = setElement( index, j );	if( !st ) return st;
	return 0;
}

template<class T> inline
axStatus	cjsObject::getElement  ( int index, T & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	jsval j;
	axStatus st;
	st = getElement( index, j );	if( !st ) return st;
	st = cjs_->convert( value, j );		if( !st ) return st;
	return 0;
}

#endif //__cjsObject_h__

