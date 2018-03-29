#include <node.h>
#include <nan.h>

namespace display {
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_createDisplay(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_destroyDisplay(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_getDisplayPreviewOffset(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_getDisplayPreviewSize(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_createSourcePreviewDisplay(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_resizeDisplay(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_moveDisplay(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setPaddingSize(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setPaddingColor(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setOutlineColor(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setGuidelineColor(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setResizeBoxInnerColor(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setResizeBoxOuterColor(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setShouldDrawUI(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_selectSource(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_selectSources(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_dragSelectedSource(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_getDrawGuideLines(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_content_setDrawGuideLines(Nan::NAN_METHOD_ARGS_TYPE info);
}