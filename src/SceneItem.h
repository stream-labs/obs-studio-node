#pragma once

#include <nan.h>

#include "Scene.h"
#include "Common.h"

namespace osn
{

class SceneItem : public Nan::ObjectWrap
{
	static Nan::Persistent<v8::FunctionTemplate> prototype;

public:
	typedef common::Object<SceneItem, obs::scene::item> Object;
	friend Object;

	SceneItem(obs::scene::item item);

	obs::scene::item handle;

	static void Init();

	static NAN_METHOD(get_source);
	static NAN_METHOD(get_scene);

	static NAN_METHOD(remove);
	static NAN_METHOD(get_id);

	static NAN_METHOD(get_visible);
	static NAN_METHOD(set_visible);

	static NAN_METHOD(get_selected);
	static NAN_METHOD(set_selected);

	static NAN_METHOD(get_position);
	static NAN_METHOD(set_position);

	static NAN_METHOD(get_rotation);
	static NAN_METHOD(set_rotation);

	static NAN_METHOD(get_scale);
	static NAN_METHOD(set_scale);

	static NAN_METHOD(get_alignment);
	static NAN_METHOD(set_alignment);

	static NAN_METHOD(get_boundsAlignment);
	static NAN_METHOD(set_boundsAlignment);

	static NAN_METHOD(get_bounds);
	static NAN_METHOD(set_bounds);

	static NAN_METHOD(get_transformInfo);
	static NAN_METHOD(set_transformInfo);

	static NAN_METHOD(get_boundsType);
	static NAN_METHOD(set_boundsType);

	static NAN_METHOD(get_crop);
	static NAN_METHOD(set_crop);

	static NAN_METHOD(get_scaleFilter);
	static NAN_METHOD(set_scaleFilter);

	static NAN_METHOD(moveUp);
	static NAN_METHOD(moveDown);
	static NAN_METHOD(moveTop);
	static NAN_METHOD(moveBottom);
	static NAN_METHOD(move);
	static NAN_METHOD(deferUpdateBegin);
	static NAN_METHOD(deferUpdateEnd);
};

}