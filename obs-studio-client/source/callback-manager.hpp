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

#include <mutex>
#include <napi.h>
#include <thread>
#include <map>
#include "utility-v8.hpp"
#ifdef WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

struct SourceSizeInfo
{
	std::string name;
	uint32_t    width;
	uint32_t    height;
	uint32_t    flags;
};

struct SourceSizeInfoData
{
	std::vector<SourceSizeInfo*> items;
};

extern const char* source_sem_name;
#ifdef WIN32
extern HANDLE source_sem;
#else
extern sem_t *source_sem;
#endif

namespace sourceCallback
{
	class Worker: public Napi::AsyncWorker
    {
        public:
        std::shared_ptr<SourceSizeInfoData> data = nullptr;

        public:
        Worker(Napi::Function& callback) : AsyncWorker(callback){};
        virtual ~Worker() {};

        void Execute() {
            if (!data)
                SetError("Invalid signal object");
        };
        void OnOK() {
            Napi::Array result = Napi::Array::New(Env(), data->items.size());

			for (size_t i = 0; i < data->items.size(); i++) {
				Napi::Object obj = Napi::Object::New(Env());
				obj.Set("name", Napi::String::New(Env(), data->items[i]->name));
				obj.Set("width", Napi::Number::New(Env(), data->items[i]->width));
				obj.Set("height", Napi::Number::New(Env(), data->items[i]->height));
				obj.Set("flags", Napi::Number::New(Env(), data->items[i]->flags));
				result.Set(i, obj);
			}

            Callback().Call({ result });
			release_semaphore(source_sem);
        };
		void SetData(std::shared_ptr<SourceSizeInfoData> new_data) {
			data = new_data;
		};
    };

	extern bool isWorkerRunning;
	extern bool worker_stop;
	extern uint32_t sleepIntervalMS;
	extern Worker* asyncWorker;
	extern std::thread* worker_thread;
	extern std::vector<std::thread*> source_queue_task_workers;

	void worker(void);
	void start_worker(void);
	void stop_worker(void);
	void queueTask(std::shared_ptr<SourceSizeInfoData> data);

	void Init(Napi::Env env, Napi::Object exports);

	Napi::Value RegisterSourceCallback(const Napi::CallbackInfo& info);
	Napi::Value RemoveSourceCallback(const Napi::CallbackInfo& info);
}
