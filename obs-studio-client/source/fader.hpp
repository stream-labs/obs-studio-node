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
#include <napi.h>
#include "utility.hpp"
#include <thread>

namespace osn
{
	class FaderWorker: public Napi::AsyncWorker
	{
		public:
			FaderWorker(Napi::Function& callback) : AsyncWorker(callback) {};
			virtual ~FaderWorker() {};

			void Execute() {
				// NO V8 / NAPI
			};
			void OnOK() {
				// NODE THREAD
				Callback().Call({Napi::String::New(Env(), "hello"), Napi::String::New(Env(), "world")});
			};
	};
	class Fader : public Napi::ObjectWrap<osn::Fader>
	{
		private:
		static Napi::FunctionReference constructor;
		double value_;
		uint64_t uid;
		Napi::Function cb;
		std::thread* t_worker;

		public:
		uint64_t GetId();

		public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		Fader(const Napi::CallbackInfo& info);

		Napi::Value GetValue(const Napi::CallbackInfo& info);
		static Napi::Value Create(const Napi::CallbackInfo& info);
		Napi::Value RegisterCallback(const Napi::CallbackInfo& info);
		Napi::Value UnregisterCallback(const Napi::CallbackInfo& info);

		// static void Create(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void GetDeziBel(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void SetDezibel(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void GetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void SetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void GetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void SetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void Attach(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void Detach(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void AddCallback(Nan::NAN_METHOD_ARGS_TYPE info);
		// static void RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info);
	};
} // namespace osn
