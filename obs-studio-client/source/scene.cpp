/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "scene.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include "controller.hpp"
#include "error.hpp"
#include "input.hpp"
#include "ipc-value.hpp"
#include "sceneitem.hpp"
#include "shared.hpp"
#include "utility.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::Scene::prototype;

osn::Scene::Scene(uint64_t id)
{
	this->sourceId = id;
}

void osn::Scene::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Scene").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "create", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Create));
	utilv8::SetTemplateField(fnctemplate, "createPrivate", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), CreatePrivate));
	utilv8::SetTemplateField(fnctemplate, "fromName", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), FromName));

	// Prototype/Class Template
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "release", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Release));
	utilv8::SetTemplateField(objtemplate, "remove", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Remove));

	utilv8::SetTemplateAccessorProperty(objtemplate, "source", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), AsSource));
	utilv8::SetTemplateField(objtemplate, "duplicate", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Duplicate));
	utilv8::SetTemplateField(objtemplate, "add", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), AddSource));
	utilv8::SetTemplateField(objtemplate, "findItem", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), FindItem));
	utilv8::SetTemplateField(objtemplate, "moveItem", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), MoveItem));
	utilv8::SetTemplateField(objtemplate, "orderItems", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), OrderItems));
	utilv8::SetTemplateField(objtemplate, "getItemAtIdx", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetItemAtIndex));
	utilv8::SetTemplateField(objtemplate, "getItems", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetItems));
	utilv8::SetTemplateField(objtemplate, "getItemsInRange", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetItemsInRange));
	utilv8::SetTemplateField(objtemplate, "connect", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Connect));
	utilv8::SetTemplateField(objtemplate, "disconnect", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Disconnect));

	// Stuff
	utilv8::SetObjectField(
	    target, "Scene", fnctemplate->GetFunction(target->GetIsolate()->GetCurrentContext()).ToLocalChecked());
	prototype.Reset(fnctemplate);
}

void osn::Scene::Create(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], name);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Scene", "Create", std::vector<ipc::value>{ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	uint64_t sourceId = response[1].value_union.ui64;

	osn::Scene* obj   = new osn::Scene(sourceId);

	SceneInfo* si       = new SceneInfo();
	si->name            = name;
	si->id              = sourceId;
	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = "scene";
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(sourceId, name, sdi);
	CacheManager<SceneInfo*>::getInstance().Store(sourceId, name, si);

	args.GetReturnValue().Set(osn::Scene::Store(obj));
}

void osn::Scene::CreatePrivate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], name);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Scene", "CreatePrivate", std::vector<ipc::value>{ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	uint64_t sourceId = response[1].value_union.ui64;

	osn::Scene* obj   = new osn::Scene(sourceId);

	SceneInfo* si       = new SceneInfo();
	si->name            = name;
	si->id              = sourceId;
	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = "scene";
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(sourceId, name, sdi);
	CacheManager<SceneInfo*>::getInstance().Store(sourceId, name, si);

	args.GetReturnValue().Set(osn::Scene::Store(obj));
}

void osn::Scene::FromName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], name);

	SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(name);

	if (!si) {
		auto conn = GetConnection();
		if (!conn)
			return;

		std::vector<ipc::value> response = conn->call_synchronous_helper("Scene", "FromName", {ipc::value(name)});

		if (!ValidateResponse(response))
			return;

		si = new SceneInfo;
		si->id = response[1].value_union.ui64;
		si->name = name;
		CacheManager<SceneInfo*>::getInstance().Store(response[1].value_union.ui64, name, si);
	}
	osn::Scene* obj = new osn::Scene(si->id);
	args.GetReturnValue().Set(osn::Scene::Store(obj));
}

void osn::Scene::Release(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* source = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), source)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Scene", "Release", std::vector<ipc::value>{ipc::value(source->sourceId)});
}

void osn::Scene::Remove(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* source = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), source)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Scene", "Remove", std::vector<ipc::value>{ipc::value(source->sourceId)});
}

void osn::Scene::AsSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// Scenes are simply stored as a normal source object on the server, no additional calls necessary.
	osn::Scene* source = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), source)) {
		return;
	}

	osn::Input* obj = new osn::Input(source->sourceId);
	args.GetReturnValue().Set(osn::Input::Store(obj));
}

