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
#include "Transition.h"
#include "Properties.h"

NAN_MODULE_INIT(node_initialize)
{
    Nan::SetAccessor(target, FIELD_NAME("status"), osn::status);
    Nan::SetAccessor(target, FIELD_NAME("locale"), osn::locale, osn::locale);
    Nan::SetAccessor(target, FIELD_NAME("version"), osn::version);
    Nan::SetMethod(target, "startup", osn::startup);
    Nan::SetMethod(target, "shutdown", osn::shutdown);
    //Nan::SetMethod(video_ns, "reset", osn::Video::reset);

    /* NOTE: Init order matters here. */
    osn::IEncoder::Init(target);
    osn::Audio::Init(target);
    osn::AudioEncoder::Init(target);
    osn::Video::Init(target);
    osn::VideoEncoder::Init(target);
    osn::Display::Init(target);
    osn::Module::Init(target);
    osn::ISource::Init(target);
    osn::Input::Init(target);
    osn::SceneItem::Init();
    osn::Scene::Init(target);
    osn::Transition::Init(target);
    osn::Properties::Init(target);
    osn::Property::Init(target);
}

NODE_MODULE(obs_node, node_initialize)