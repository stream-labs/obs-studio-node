#pragma once

#include <mutex>
#include <vector>

#include <uv.h>

/* This structure is an adaptation of one used in SQLite Node bindings. 
 * You can find the original here:
 * https://github.com/mapbox/node-sqlite3/blob/master/src/async.h */

namespace osn {

template <class Item, class Parent> class Async {
public:
    typedef void (*Callback)(Parent* parent, Item* item);
    
protected:
    uv_async_t watcher;
    std::mutex mutex;
    std::vector<Item*> data;
    Callback callback;
public:
    Parent* parent;

public:
    Async(Parent* parent_, Callback cb_)
        : callback(cb_), parent(parent_) {
        watcher.data = this;
        uv_async_init(uv_default_loop(), &watcher, reinterpret_cast<uv_async_cb>(listener));
    }

    static void listener(uv_async_t* handle, int status) {
        Async* async = static_cast<Async*>(handle->data);
        std::vector<Item*> rows;

        {
            std::unique_lock<std::mutex> lock(async->mutex);
            rows.swap(async->data);
        }

        for (decltype(rows)::size_type i = 0, size = rows.size(); i < size; i++) {
            async->callback(async->parent, rows[i]);
        }
    }

    static void close(uv_handle_t* handle) {
        assert(handle != NULL);
        assert(handle->data != NULL);
        Async* async = static_cast<Async*>(handle->data);
        delete async;
    }

    void close() {
        listener(&watcher, 0);
        uv_close((uv_handle_t*)&watcher, close);
    }

    void add(Item* item) {
        std::unique_lock<std::mutex> lock(mutex);
        data.push_back(item);
    }

    void send() {
        uv_async_send(&watcher);
    }

    void send(Item* item) {
        add(item);
        send();
    }
};

}