void osn::Scene::Duplicate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;
	int         duplicate_type;
	osn::Scene* source = nullptr;

	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), source)) {
		return;
	}

	ASSERT_INFO_LENGTH(args, 2);
	ASSERT_GET_VALUE(args[0], name);
	ASSERT_GET_VALUE(args[1], duplicate_type);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene",
	    "Duplicate",
	    std::vector<ipc::value>{ipc::value(source->sourceId), ipc::value(name), ipc::value(duplicate_type)});

	if (!ValidateResponse(response))
		return;

	uint64_t sourceId = response[1].value_union.ui64;

	osn::Scene* obj = new osn::Scene(sourceId);

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = "scene";
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(sourceId, name, sdi);
	CacheManager<SceneInfo*>::getInstance().Store(sourceId, name, new SceneInfo);

	args.GetReturnValue().Set(osn::Scene::Store(obj));
}

void osn::Scene::AddSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::vector<ipc::value> params;
	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	osn::Input* input = nullptr;
	if (args.Length() >= 1) {
		if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Input>(
		        args[0]->ToObject(Nan::GetCurrentContext()).ToLocalChecked(), input)) {
			return;
		}
	}
	params.push_back(ipc::value(scene->sourceId));
	params.push_back(ipc::value(input->sourceId));
	v8::Local<v8::Object> transform = v8::Object::New(v8::Isolate::GetCurrent());
	v8::Local<v8::Object> crop      = v8::Object::New(v8::Isolate::GetCurrent());
	
	transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("scaleX"))
	    .ToLocalChecked()
	    ->ToNumber(args.GetIsolate()->GetCurrentContext()).ToLocalChecked()->Value();

	if (args.Length() >= 2) {
		transform = args[1]->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("scaleX"))
		                                .ToLocalChecked()
		                                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()
		));

		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("scaleY"))
		                                .ToLocalChecked()
		                                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("visible"))
		                                .ToLocalChecked()
		                                ->ToBoolean(v8::Isolate::GetCurrent())
		                                ->Value()));
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("x"))
		                                .ToLocalChecked()
		                                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("y"))
		                                .ToLocalChecked()
		                                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("rotation"))
		                                .ToLocalChecked()
		                                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));

		crop = transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("crop"))
		           .ToLocalChecked()
		           ->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
		params.push_back(ipc::value(crop->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("left"))
		                                .ToLocalChecked()
		                                ->ToInteger(Nan::GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(crop->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("top"))
		                                .ToLocalChecked()
		                                ->ToInteger(Nan::GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(crop->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("right"))
		                                .ToLocalChecked()
		                                ->ToInteger(Nan::GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));
		params.push_back(ipc::value(crop->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("bottom"))
		                                .ToLocalChecked()
		                                ->ToInteger(Nan::GetCurrentContext())
		                                .ToLocalChecked()
		                                ->Value()));

		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("streamVisible"))
										.ToLocalChecked()
										->ToBoolean(v8::Isolate::GetCurrent())
										->Value()));
		params.push_back(ipc::value(transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("recordingVisible"))
										.ToLocalChecked()
										->ToBoolean(v8::Isolate::GetCurrent())
										->Value()));
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene", "AddSource", params);

	if (!ValidateResponse(response))
		return;

	uint64_t id     = response[1].value_union.ui64;
	int64_t  obs_id = response[2].value_union.i64;
	// Create new SceneItem
	osn::SceneItem* obj = new osn::SceneItem(id);
	SceneInfo*      si  = CacheManager<SceneInfo*>::getInstance().Retrieve(scene->sourceId);

	if (si) {
		si->items.push_back(std::make_pair(obs_id, id));
		si->itemsOrderCached = true;
	}

	SceneItemData* sid = new SceneItemData;
	sid->obs_itemId    = obs_id;
	sid->scene_id      = scene->sourceId;

	if (args.Length() >= 2) {
		// Position
		sid->posX = transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("x"))
		                .ToLocalChecked()
		                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                .ToLocalChecked()
		                ->Value();
		sid->posY = transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("y"))
		                .ToLocalChecked()
		                ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                .ToLocalChecked()
		                ->Value();
		sid->posChanged = false;

		// Scale
		sid->scaleX = transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("scaleX"))
		                  .ToLocalChecked()
		                  ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                  .ToLocalChecked()
		                  ->Value();
		sid->scaleY = transform->Get(args.GetIsolate()->GetCurrentContext(), utilv8::ToValue("scaleY"))
		                  .ToLocalChecked()
		                  ->ToNumber(args.GetIsolate()->GetCurrentContext())
		                  .ToLocalChecked()
		                  ->Value();
		sid->scaleChanged = false;

		// Visibility
		sid->isVisible = transform->Get(Nan::GetCurrentContext(), utilv8::ToValue("visible"))
		                     .ToLocalChecked()
		                     ->ToBoolean(v8::Isolate::GetCurrent())
		                     ->Value();
		sid->visibleChanged = false;

		// Crop
		sid->cropLeft = crop->Get(Nan::GetCurrentContext(), utilv8::ToValue("left"))
		                    .ToLocalChecked()
		                    ->ToInteger(Nan::GetCurrentContext())
		                    .ToLocalChecked()
		                    ->Value();
		sid->cropTop = crop->Get(Nan::GetCurrentContext(), utilv8::ToValue("top"))
		                   .ToLocalChecked()
		                   ->ToInteger(Nan::GetCurrentContext())
		                   .ToLocalChecked()
		                   ->Value();
		sid->cropRight = crop->Get(Nan::GetCurrentContext(), utilv8::ToValue("right"))
		                     .ToLocalChecked()
		                     ->ToInteger(Nan::GetCurrentContext())
		                     .ToLocalChecked()
		                     ->Value();
		sid->cropBottom = crop->Get(Nan::GetCurrentContext(), utilv8::ToValue("bottom"))
		                      .ToLocalChecked()
		                      ->ToInteger(Nan::GetCurrentContext())
		                      .ToLocalChecked()
		                      ->Value();
		sid->cropChanged = false;

		// Rotation
		sid->rotation = transform->Get(Nan::GetCurrentContext(), utilv8::ToValue("rotation"))
		                    .ToLocalChecked()
		                    ->ToNumber(Nan::GetCurrentContext())
		                    .ToLocalChecked()
		                    ->Value();
		sid->rotationChanged = false;

		// Stream visible
		sid->isStreamVisible = transform->Get(Nan::GetCurrentContext(), utilv8::ToValue("streamVisible"))
		                           .ToLocalChecked()
		                           ->ToBoolean(v8::Isolate::GetCurrent())
		                           ->Value();
		sid->streamVisibleChanged = false;

		// Recording visible
		sid->isRecordingVisible = transform->Get(Nan::GetCurrentContext(), utilv8::ToValue("recordingVisible"))
		                              .ToLocalChecked()
		                              ->ToBoolean(v8::Isolate::GetCurrent())
		                              ->Value();
		sid->recordingVisibleChanged = false;
	}

	CacheManager<SceneItemData*>::getInstance().Store(id, sid);	

	args.GetReturnValue().Set(osn::SceneItem::Store(obj));
}

