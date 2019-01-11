#include <nan.h>
#include <node.h>

namespace display
{
	static void OBS_content_createDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_destroyDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_getDisplayPreviewOffset(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_getDisplayPreviewSize(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_createSourcePreviewDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_resizeDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_moveDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setPaddingSize(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setPaddingColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setOutlineColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setGuidelineColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setResizeBoxInnerColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setResizeBoxOuterColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setShouldDrawUI(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_selectSource(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setDrawGuideLines(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace display
