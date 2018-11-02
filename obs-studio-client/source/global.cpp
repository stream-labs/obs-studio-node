// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "global.hpp"
#include <condition_variable>
#include <ipc-value.hpp>
#include <mutex>
#include "controller.hpp"
#include "error.hpp"
#include "input.hpp"
#include "scene.hpp"
#include "transition.hpp"
#include "utility-v8.hpp"

void osn::Global::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto ObsGlobal = Nan::New<v8::Object>();

	utilv8::SetObjectField(ObsGlobal, "getOutputSource", getOutputSource);
	utilv8::SetObjectField(ObsGlobal, "setOutputSource", setOutputSource);
	utilv8::SetObjectField(ObsGlobal, "getOutputFlagsFromId", getOutputFlagsFromId);
	utilv8::SetObjectAccessorProperty(ObsGlobal, "laggedFrames", laggedFrames);
	utilv8::SetObjectAccessorProperty(ObsGlobal, "totalFrames", totalFrames);

	utilv8::SetObjectAccessorProperty(ObsGlobal, "locale", getLocale, setLocale);

	Nan::Set(target, FIELD_NAME("Global"), ObsGlobal);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::getOutputSource(Nan::NAN_METHOD_ARGS_TYPE info)
{
	ASSERT_INFO_LENGTH(info, 1);
	uint32_t channel;
	ASSERT_GET_VALUE(info[0], channel);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Global", "GetOutputSource", {ipc::value(channel)});

	if (!ValidateResponse(response))
		return;

	if (response[2].value_union.i32 == 0) {
		// Input
		osn::Input* scene = new osn::Input(response[1].value_union.ui64);
		info.GetReturnValue().Set(osn::Input::Store(scene));
	} else if (response[2].value_union.i32 == 2) {
		// Transition
		osn::Transition* scene = new osn::Transition(response[1].value_union.ui64);
		info.GetReturnValue().Set(osn::Transition::Store(scene));
	} else if (response[2].value_union.i32 == 3) {
		// Scene
		osn::Scene* scene = new osn::Scene(response[1].value_union.ui64);
		info.GetReturnValue().Set(osn::Scene::Store(scene));
	}
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::setOutputSource(Nan::NAN_METHOD_ARGS_TYPE info)
{
	uint32_t              channel;
	v8::Local<v8::Object> source_object;
	osn::ISource*         source = nullptr;

	ASSERT_INFO_LENGTH(info, 2);
	ASSERT_GET_VALUE(info[0], channel);
	if (info[1]->IsObject()) {
		ASSERT_GET_VALUE(info[1], source_object);

		if (!osn::ISource::Retrieve(source_object, source)) {
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Global", "SetOutputSource", {ipc::value(channel), ipc::value(source ? source->sourceId : UINT64_MAX)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::getOutputFlagsFromId(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string id;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], id);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Global", "GetOutputFlagsFromId", {ipc::value(id)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::laggedFrames(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Global", "LaggedFrames", {});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::totalFrames(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Global", "TotalFrames", {});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::getLocale(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Global", "GetLocale", {});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::setLocale(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string locale;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], locale);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Global", "SetLocale", {ipc::value(locale)});

	if (!ValidateResponse(response))
		return;
}