void osn::Scene::FindItem(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool        haveName = false;
	std::string name;
	int64_t     position;

	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);
	if (args[0]->IsNumber()) {
		haveName = false;
		ASSERT_GET_VALUE(args[0], position);
	} else {
		haveName = true;
		ASSERT_GET_VALUE(args[0], name);
	}
	// } else {
	// 	Nan::TypeError("Expected string or number");
	// 	return;
	// }

	SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(scene->sourceId);

	if (si && !haveName) {
		auto find = [position](const std::pair<int64_t, uint64_t> &item) {
			return item.first == position;
		};

		auto itemIt = std::find_if(si->items.begin(), si->items.end(), find);
		if (itemIt != si->items.end()) {
			osn::SceneItem* obj = new osn::SceneItem(itemIt->second);
			args.GetReturnValue().Set(osn::SceneItem::Store(obj));
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene",
	    "FindItem",
	    std::vector<ipc::value>{ipc::value(scene->sourceId), (haveName ? ipc::value(name) : ipc::value(position))});

	if (!ValidateResponse(response))
		return;

	uint64_t id = response[1].value_union.ui64;

	osn::SceneItem* obj = new osn::SceneItem(id);
	args.GetReturnValue().Set(osn::SceneItem::Store(obj));
}

void osn::Scene::MoveItem(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	int from, to;
	ASSERT_INFO_LENGTH(args, 2);
	ASSERT_GET_VALUE(args[0], from);
	ASSERT_GET_VALUE(args[1], to);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene", "MoveItem", std::vector<ipc::value>{ipc::value(scene->sourceId), ipc::value(from), ipc::value(to)});

	ValidateResponse(response);

	SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(scene->sourceId);

	if (si && response.size() > 2) {
		si->items.clear();

		for (size_t i = 1; i < response.size(); i += 2) {
			si->items.push_back(std::make_pair(response[i + 1].value_union.i64, response[i].value_union.ui64));
		}
		si->itemsOrderCached = true;
	}
}

