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
#include <inttypes.h>
#include <map>
#include <math.h>
#include <nan.h>
#include <node.h>
#include <unordered_map>
#include "utility-v8.hpp"

namespace osn
{
	struct Property
	{
		enum class Type
		{
			INVALID,
			BOOL,
			INT,
			FLOAT,
			TEXT,
			PATH,
			LIST,
			COLOR,
			BUTTON,
			FONT,
			EDITABLELIST,
			FRAMERATE,
		};

		std::string name;
		std::string description;
		std::string long_description;

		bool enabled;
		bool visible;

		Type type;
	};

	struct NumberProperty : Property
	{
		enum class Type
		{
			SCROLLER,
			SLIDER,
		};

		union
		{
			struct
			{
				int64_t min, max, step, value;
			} int_value;
			struct
			{
				double_t min, max, step, value;
			} float_value;
			struct
			{
				bool value;
			} bool_value;
		};

		Type field_type;
	};

	struct TextProperty : Property
	{
		enum class Type
		{
			DEFAULT,
			PASSWORD,
			MULTILINE,
		};

		Type        field_type;
		std::string value;
	};

	struct PathProperty : Property
	{
		enum class Type
		{
			FILE,
			FILE_SAVE,
			PATH_DIRECTORY
		};

		Type        field_type;
		std::string filter;
		std::string default_path;
		std::string value;
	};

	struct ListProperty : Property
	{
		enum class Type
		{
			INVALID,
			EDITABLE,
			LIST,
		};
		enum class Format
		{
			INVALID,
			INT,
			FLOAT,
			STRING,
		};

		struct Item
		{
			std::string name;
			bool        disabled;

			union
			{
				int64_t  value_int;
				double_t value_float;
			};
			std::string value_str;
		};

		union
		{
			int64_t  current_value_int;
			double_t current_value_float;
		};
		std::string     current_value_str;

		Type            field_type;
		Format          item_format;
		std::list<Item> items;
	};

	struct FontProperty : Property
	{
		std::string face;
		std::string style;
		std::string path;
		int64_t     sizeF;
		uint32_t    flags;
	};

	// Contrary to the name, not compatible with the ListProperty. Actually more comparable to PathProperty.
	struct EditableListProperty : Property
	{
		enum class Type
		{
			STRINGS,
			FILES,
			FILES_AND_URLS,
		};

		Type        field_type;
		std::string filter;
		std::string default_path;
		std::string value;
	};

	struct FrameRateProperty : Property
	{
		struct FrameRate
		{
			uint32_t numerator;
			uint32_t denominator;
		};
		struct Option
		{
			std::string name;
			std::string description;
		};

		std::list<std::pair<FrameRate, FrameRate>> ranges; // minmax range
		std::list<Option> options; // Could also be a list, but this allows fast indexing by name.
	};

	// This is a class that basically implements the OBS behavior.
	typedef std::map<size_t, std::shared_ptr<Property>> property_map_t;

	// The actual classes that work with JavaScript
	class Properties : public Nan::ObjectWrap,
	                   public utilv8::InterfaceObject<Properties>,
	                   public utilv8::ManagedObject<Properties>
	{
		std::shared_ptr<property_map_t> properties;
		v8::Persistent<v8::Object>      owner;

		protected:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		public:
		Properties();
		Properties(property_map_t container);
		Properties(property_map_t container, v8::Local<v8::Object> owner);
		~Properties();

		std::shared_ptr<property_map_t> GetProperties();
		v8::Local<v8::Object>           GetOwner();

		static void                        Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);
		static Nan::NAN_METHOD_RETURN_TYPE Count(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE First(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Last(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Get(Nan::NAN_METHOD_ARGS_TYPE info);

		friend class utilv8::ManagedObject<Properties>;
		friend class utilv8::InterfaceObject<Properties>;
	};

	class PropertyObject : public Nan::ObjectWrap,
	                       public utilv8::InterfaceObject<PropertyObject>,
	                       public utilv8::ManagedObject<PropertyObject>
	{
		v8::Persistent<v8::Object> parent;
		size_t                     index;

		protected:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		public:
		PropertyObject(v8::Local<v8::Object> parent, size_t index);
		~PropertyObject();

		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE Previous(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Next(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsFirst(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsLast(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE GetValue(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE GetName(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetDescription(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetLongDescription(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsEnabled(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsVisible(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetType(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE GetDetails(Nan::NAN_METHOD_ARGS_TYPE info);

		static Nan::NAN_METHOD_RETURN_TYPE Modified(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE ButtonClicked(Nan::NAN_METHOD_ARGS_TYPE info);

		friend class utilv8::ManagedObject<PropertyObject>;
		friend class utilv8::InterfaceObject<PropertyObject>;
	};
} // namespace osn
