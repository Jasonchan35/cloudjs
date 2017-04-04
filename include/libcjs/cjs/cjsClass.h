#pragma once
#ifndef __cjsClass_h__
#define __cjsClass_h__

#include "cjsContext.h"

class	cjsClass : public axNonCopyable {
public:
	class	Prototype {
	public:
		Prototype( JSClass* c ) { obj=NULL; cls=c; }
		JSObject*	obj;
		JSClass*	cls;
	};

	JSObject*	js_obj() { return js_obj_; }

/*
	static	axStatus	newJSObject		( JSObject* &out, cjsContext* cjs, JSObject* parent = NULL );
	static	axStatus	newThis			( CLASS*	&out, cjsContext* cjs, JSObject* parent = NULL );

	static	JSObject*	js_proto		();
	static	axStatus	initPrototype	( cjsContext* cx, JSObject* parent_proto = NULL );
	static	Prototype&	prototype		();
*/
	cjsClass() { js_obj_ = NULL; }
	~cjsClass() {}

		virtual	axStatus	onSetPropertyByName	( cjsContext* cjs, jsid _id, const char* name, jsval & value ) { return 0; }
		virtual	axStatus	onGetPropertyByName	( cjsContext* cjs, jsid _id, const char* name, jsval & value ) { return 0; }

		virtual	axStatus	onSetPropertyByIndex( cjsContext* cjs, jsid _id, int index,        jsval & value ) { return 0; }
		virtual	axStatus	onGetPropertyByIndex( cjsContext* cjs, jsid _id, int index,        jsval & value ) { return 0; }

		static	JSBool		_onAddProperty	( JSContext *cx, JSObject *obj, jsid id, jsval *argv )		{ return JS_TRUE; } 
		static	JSBool		_onDelProperty	( JSContext *cx, JSObject *obj, jsid id, jsval *argv )		{ return JS_TRUE; }

		static	JSBool		_onCall			( JSContext *cx, uintN argc, jsval *argv )					{ return JS_TRUE; } 
		static	JSBool		_onEnumerate	( JSContext *cx, JSObject *obj )							{ return JS_TRUE; } 
		static	JSBool		_onResolve		( JSContext *cx, JSObject *obj, jsid id )					{ return JS_TRUE; } 
		static	JSBool		_onConvert		( JSContext *cx, JSObject *obj, JSType type, jsval *argv )	{ return JS_TRUE; }

protected:
	JSObject* js_obj_;
};

