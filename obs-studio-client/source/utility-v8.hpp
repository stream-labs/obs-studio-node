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

#pragma once
#include <node.h>
#include <nan.h>
#include <inttypes.h>
#include <math.h>

#ifdef __GNUC__
#define __deprecated__ __attribute_deprecated__
#else
#define __deprecated__ __declspec(deprecated)
#endif

#define FIELD_NAME(name) \
    Nan::New(name).ToLocalChecked()

#define ASSERT_INFO_LENGTH_AT_LEAST(info, length) \
    if ((info).Length() < length) { \
        Nan::ThrowError("Unexpected number of arguments"); \
        return; \
    }

#define ASSERT_INFO_LENGTH(info, length) \
    if ((info).Length() != (length)) { \
        Nan::ThrowError("Unexpected number of arguments"); \
        return; \
    }

#define ASSERT_GET_OBJECT_FIELD(object, field, var) \
    if (!utilv8::GetFromObject((object), (field), (var))) { \
        Nan::ThrowTypeError("Wrong Type"); \
        return; \
    }

#define ASSERT_GET_VALUE(value, var) \
    if (!utilv8::FromValue((value), (var))) { \
        Nan::ThrowTypeError("Wrong Type"); \
        return; \
    }

namespace utilv8 {
#pragma region ToValue
	// Integers
	inline v8::Local<v8::Value> ToValue(bool v) {
		return Nan::New<v8::Boolean>(v);
	}

	inline v8::Local<v8::Value> ToValue(int8_t v) {
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint8_t v) {
		return Nan::New<v8::Uint32>(v);
	}

	inline v8::Local<v8::Value> ToValue(int16_t v) {
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint16_t v) {
		return Nan::New<v8::Uint32>(v);
	}

