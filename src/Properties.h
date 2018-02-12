#pragma once

#include <nan.h>

#include "obspp/obspp-properties.hpp"
#include "Common.h"

namespace osn
{

class Properties : public Nan::ObjectWrap
{
public:
	static Nan::Persistent<v8::FunctionTemplate> prototype;
	obs::properties handle;

	typedef common::Object<Properties, obs::properties> Object;
	friend Object;

	Properties(obs::properties &properties) = delete;
	Properties(obs::properties &&properties);
	Properties(std::string id, obs::properties::object_type type);

	static NAN_MODULE_INIT(Init);

	/* NOTE Can create properties with an id but if you want
	   properties that corresond to an object, call the properties
	   method on that object. */
	static NAN_METHOD(New);

	static NAN_METHOD(first);
	static NAN_METHOD(count);
	static NAN_METHOD(get);
	static NAN_METHOD(apply);
	static NAN_METHOD(items);
};

class Property : public Nan::ObjectWrap
{
public:
	Nan::Persistent<v8::Object> parent; /** Properties object reference */

	static Nan::Persistent<v8::FunctionTemplate> prototype;
	obs::property handle;

	typedef common::Object<Property, obs::property> Object;
	friend Object;

	Property(v8::Local<v8::Object> parent, obs::property &property);
	~Property();

	static NAN_MODULE_INIT(Init);
	static NAN_METHOD(get_value);
	static NAN_METHOD(get_status);
	static NAN_METHOD(get_name);
	static NAN_METHOD(get_description);
	static NAN_METHOD(get_longDescription);
	static NAN_METHOD(get_type);
	static NAN_METHOD(get_enabled);
	static NAN_METHOD(get_visible);
	static NAN_METHOD(get_details);
	static NAN_METHOD(next);
	static NAN_METHOD(modified);
	static NAN_METHOD(buttonClicked);
};

}