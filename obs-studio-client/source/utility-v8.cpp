
#include "utility-v8.hpp"

std::string ValueToTypeName(v8::Local<v8::Value> v) {
	v8::Local<v8::String> type = v->TypeOf(v8::Isolate::GetCurrent());
	return std::string(*v8::String::Utf8Value(type));
}

