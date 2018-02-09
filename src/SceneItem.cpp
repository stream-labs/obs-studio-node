#include "SceneItem.h"
#include "Input.h"

namespace osn
{

Nan::Persistent<v8::FunctionTemplate> SceneItem::prototype
      = Nan::Persistent<v8::FunctionTemplate>();

SceneItem::SceneItem(obs::scene::item item)
	: handle(item)
{
}

void SceneItem::Init()
{
	auto locProto = Nan::New<v8::FunctionTemplate>();
	locProto->InstanceTemplate()->SetInternalFieldCount(1);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "source",
	                                      get_source);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "scene",
	                                      get_scene);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "visible",
	                                      get_visible, set_visible);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "selected",
	                                      get_selected, set_selected);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "position",
	                                      get_position, set_position);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "rotation",
	                                      get_rotation, set_rotation);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "scale",
	                                      get_scale, set_scale);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "alignment",
	                                      get_alignment, set_alignment);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(),
	                                      "boundsAlignment", get_boundsAlignment, set_boundsAlignment);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "bounds",
	                                      get_bounds, set_bounds);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(),
	                                      "transformInfo", get_transformInfo, set_transformInfo);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(),
	                                      "boundsType", get_boundsType, set_boundsType);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "crop",
	                                      get_crop, set_crop);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(),
	                                      "scaleFilter", get_scaleFilter, set_scaleFilter);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "id",
	                                      get_id);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "moveUp", moveUp);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "moveDown",
	                               moveDown);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "moveTop",
	                               moveTop);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "moveBottom",
	                               moveBottom);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "move", move);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "remove", remove);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "deferUpdateBegin",
	                               deferUpdateBegin);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "deferUpdateEnd",
	                               deferUpdateEnd);
	locProto->SetClassName(FIELD_NAME("SceneItem"));
	prototype.Reset(locProto);
}

NAN_METHOD(SceneItem::get_source)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	Input *binding = new Input(handle.source());
	auto object = Input::Object::GenerateObject(binding);

	info.GetReturnValue().Set(object);
}

NAN_METHOD(SceneItem::get_scene)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	Scene *binding = new Scene(handle.scene());
	auto object = Scene::Object::GenerateObject(binding);

	info.GetReturnValue().Set(object);
}

NAN_METHOD(SceneItem::remove)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	handle.remove();
}

NAN_METHOD(SceneItem::get_id)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	/* FIXME: id() returns uint64_t but JS can't hold that */
	info.GetReturnValue().Set((uint32_t)handle.id());
}

NAN_METHOD(SceneItem::get_visible)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.visible()));
}

NAN_METHOD(SceneItem::set_visible)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	bool visible;

	ASSERT_GET_VALUE(info[0], visible);

	handle.visible(visible);
}

NAN_METHOD(SceneItem::set_selected)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	bool selected;

	ASSERT_GET_VALUE(info[0], selected);

	handle.selected(selected);
}

NAN_METHOD(SceneItem::get_selected)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.selected()));
}

NAN_METHOD(SceneItem::set_position)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	vec2 position;

	ASSERT_GET_VALUE(info[0], position);

	handle.position(position);
}

NAN_METHOD(SceneItem::get_position)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	vec2 position = handle.position();

	info.GetReturnValue().Set(common::ToValue(position));
}

NAN_METHOD(SceneItem::set_rotation)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	float rotation;

	ASSERT_GET_VALUE(info[0], rotation);

	handle.rotation(rotation);
}

NAN_METHOD(SceneItem::get_rotation)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	float rotation = handle.rotation();

	info.GetReturnValue().Set(common::ToValue(rotation));
}

NAN_METHOD(SceneItem::set_scale)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	vec2 scale;

	ASSERT_GET_VALUE(info[0], scale);

	handle.scale(scale);
}

NAN_METHOD(SceneItem::get_scale)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	vec2 scale = handle.scale();

	info.GetReturnValue().Set(common::ToValue(scale));
}

NAN_METHOD(SceneItem::set_alignment)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	uint32_t alignment;

	ASSERT_GET_VALUE(info[0], alignment);

	handle.alignment(alignment);
}

NAN_METHOD(SceneItem::get_alignment)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.alignment()));
}

NAN_METHOD(SceneItem::set_boundsAlignment)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	uint32_t alignment;

	ASSERT_GET_VALUE(info[0], alignment);

	handle.bounds_alignment(alignment);
}