#define CJS_CLASS_START( T ) \
	class cjsClass_##T : public cjsClass { \
		typedef cjsClass_##T CLASS;	\
	public: \
		static	JSBool		_onConstructor( JSContext *cx, uintN argc, jsval *argv ) { \
			JSObject* obj; \
			if (JS_IsConstructing(cx, argv)) { \
				Prototype &proto = prototype(); \
				obj = JS_NewObjectWithGivenProto(cx, proto.cls, proto.obj, NULL); \
				if (!obj) return JS_FALSE; \
			} else { \
				obj = JS_THIS_OBJECT(cx, argv); \
			} \
			CLASS* c = new CLASS;	if( ! c ) return JS_FALSE; \
			JS_SetPrivate(cx,obj,c);  \
			c->js_obj_ = obj; \
			JS_SET_RVAL( cx, argv, OBJECT_TO_JSVAL(obj) ) ; \
			return JS_TRUE; \
		} \
		static void		_onFinalize	(JSContext *cx, JSObject *obj) { \
			Prototype &proto = prototype(); \
			CLASS *c = (CLASS*) JS_GetInstancePrivate(cx,obj,proto.cls,NULL); \
			delete c; \
		} \
		static	JSBool	_onSetProperty	(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp) { \
			Prototype &proto = prototype(); \
			CLASS *c = (CLASS*) JS_GetInstancePrivate(cx,obj,proto.cls,NULL); \
			if( !c ) return JS_TRUE; \
			cjsContext* cjs = cjsContext::getInstance(cx);	\
			jsval jv; \
			if( ! JS_IdToValue( cx, id, &jv ) ) return JS_FALSE; \
			switch( cjs->typeOf( jv ) ) { \
				case JSTYPE_STRING: { \
					axTempStringA name; \
					if( ! cjs->convert( name, jv ) ) return JS_FALSE; \
					c->onSetPropertyByName( cjs, id, name, *vp ); \
				}break; \
				case JSTYPE_NUMBER: { \
					int index; \
					if( ! cjs->convert( index, jv ) ) return JS_FALSE; \
					c->onSetPropertyByIndex( cjs, id, index, *vp ); \
				}break; \
				default: assert(false); \
			} \
			return JS_TRUE; \
		} \
		static	JSBool	_onGetProperty	(JSContext *cx, JSObject *obj, jsid id, jsval *vp) { \
			Prototype &proto = prototype(); \
			CLASS *c = (CLASS*) JS_GetInstancePrivate(cx,obj,proto.cls,NULL); \
			if( !c ) return JS_TRUE; \
			cjsContext* cjs = cjsContext::getInstance(cx);	\
			jsval jv; \
			if( ! JS_IdToValue( cx, id, &jv ) ) return JS_FALSE; \
			switch( cjs->typeOf( jv ) ) { \
				case JSTYPE_STRING: { \
					axTempStringA name; \
					if( ! cjs->convert( name, jv ) ) return JS_FALSE; \
					c->onGetPropertyByName( cjs, id, name, *vp ); \
				}break; \
				case JSTYPE_NUMBER: { \
					int index; \
					if( ! cjs->convert( index, jv ) ) return JS_FALSE; \
					c->onGetPropertyByIndex( cjs, id, index, *vp ); \
				}break; \
				default: assert(false); \
			} \
			return JS_TRUE; \
		} \
		static	JSObject* js_proto() { \
			Prototype &proto = prototype(); \
			return proto.obj; \
		} \
		static	Prototype&	prototype() { \
			static	JSClass	cls = { \
				#T,	\
				JSCLASS_HAS_PRIVATE,\
				_onAddProperty, \
				_onDelProperty, \
				_onGetProperty, \
				_onSetProperty,\
				_onEnumerate, \
				_onResolve, \
				_onConvert, \
				_onFinalize, \
				NULL, \
				NULL, \
				_onCall, \
				_onConstructor, \
				NULL, NULL, NULL, NULL,\
			}; \
			static	Prototype proto(&cls); \
			return proto; \
		} \
		static	axStatus	initPrototype( cjsContext* cjs ) { \
			Prototype &proto = prototype(); \
			proto.obj = JS_InitClass( cjs->js_cx(), cjs->js_global(), NULL, proto.cls, NULL, 0, cjs_props(), cjs_funcs(), NULL, NULL ); \
			return 0; \
		} \
		static	axStatus	newJSObject ( JSObject* &out, cjsContext* cjs ) { \
			Prototype &proto = prototype(); \
			out = JS_ConstructObject( cjs->js_cx(), proto.cls, proto.obj, NULL ); \
			if( !out ) return cjsStatus::error_newJSObject; \
			return 0; \
		} \
		static	axStatus	newObject ( CLASS* & out, cjsContext *cjs ) { \
			axStatus st; \
			JSObject* obj; \
			st = newJSObject( obj, cjs );	if( !st ) { out=NULL; return st; } \
			out = (CLASS*)JS_GetPrivate( cjs->js_cx(), obj ); \
			return 0; \
		} \
// -- End CJS_CLASS_START

#define CJS_CLASS_END	\
	};
//---

#define CJS_PROP_SPEC_START	\
	static JSPropertySpec* cjs_props() { \
		static	JSPropertySpec _spec[] = { \
//----
#define CJS_PROP_SPEC_END \
	{0} }; \
	return _spec; } \
//---
#define	CJS_PROP_SPEC( name, index, flags )	\
	{ #name, index, flags, NULL, NULL }, \
//----
#define	CJS_FUNC_SPEC_START \
	static JSFunctionSpec* cjs_funcs() { \
		static	JSFunctionSpec _spec[] = { \
//----
#define	CJS_FUNC_SPEC( F, min_args, flags ) \
	JS_FS( #F, _cjs_wrapper_##F, min_args, flags ), \
//----
#define	CJS_FUNC_SPEC_END	\
	JS_FS_END }; \
	return _spec; } \
//----
#define	CJS_FUNC( F ) \
	static	JSBool	_cjs_wrapper_##F( JSContext *cx, uintN argc, jsval *argv ) { \
		Prototype &proto = prototype(); \
		JSObject* o = JS_THIS_OBJECT(cx,argv); \
		CLASS *c = (CLASS*) JS_GetInstancePrivate(cx,o,proto.cls,NULL); \
		cjsContext* cjs = cjsContext::getInstance(cx); \
		axStatus st = c->cjs_##F( cjs, argc, argv ); \
		if( !st ) { \
			ax_log("JS FUNC {?} Error {?}", axFUNC_NAME, st ); \
			return JS_FALSE; \
		} \
		return JS_TRUE; \
	} \
	axStatus cjs_##F( cjsContext *cjs, uintN argc, jsval *argv ) 
//----
#define	CJS_GET_ARGV( index, value )	\
	cjs->getFuncArg( argc, argv, index, value )
//----
#define CJS_FIXED_ARGV( index, T, value ) \
	T value; { axStatus st = CJS_GET_ARGV(index,value); if( !st ) { ax_log("error CJS_FIXED_ARGV({?}", index ); return st; } }
//----
#define CJS_RETURN( value ) \
	{ jsval _ret; cjs->convert( _ret, value ); JS_SET_RVAL( cjs->cx_, argv, _ret ); return 0; }
//----

#endif //__cjsClass_h__