void osn::Scene::OrderItems(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	std::vector<int64_t> order;
	std::vector<char> order_char;
	ASSERT_INFO_LENGTH(args, 1);

	v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(args[0]); 

	order.resize(array->Length());
	for (unsigned int i = 0; i < array->Length(); i++ ) {
		if (Nan::Has(array, i).FromJust()) {
			order[i] = Nan::Get(array, i).ToLocalChecked()->IntegerValue(Nan::GetCurrentContext()).FromJust();
		}
	}
	order_char.resize(order.size()*sizeof(int64_t));
	memcpy(order_char.data(), order.data(), order_char.size());

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene", "OrderItems", std::vector<ipc::value>{ipc::value(scene->sourceId), ipc::value(order_char)});

	ValidateResponse(response);

	SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(scene->sourceId);

	if (si && response.size() > 2) {
		si->items.clear();

		for (size_t i = 1; i < response.size(); i += 2) {
			si->items.push_back(std::make_pair(response[i + 1].value_union.i64, response[i].value_union.ui64));
		}
		si->itemsOrderCached = true;
	}
}

void osn::Scene::GetItemAtIndex(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	int32_t index;

	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], index);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene", "GetItem", std::vector<ipc::value>{ipc::value(scene->sourceId), ipc::value(index)});

	if (!ValidateResponse(response))
		return;

	uint64_t id = response[1].value_union.ui64;

	osn::SceneItem* obj = new osn::SceneItem(id);
	args.GetReturnValue().Set(osn::SceneItem::Store(obj));
}

void osn::Scene::GetItems(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(scene->sourceId);

	if (si && si->itemsOrderCached) {
		auto   arr         = Nan::New<v8::Array>(int(si->items.size()) - 1);
		size_t index = 0;
		bool   itemRemoved = false;

		for (auto item : si->items) {
			SceneItemData*  sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item.first);
			if (!sid) {
				itemRemoved = true;
				break;
			}
			osn::SceneItem* obj = new osn::SceneItem(item.second);
			Nan::Set(arr, uint32_t(index++), osn::SceneItem::Store(obj));
		}
		if (!itemRemoved) {
			args.GetReturnValue().Set(arr);
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Scene", "GetItems", std::vector<ipc::value>{ipc::value(scene->sourceId)});

	if (!ValidateResponse(response))
		return;

	auto arr = Nan::New<v8::Array>(int((response.size()) - 1)/2);
	size_t index = 0;
	for (size_t i = 1; i < response.size(); i++) {
		osn::SceneItem* obj = new osn::SceneItem(response[i++].value_union.ui64);
		Nan::Set(arr, uint32_t(index++), osn::SceneItem::Store(obj));
	}

	args.GetReturnValue().Set(arr);

	if (si) {
		si->items.clear();

		for (size_t i = 1; i < response.size(); i+= 2) {
			si->items.push_back(std::make_pair(response[i + 1].value_union.i64, response[i].value_union.ui64));
		}

		si->itemsOrderCached = true;
	}
}

void osn::Scene::GetItemsInRange(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Scene* scene = nullptr;
	if (!utilv8::RetrieveDynamicCast<osn::ISource, osn::Scene>(args.This(), scene)) {
		return;
	}

	int32_t from, to;
	ASSERT_INFO_LENGTH(args, 2);
	ASSERT_GET_VALUE(args[0], from);
	ASSERT_GET_VALUE(args[1], to);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Scene",
	    "GetItemsInRange",
	    std::vector<ipc::value>{ipc::value(scene->sourceId), ipc::value(from), ipc::value(to)});

	if (!ValidateResponse(response))
		return;

	auto arr = Nan::New<v8::Array>(int(response.size() - 1));

	for (size_t i = 1; i < response.size(); i++) {
		osn::SceneItem* obj = new osn::SceneItem(response[i].value_union.ui64);
		Nan::Set(arr, uint32_t(i - 1), osn::SceneItem::Store(obj));
	}

	args.GetReturnValue().Set(arr);
}

