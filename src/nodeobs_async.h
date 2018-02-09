#pragma once

#include <deque>
#include <mutex>

#include <node.h>
#include <nan.h>
#include <uv.h>
#include <v8.h>

namespace obs {

class ForeignWorker {
private:
    uv_async_t *async;

    static void AsyncClose(uv_handle_t *handle) {
        ForeignWorker *worker = 
            reinterpret_cast<ForeignWorker*>(handle->data);

        worker->Destroy();
    }

    static NAUV_WORK_CB(AsyncCallback) {
        ForeignWorker *worker = 
            reinterpret_cast<ForeignWorker*>(async->data);

        worker->Execute();
        uv_close(reinterpret_cast<uv_handle_t*>(async), ForeignWorker::AsyncClose);
    }

protected:
    Nan::Callback *callback;

    v8::Local<v8::Value> Call(int argc = 0, v8::Local<v8::Value> params[] = 0) {
        return callback->Call(argc, params);
    }

public:
    ForeignWorker(Nan::Callback *callback) {
        async = new uv_async_t;

        uv_async_init(
            uv_default_loop()
            , async
            , AsyncCallback
            );

        async->data = this;
        this->callback = callback;
    }

    void Send() {
        uv_async_send(async);
    }

    virtual void Execute() = 0;
    virtual void Destroy() {
        delete this;
    };

    virtual ~ForeignWorker() 
    {
        delete async;
    }
};

struct CallbackInfo {
    CallbackInfo(v8::Local<v8::Function> func) 
        : callback(func), stopped(false)
    {
    }

    Nan::Callback callback;
    bool stopped;
};

template <typename T>
struct CallbackQueue {
    std::mutex mutex;
    std::deque<T> work_queue;
    uv_async_t async;

    CallbackQueue(void(*func)(uv_async_t* handle))
    {
        uv_async_init(uv_default_loop(), &async, func);
    }

    void Signal()
    {
        uv_async_send(&async);
    }
};

}