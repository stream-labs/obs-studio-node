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

#pragma once
#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"
#include "properties.hpp"
#include "obs-property.hpp"
#include "cache-manager.hpp"

#undef strtoll
#include "nlohmann/json.hpp"

namespace osn
{
	struct SourceHotkeyInfo
	{
		std::string sourceName;
		std::string hotkeyName;
		std::string hotkeyDesc;
		size_t		hotkeyId;
	};

	typedef utilv8::managed_callback<std::shared_ptr<std::vector<SourceHotkeyInfo>>>	SourceCallback;

	class ISource : public Nan::ObjectWrap, public utilv8::InterfaceObject<osn::ISource>
	{
		friend class utilv8::InterfaceObject<osn::ISource>;

		protected:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		public:
		uint64_t sourceId;

		private:
		uint64_t m_uid;
	
		public:
		~ISource();
		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE Release(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Remove(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE IsConfigurable(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetProperties(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetSettings(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Update(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Load(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Save(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE GetType(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetName(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetName(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetOutputFlags(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetFlags(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetFlags(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetStatus(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetId(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetMuted(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetMuted(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetEnabled(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetEnabled(Nan::NAN_METHOD_ARGS_TYPE info);

		// Browser source interaction
		static Nan::NAN_METHOD_RETURN_TYPE SendMouseClick(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SendMouseMove(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SendMouseWheel(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SendFocus(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SendKeyClick(Nan::NAN_METHOD_ARGS_TYPE info);
	};
} // namespace osn
