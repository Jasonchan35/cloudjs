#include <libcjs/cjs/cjsContext.h>
#include <libcjs/cjs/cjsObject.h>


cjsObject::cjsObject() { 
	cjs_=NULL;
	o_=NULL;
}

void cjsObject::init( cjsContext* cjs, JSObject* o ) { 
	cjs_=cjs; 
	o_=o; 
}

axStatus	cjsObject::newProperty( cjsObject & out, const char* name ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;

	axStatus st;
	st = cjs_->newJSObject( out );		if( !st ) return st;
	return setProperty( name, out );
}

axStatus	cjsObject::setProperty ( const char * name, jsval value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;

	if( ! JS_SetProperty( cjs_->js_cx(), o_, name, &value ) ) return cjsStatus::error_set_object_property;
	return 0;
}

axStatus	cjsObject::getProperty ( const char * name, jsval & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	if( ! JS_GetProperty( cjs_->js_cx(), o_, name, &value ) ) return cjsStatus::error_get_object_property;
	return 0;
}

axStatus	cjsObject::setElement  ( jsint index,       jsval value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	if( ! JS_SetElement( cjs_->js_cx(), o_, index, &value ) ) return cjsStatus::error_set_object_element;
	return 0;
}

axStatus	cjsObject::getElement  ( jsint index,       jsval & value ) {
	if( ! cjs_ ) return axStatus_Std::not_initialized;
	if( ! o_   ) return axStatus_Std::not_initialized;
	if( ! JS_GetElement( cjs_->js_cx(), o_, index, &value ) ) return cjsStatus::error_get_object_element;
	return 0;
}