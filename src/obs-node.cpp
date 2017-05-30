#include "obspp/obspp.hpp"
#include <nan.h>

#include "Common.h"
#include "Module.h"
#include "Input.h"
#include "Global.h"
#include "Display.h"
#include "Audio.h"
#include "Video.h"
#include "Scene.h"
#include "SceneItem.h"

#include <sstream>
#include <iostream>

NAN_MODULE_INIT(node_initialize)
{
    using namespace osn;

    auto global_ns = Nan::New<v8::Object>();
    
    Nan::SetAccessor(global_ns, FIELD_NAME("status"), osn::status);
    Nan::SetAccessor(global_ns, FIELD_NAME("locale"), osn::locale, osn::locale);
    Nan::SetAccessor(global_ns, FIELD_NAME("version"), osn::version);
    Nan::SetMethod(global_ns, "startup", osn::startup);
    Nan::SetMethod(global_ns, "shutdown", osn::shutdown);
    //Nan::SetMethod(video_ns, "reset", osn::Video::reset);

    /* NOTE: Init order matters here. */
    osn::IEncoder::Init(global_ns);
    osn::Audio::Init(global_ns);
    osn::AudioEncoder::Init(global_ns);
    osn::Video::Init(global_ns);
    osn::VideoEncoder::Init(global_ns);
    osn::Display::Init(global_ns);
    osn::Module::Init(global_ns);
    osn::ISource::Init(global_ns);
    osn::Input::Init(global_ns);
    osn::SceneItem::Init();
    osn::Scene::Init(global_ns);

    Nan::Set(target, FIELD_NAME("obs"), global_ns);
}

NODE_MODULE(obs_node, node_initialize)