/**
If libobs allowed us the ability to parse
or obtain info about the signals associated with
a handler, this could be done generically instead of a
hard coded table like this.

Notice that in this case, the signal handler of a scene
is in addition to the signals a source can receive.
However, I just require you use the signal handler
associated with the input object instead to keep things
simple.
*/
//static const char *signal_type_map[] = {
//	"item_add",
//	"item_remove",
//	"reorder",
//	"item_visible",
//	"item_select",
//	"item_deselect",
//	"item_transform"
//};
//
//enum signal_types {
//	SIG_ITEM_ADD,
//	SIG_ITEM_REMOVE,
//	SIG_REORDER,
//	SIG_ITEM_VISIBLE,
//	SIG_ITEM_SELECT,
//	SIG_ITEM_DESELECT,
//	SIG_ITEM_TRANSORM,
//	SIG_TYPE_OVERFLOW
//};
//
//static calldata_desc scene_signal_desc[] = {
//	{ "scene", CALLDATA_TYPE_SCENE },
//{ "", CALLDATA_TYPE_END }
//};
//
//static calldata_desc item_signal_desc[] = {
//	{ "scene", CALLDATA_TYPE_SCENE },
//{ "item", CALLDATA_TYPE_SCENEITEM },
//{ "", CALLDATA_TYPE_END }
//};
//
//static calldata_desc item_visible_signal_desc[] = {
//	{ "scene", CALLDATA_TYPE_SCENE },
//{ "item", CALLDATA_TYPE_SCENEITEM },
//{ "visibility", CALLDATA_TYPE_BOOL },
//{ "", CALLDATA_TYPE_END }
//};
//
//
//static calldata_desc *callback_desc_map[] = {
//	item_signal_desc,
//	item_signal_desc,
//	scene_signal_desc,
//	item_visible_signal_desc,
//	item_signal_desc,
//	item_signal_desc,
//	item_signal_desc
//};

void osn::Scene::Connect(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	//obs::weak<obs::scene> &scene = Scene::Object::GetHandle(args.Holder());
	//Scene* this_binding = Nan::ObjectWrap::Unwrap<Scene>(args.Holder());

	//uint32_t signal_type;
	//v8::Local<v8::Function> callback;

	//ASSERT_GET_VALUE(args[0], signal_type);
	//ASSERT_GET_VALUE(args[1], callback);

	//if (signal_type >= SIG_TYPE_OVERFLOW || signal_type < 0) {
	//	Nan::ThrowError("Detected signal type out of range");
	//	return;
	//}

	//SceneSignalCallback *cb_binding =
	//	new SceneSignalCallback(
	//		this_binding,
	//		CalldataEventHandler<Scene, callback_data, SceneSignalCallback>,
	//		callback);

	//cb_binding->user_data =
	//	callback_desc_map[signal_type];

	//scene.get()->connect(
	//	signal_type_map[signal_type],
	//	GenericSignalHandler<SceneSignalCallback>,
	//	cb_binding);

	//auto object = SceneSignalCallback::Object::GenerateObject(cb_binding);
	//cb_binding->obj_ref.Reset(object);
	//args.GetReturnValue().Set(object);
}

void osn::Scene::Disconnect(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	//obs::weak<obs::scene> &scene = Scene::Object::GetHandle(args.Holder());

	//uint32_t signal_type;
	//v8::Local<v8::Object> cb_data_object;

	//ASSERT_GET_VALUE(args[0], signal_type);
	//ASSERT_GET_VALUE(args[1], cb_data_object);

	//if (signal_type >= SIG_TYPE_OVERFLOW || signal_type < 0) {
	//	Nan::ThrowError("Detected signal type out of range");
	//	return;
	//}

	//SceneSignalCallback *cb_binding =
	//	SceneSignalCallback::Object::GetHandle(cb_data_object);

	//cb_binding->stopped = true;
	//cb_binding->obj_ref.Reset();

	//scene.get()->disconnect(
	//	signal_type_map[signal_type],
	//	GenericSignalHandler<SceneSignalCallback>,
	//	cb_binding);
}
