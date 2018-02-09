#pragma once

#include <nan.h>

#include "nodeobs_display.h"
#include "nodeobs_api.h"

using namespace v8;

class OBS_content
{
public:
	static void OBS_content_createDisplay(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_destroyDisplay(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_getDisplayPreviewOffset(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_getDisplayPreviewSize(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_createSourcePreviewDisplay(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_resizeDisplay(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_moveDisplay(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setPaddingSize(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setPaddingColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setBackgroundColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setOutlineColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setGuidelineColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setResizeBoxOuterColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setResizeBoxInnerColor(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setShouldDrawUI(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_selectSource(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_selectSources(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_dragSelectedSource(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_loadConfigFile(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_saveIntoConfigFile(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_getSourceFlags(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_sourceSetMuted(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_isSourceMuted(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_getSourceVisibility(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setSourceVisibility(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_fillTabScenes(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_getDrawGuideLines(const FunctionCallbackInfo<Value> &args);
	static void OBS_content_setDrawGuideLines(const FunctionCallbackInfo<Value> &args);

	void DisplayCallback(OBS::Display *dp, uint32_t cx, uint32_t cy);
};
