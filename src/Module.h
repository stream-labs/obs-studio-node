#pragma once
#include <nan.h>

#include "obspp/obspp-module.hpp"
#include "Common.h"

namespace osn
{


NAN_METHOD(add_path);
NAN_METHOD(load_all);
NAN_METHOD(log_loaded);

class Module : Nan::ObjectWrap
{

private:
	static NAN_METHOD(New);

public:
	typedef common::Object<Module, obs::module> Object;
	friend Object;

	obs::module handle;

	Module(std::string path, std::string data_path);
	Module(obs::module &module);

	/* Prototype Scope */
	static NAN_MODULE_INIT(Init);
	static NAN_METHOD(create);
	static NAN_METHOD(addPath);
	static NAN_METHOD(loadAll);
	static NAN_METHOD(logLoaded);

	/* Instance Scope */
	static NAN_METHOD(initialize);
	static NAN_METHOD(get_filename);
	static NAN_METHOD(get_name);
	static NAN_METHOD(get_author);
	static NAN_METHOD(get_description);
	static NAN_METHOD(get_binPath);
	static NAN_METHOD(get_dataPath);
	static NAN_METHOD(get_status);
};

}