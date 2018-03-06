#include <node.h>

#include "nodeobs_content.h"
#include "nodeobs_module.h"

std::string g_moduleDirectory = "";

using namespace v8;

void replaceAll(std::string &str, const std::string &from,
                const std::string &to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos +=
		      to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value> &args)
{
	v8::String::Utf8Value param0(args[0]->ToString());
	g_moduleDirectory = std::string(*param0);
	replaceAll(g_moduleDirectory, "\\", "/");
}

void nodeobs_init(Local<Object> exports)
{
	// EDIT: Add function to specify actual load directory.
	NODE_SET_METHOD(exports, "SetWorkingDirectory", SetWorkingDirectory);
	// END OF EDIT:

	//OBS_content
	NODE_SET_METHOD(exports, "OBS_content_createDisplay",
	                OBS_content::OBS_content_createDisplay);
	NODE_SET_METHOD(exports, "OBS_content_destroyDisplay",
	                OBS_content::OBS_content_destroyDisplay);
	NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewOffset",
	                OBS_content::OBS_content_getDisplayPreviewOffset);
	NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewSize",
	                OBS_content::OBS_content_getDisplayPreviewSize);
	NODE_SET_METHOD(exports, "OBS_content_createSourcePreviewDisplay",
	                OBS_content::OBS_content_createSourcePreviewDisplay);
	NODE_SET_METHOD(exports, "OBS_content_resizeDisplay",
	                OBS_content::OBS_content_resizeDisplay);
	NODE_SET_METHOD(exports, "OBS_content_moveDisplay",
	                OBS_content::OBS_content_moveDisplay);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingSize",
	                OBS_content::OBS_content_setPaddingSize);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingColor",
	                OBS_content::OBS_content_setPaddingColor);
	NODE_SET_METHOD(exports, "OBS_content_setBackgroundColor",
	                OBS_content::OBS_content_setBackgroundColor);
	NODE_SET_METHOD(exports, "OBS_content_setOutlineColor",
	                OBS_content::OBS_content_setOutlineColor);
	NODE_SET_METHOD(exports, "OBS_content_setGuidelineColor",
	                OBS_content::OBS_content_setGuidelineColor);
	NODE_SET_METHOD(exports, "OBS_content_setResizeBoxOuterColor",
	                OBS_content::OBS_content_setResizeBoxOuterColor);
	NODE_SET_METHOD(exports, "OBS_content_setResizeBoxInnerColor",
	                OBS_content::OBS_content_setResizeBoxInnerColor);
	NODE_SET_METHOD(exports, "OBS_content_selectSource",
	                OBS_content::OBS_content_selectSource);
	NODE_SET_METHOD(exports, "OBS_content_selectSources",
	                OBS_content::OBS_content_selectSources);
	NODE_SET_METHOD(exports, "OBS_content_dragSelectedSource",
	                OBS_content::OBS_content_dragSelectedSource);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI",
	                OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI",
	                OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI",
	                OBS_content::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_getDrawGuideLines",
	                OBS_content::OBS_content_getDrawGuideLines);
	NODE_SET_METHOD(exports, "OBS_content_setDrawGuideLines",
	                OBS_content::OBS_content_setDrawGuideLines);
}
