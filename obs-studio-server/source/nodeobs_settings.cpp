#include <windows.h>
#include "nodeobs_settings.h"
#include "error.hpp"
#include "shared.hpp"

vector<const char*> tabStreamTypes;
const char* currentServiceName;
bool useAdvancedOutput;

namespace fb = flatbuffers;

/* some nice default output resolution vals */
static const double vals[] =
{
	1.0,
	1.25,
	(1.0 / 0.75),
	1.5,
	(1.0 / 0.6),
	1.75,
	2.0,
	2.25,
	2.5,
	2.75,
	3.0
};

static const size_t numVals = sizeof(vals) / sizeof(double);

static string ResString(uint32_t cx, uint32_t cy)
{
	ostringstream res;
	res << cx << "x" << cy;
	return res.str();
}


void OBS_settings::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls =
		std::make_shared<ipc::collection>("Settings");

	cls->register_function(
		std::make_shared<ipc::function>(
			"OBS_settings_getSettings",
			std::vector<ipc::type>{ ipc::type::String },
			OBS_settings_getSettings
		)
	);

	cls->register_function(
		std::make_shared<ipc::function>(
			"OBS_settings_saveSettings",
			std::vector<ipc::type>{
				ipc::type::String,
				ipc::type::UInt32,
				ipc::type::UInt32,
				ipc::type::Binary
			},
			OBS_settings_saveSettings
		)
	);

	cls->register_function(
		std::make_shared<ipc::function>(
			"OBS_settings_getListCategories",
			std::vector<ipc::type>{},
			OBS_settings_getListCategories
		)
	);

	srv.register_collection(cls);
}

void OBS_settings::OBS_settings_getListCategories(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	for (auto &type : categories) {
		rval.push_back(ipc::value(type.c_str()));
	}

	AUTO_DEBUG;
}

void OBS_settings::OBS_settings_getSettings(
  void* data,
  const int64_t id,
  const std::vector<ipc::value>& args,
  std::vector<ipc::value>& rval
) {
}

static void saveGeneral(Category *category)
{
	
}

static void saveStream(Category *category)
{

}

static void saveOutput(Category *category)
{

}

static void saveAudio(Category *category)
{

}

static void saveVideo(Category *category)
{

}

static void saveHotkeys(Category *category)
{

}

static void saveAdvanced(Category *category)
{

}

void OBS_settings::OBS_settings_saveSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
{
	/* Here, we build a map that maps the string to a function. This isn't quite
	 * like a jump table but I find it a bit more readable.
	 * Unfortunately, we can't use overloading data here since we don't
	 * have enough meta data to accomplish that. This is a good reason why
	 * "categories" shouldn't exist here. In addition, everything is hardcoded
	 * such as parameter position and order of elements so everything is treated
	 * differently making this infinitely harder. */
	static const std::map<std::string, std::function<void(Category*)> categories {
		{ "General", saveGeneral },
		{ "Stream", saveStream },
		{ "Output", saveOutput },
		{ "Audio", saveAudio },
		{ "Video",  saveVideo },
		{ "Hotkeys", saveHotkeys },
		{ "Advanced", saveAdvanced }
	};

	/* We're sent binary data here, formatted with flatbuffers */
	const no::Category *category = no::GetCategory(args[0].value_bin.data());

	auto iter = categories.find(category->name().str());

	if (iter != categories.end()) {
		iter->second(category);
	}

	/* FIXME Does input validation need to happen here? */

	AUTO_DEBUG;
}

static bool EncoderAvailable(const char *encoder)
{
	const char *val;
	int i = 0;

	while (obs_enum_encoder_types(i++, &val)) {
		if (strcmp(val, encoder) == 0)
			return true;
	}

	return false;
}

void OBS_settings::getSimpleAvailableEncoders(std::vector<std::pair<std::string, std::string>> *streamEncoder)
{
	streamEncoder->push_back(std::make_pair("Software (x264)", SIMPLE_ENCODER_X264));

	if (EncoderAvailable("ffmpeg_nvenc"))
		streamEncoder->push_back(std::make_pair("NVENC", SIMPLE_ENCODER_NVENC));

	if (EncoderAvailable("amd_amf_h264"))
		streamEncoder->push_back(std::make_pair("AMD", SIMPLE_ENCODER_AMD));
}

void OBS_settings::getAdvancedAvailableEncoders(std::vector<std::pair<std::string, std::string>> *streamEncoder)
{
	streamEncoder->push_back(std::make_pair("Software (x264)", ADVANCED_ENCODER_X264));

	/*if (EncoderAvailable("obs_qsv11"))
		streamEncoder->push_back(std::make_pair("QSV", ADVANCED_ENCODER_QSV));*/

	if (EncoderAvailable("ffmpeg_nvenc"))
		streamEncoder->push_back(std::make_pair("NVENC", ADVANCED_ENCODER_NVENC));

	if (EncoderAvailable("amd_amf_h264"))
		streamEncoder->push_back(std::make_pair("AMD", ADVANCED_ENCODER_AMD));
}

std::vector<pair<uint32_t, uint32_t>> OBS_settings::getOutputResolutions(int base_cx, int base_cy)
{
	std::vector<pair<uint32_t, uint32_t>> outputResolutions;
	for (size_t idx = 0; idx < numVals; idx++) {
		uint32_t outDownscaleCX = uint32_t(double(base_cx) / vals[idx]);
		uint32_t outDownscaleCY = uint32_t(double(base_cy) / vals[idx]);

		outDownscaleCX &= 0xFFFFFFFE;
		outDownscaleCY &= 0xFFFFFFFE;

		outputResolutions.push_back(std::make_pair(outDownscaleCX, outDownscaleCY));
	}
	return outputResolutions;
}

struct BaseLexer {
	lexer lex;
public:
	inline BaseLexer() { lexer_init(&lex); }
	inline ~BaseLexer() { lexer_free(&lex); }
	operator lexer*() { return &lex; }
};

// parses "[width]x[height]", string, i.e. 1024x768

static bool ConvertResText(const char *res, uint32_t &cx, uint32_t &cy)
{
	BaseLexer lex;
	base_token token;

	lexer_start(lex, res);

	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	if (token.type != BASETOKEN_DIGIT)
		return false;

	cx = std::stoul(token.text.array);

	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	if (strref_cmpi(&token.text, "x") != 0)
		return false;

	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	cy = std::stoul(token.text.array);

	// shouldn't be any more tokens after this
	if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	return true;
}