	inline v8::Local<v8::Value> ToValue(int32_t v) {
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint32_t v) {
		return Nan::New<v8::Uint32>(v);
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value> ToValue(int64_t v) {
		return Nan::New<v8::Number>((double_t)v);
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value> ToValue(uint64_t v) {
		return Nan::New<v8::Number>((double_t)v);
	}

	// Floating Point
	inline v8::Local<v8::Value> ToValue(float_t v) {
		return Nan::New<v8::Number>(v);
	}

	inline v8::Local<v8::Value> ToValue(double_t v) {
		return Nan::New<v8::Number>(v);
	}

	// Text
	inline v8::Local<v8::Value> ToValue(const char* v) {
		return Nan::New<v8::String>(v).ToLocalChecked();
	}

	inline v8::Local<v8::Value> ToValue(std::string v) {
		return Nan::New<v8::String>(v).ToLocalChecked();
	}

	// Arrays
	inline v8::Local<v8::Value> ToValue(std::vector<int8_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int8_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Int8Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<uint8_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint8_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint8Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<int16_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int16_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Int16Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<uint16_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint16_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint16Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<int32_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int32_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint32Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<uint32_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint32_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint32Array::New(rv, 0, v.size());
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value> ToValue(std::vector<int64_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int64_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value> ToValue(std::vector<uint64_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint64_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<float_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(float_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float32Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToValue(std::vector<double_t> v) {
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(double_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	// Functions (Nan)
	inline v8::Local<v8::Function> ToValue(Nan::FunctionCallback v) {
		return Nan::New<v8::Function>(v);
	}

	// Helpers (Allows simply just using ToValue with already correct types)
	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Object> v) {
		return v;
	}

	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Value> v) {
		return v;
	}

	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Array> v) {
		return v;
	}
	
	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Function> v) {
		return v;
	}

#pragma endregion ToValue

#pragma region FromValue
	// Integers
	inline bool FromValue(v8::Local<v8::Value> l, bool& r) {
		if (l->IsBoolean()) {
			r = l->BooleanValue();
			return true;
		}
		return false;
	}
	
	inline bool FromValue(v8::Local<v8::Value> l, int8_t& r) {
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint8_t& r) {
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, int16_t& r) {
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint16_t& r) {
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, int32_t& r) {
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint32_t& r) {
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline bool FromValue(v8::Local<v8::Value> l, int64_t& r) {
		if (l->IsNumber()) {
			r = (int64_t)l->NumberValue();
			return true;
		}
		return false;
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline bool FromValue(v8::Local<v8::Value> l, uint64_t& r) {
		if (l->IsNumber()) {
			r = (uint64_t)l->NumberValue();
			return true;
		}
		return false;
	}

	// Floating Point
	inline bool FromValue(v8::Local<v8::Value> l, float_t& r) {
		if (l->IsNumber()) {
			r = (float_t)l->NumberValue();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, double_t& r) {
		if (l->IsNumber()) {
			r = (double_t)l->NumberValue();
			return true;
		}
		return false;
	}

	// Text
	inline bool FromValue(v8::Local<v8::Value> l, char*& r, size_t length) {
		if (l->IsString()) {
			auto v8s = l->ToString();
			if ((v8s->Utf8Length() + 1) <= length) {
				v8::String::Utf8Value utfv8(v8s);
				if (*utfv8) {
					memcpy(r, *utfv8, v8s->Utf8Length());
					r[v8s->Utf8Length()] = 0;
					return true;
				}
			}

		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, char*& r) {
		if (l->IsString()) {
			auto v8s = l->ToString();
			v8::String::Utf8Value utfv8(v8s);
			if (*utfv8) {
				r = (char*)malloc(v8s->Utf8Length() + 1);
				memcpy(r, *utfv8, v8s->Utf8Length());
				r[v8s->Utf8Length()] = 0;
				return true;
			}
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, std::string& r) {
		if (l->IsString()) {
			auto v8s = l->ToString();
			v8::String::Utf8Value utfv8(v8s);
			if (*utfv8) {
				r = std::string(*utfv8, v8s->Utf8Length());
				return true;
			}
		}
		return false;
	}

#pragma endregion FromValue
	
#pragma region Objects
	template<typename T> inline void SetObjectField(v8::Local<v8::Object> object, const char* field, T value) {
		Nan::Set(object, ToValue(field), ToValue(value));
	}

	template<typename T> inline void SetObjectField(v8::Local<v8::Object> object, uint32_t field, T value) {
		Nan::Set(object, field, ToValue(value));
	}

	template<> inline void SetObjectField<Nan::FunctionCallback>(v8::Local<v8::Object> object, const char *name, Nan::FunctionCallback value) {
		Nan::SetMethod(object, name, value);
	}

	template <typename Type>
	bool GetFromObject(v8::Local<v8::Object> object, const char *field, Type &var) {
		auto field_value = Nan::Get(object, FromValue(field)).ToLocalChecked();
		return FromValue(field_value, var);
	}

	inline void SetObjectAccessor(v8::Local<v8::Object> object, const char *name,
		Nan::GetterCallback get, Nan::SetterCallback set = nullptr) {
		Nan::SetAccessor(object, FIELD_NAME(name), get, set);
	}

	inline void SetObjectAccessorProperty(v8::Local<v8::Object> object, const char *name,
		Nan::FunctionCallback get, Nan::FunctionCallback set = nullptr) {
		object->SetAccessorProperty(FIELD_NAME(name), ToValue(get), ToValue(set));
	}
#pragma endregion Objects

#pragma region Object Fields
	template<typename T> inline void SetObjectTemplateField(v8::Local<v8::ObjectTemplate> object, const char* field, T value) {
		Nan::SetMethod(object, ToValue(field), ToValue(value));
	}

	template<typename T> inline void SetObjectTemplateField(v8::Local<v8::ObjectTemplate> object, uint32_t field, T value) {
		Nan::SetMethod(object, field, ToValue(value));
	}

	template<> inline void SetObjectTemplateField<Nan::FunctionCallback>(v8::Local<v8::ObjectTemplate> object, const char *name, Nan::FunctionCallback value) {
		Nan::SetMethod(object, name, value);
	}

	inline void SetObjectTemplateAccessor(v8::Local<v8::ObjectTemplate> object, const char *name,
		Nan::GetterCallback get, Nan::SetterCallback set = nullptr) {
		Nan::SetAccessor(object, FIELD_NAME(name), get, set);
	}

	inline void SetObjectTemplateAccessorProperty(v8::Local<v8::ObjectTemplate> object, const char *name,
		Nan::FunctionCallback get, Nan::FunctionCallback set = nullptr) {
		object->SetAccessorProperty(FIELD_NAME(name), Nan::New<v8::FunctionTemplate>(get),
			Nan::New<v8::FunctionTemplate>(set));
	}
#pragma endregion Object Fields

	template<typename T>
	inline bool SafeUnwrap(Nan::NAN_METHOD_ARGS_TYPE info, T*& valp) {
		T* val = Nan::ObjectWrap::Unwrap<T>(info.This());
		if (!val) {
			info.GetIsolate()->ThrowException(
				v8::Exception::TypeError(Nan::New<v8::String>(
					"No wrapped object.").ToLocalChecked()));
			return false;
		}
		valp = val;
		return true;
	}
	
	template<typename T>
	class ManagedObject {
		public:
		static v8::Local<v8::Object> Store(T* object) {
			auto obj = Nan::NewInstance(T::prototype.Get(v8::Isolate::GetCurrent())->InstanceTemplate()).ToLocalChecked();
			object->Wrap(obj);
			return obj;
		}

		static bool Retrieve(v8::Local<v8::Object> object, T*& valp) {
			T* val = Nan::ObjectWrap::Unwrap<T>(object);
			if (!val) {
				info.GetIsolate()->ThrowException(
					v8::Exception::TypeError(Nan::New<v8::String>(
						"No wrapped object.").ToLocalChecked()));
				return false;
			}
			valp = val;
			return true;
		}
	};
}
