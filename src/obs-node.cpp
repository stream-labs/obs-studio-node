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
#include "Filter.h"
#include "AudioControls.h"
#include "Output.h"
#include "Service.h"

NAN_MODULE_INIT(node_initialize)
{
    /* NOTE: Init order matters here. */
    /* NOTE: Each one adds a corresponding
       constructions function or object to
       the module.exports equivalent */
    osn::Init(target);
    osn::IEncoder::Init(target);
    osn::Audio::Init(target);
    osn::AudioEncoder::Init(target);
    osn::Video::Init(target);
    osn::VideoEncoder::Init(target);
    osn::Display::Init(target);
    osn::Module::Init(target);
    osn::Output::Init(target);
    osn::Service::Init(target);
    osn::ISource::Init(target);
    osn::Input::Init(target);
    osn::SceneItem::Init();
    osn::Scene::Init(target);
    osn::Transition::Init(target);
    osn::Filter::Init(target);
    osn::Properties::Init(target);
    osn::Property::Init(target);

    osn::VolmeterCallback::Init(target);
    osn::Volmeter::Init(target);
    osn::FaderCallback::Init(target);
    osn::Fader::Init(target);
}

NODE_MODULE(obs_node, node_initialize)