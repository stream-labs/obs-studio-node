#pragma once

#define SIGNAL_BINDING(name) \
    void name##(const v8::FunctionCallbackInfo<v8::Value> &args)

SIGNAL_BINDING(OBS_signal_sourceRemoved);
SIGNAL_BINDING(OBS_signal_sourceDestroyed);
SIGNAL_BINDING(OBS_signal_sourceSaved);
SIGNAL_BINDING(OBS_signal_sourceLoaded);
SIGNAL_BINDING(OBS_signal_sourceActivated);
SIGNAL_BINDING(OBS_signal_sourceDeactivated);
SIGNAL_BINDING(OBS_signal_sourceShown);
SIGNAL_BINDING(OBS_signal_sourceHidden);
SIGNAL_BINDING(OBS_signal_sourceMuted);

SIGNAL_BINDING(OBS_signal_createdSource);
SIGNAL_BINDING(OBS_signal_removedSource);
SIGNAL_BINDING(OBS_signal_destroyedSource);
SIGNAL_BINDING(OBS_signal_savedSource);
SIGNAL_BINDING(OBS_signal_loadedSource);
SIGNAL_BINDING(OBS_signal_activatedSource);
SIGNAL_BINDING(OBS_signal_deactivatedSource);
SIGNAL_BINDING(OBS_signal_showedSource);
SIGNAL_BINDING(OBS_signal_hidSource);

SIGNAL_BINDING(OBS_signal_outputStarted);
SIGNAL_BINDING(OBS_signal_outputStopped);
SIGNAL_BINDING(OBS_signal_outputStarting);
SIGNAL_BINDING(OBS_signal_outputStopping);
SIGNAL_BINDING(OBS_signal_outputActivated);
SIGNAL_BINDING(OBS_signal_outputDeactivated);
SIGNAL_BINDING(OBS_signal_outputReconnecting);
SIGNAL_BINDING(OBS_signal_outputReconnected);