NAN_METHOD(SceneItem::get_boundsAlignment)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.bounds_alignment()));
}

NAN_METHOD(SceneItem::set_bounds)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	vec2 bounds;

	ASSERT_GET_VALUE(info[0], bounds);

	handle.scale(bounds);
}

NAN_METHOD(SceneItem::get_bounds)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	vec2 bounds = handle.bounds();

	info.GetReturnValue().Set(common::ToValue(bounds));
}

NAN_METHOD(SceneItem::set_transformInfo)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	v8::Local<v8::Object> tf_info_object;

	ASSERT_GET_VALUE(info[0], tf_info_object);

	obs_transform_info tf_info;

	ASSERT_GET_OBJECT_FIELD(tf_info_object, "pos", tf_info.pos);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "rot", tf_info.rot);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "scale", tf_info.scale);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "alignment", tf_info.alignment);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "boundsType", tf_info.bounds_type);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "boundsAlignment",
	                        tf_info.bounds_alignment);
	ASSERT_GET_OBJECT_FIELD(tf_info_object, "bounds", tf_info.bounds);

	handle.transform_info(tf_info);
}

NAN_METHOD(SceneItem::get_transformInfo)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	obs_transform_info tf_info = handle.transform_info();

	auto object = Nan::New<v8::Object>();

	common::SetObjectField(object, "pos", tf_info.pos);
	common::SetObjectField(object, "rot", tf_info.rot);
	common::SetObjectField(object, "scale", tf_info.scale);
	common::SetObjectField(object, "alignment", tf_info.alignment);
	common::SetObjectField(object, "boundsType", tf_info.bounds_type);
	common::SetObjectField(object, "boundsAlignment", tf_info.bounds_alignment);
	common::SetObjectField(object, "bounds", tf_info.bounds);

	info.GetReturnValue().Set(object);
}

NAN_METHOD(SceneItem::moveUp)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	handle.order(OBS_ORDER_MOVE_UP);
}

NAN_METHOD(SceneItem::moveDown)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	handle.order(OBS_ORDER_MOVE_DOWN);
}

NAN_METHOD(SceneItem::moveTop)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	handle.order(OBS_ORDER_MOVE_TOP);
}

NAN_METHOD(SceneItem::moveBottom)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
	handle.order(OBS_ORDER_MOVE_BOTTOM);
}

NAN_METHOD(SceneItem::move)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	ASSERT_INFO_LENGTH(info, 1);

	int position;

	ASSERT_GET_VALUE(info[0], position);

	handle.order_position(position);
}


NAN_METHOD(SceneItem::get_boundsType)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(handle.bounds_type());
}

NAN_METHOD(SceneItem::set_boundsType)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	enum obs_bounds_type bounds_type;

	ASSERT_GET_VALUE(info[0], bounds_type);

	handle.bounds_type(bounds_type);
}

NAN_METHOD(SceneItem::get_crop)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	obs_sceneitem_crop crop_info = handle.crop();
	auto object = Nan::New<v8::Object>();

	common::SetObjectField(object, "left", crop_info.left);
	common::SetObjectField(object, "right", crop_info.right);
	common::SetObjectField(object, "top", crop_info.top);
	common::SetObjectField(object, "bottom", crop_info.bottom);

	info.GetReturnValue().Set(object);
}


NAN_METHOD(SceneItem::set_crop)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	v8::Local<v8::Object> crop_info_object;
	obs_sceneitem_crop crop_info;

	ASSERT_GET_VALUE(info[0], crop_info_object);

	ASSERT_GET_OBJECT_FIELD(crop_info_object, "left", crop_info.left);
	ASSERT_GET_OBJECT_FIELD(crop_info_object, "right", crop_info.right);
	ASSERT_GET_OBJECT_FIELD(crop_info_object, "top", crop_info.top);
	ASSERT_GET_OBJECT_FIELD(crop_info_object, "bottom", crop_info.bottom);

	handle.crop(crop_info);
}

NAN_METHOD(SceneItem::get_scaleFilter)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(handle.scale_filter());
}

NAN_METHOD(SceneItem::set_scaleFilter)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	int scale_filter;

	ASSERT_GET_VALUE(info[0], scale_filter);

	handle.scale_filter(static_cast<enum obs_scale_type>(scale_filter));
}

NAN_METHOD(SceneItem::deferUpdateBegin)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	handle.defer_update_begin();
}

NAN_METHOD(SceneItem::deferUpdateEnd)
{
	obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	handle.defer_update_end();
}

}