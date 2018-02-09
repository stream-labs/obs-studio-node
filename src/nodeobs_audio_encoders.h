#include <obs.hpp>

#include <map>

#include "nodeobs_api.h"

const std::map<int, const char *> &GetAACEncoderBitrateMap();
const char *GetAACEncoderForBitrate(int bitrate);
int FindClosestAvailableAACBitrate(int bitrate);