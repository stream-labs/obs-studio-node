/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include <obs.h>
#include "utility.hpp"
#include "osn-streaming.hpp"
#include "osn-file-output.hpp"
#include "osn-output-signals.hpp"

namespace osn
{
	enum RecQuality {
		Stream = 0,
		HighQuality = 1,
		HigherQuality = 2,
		Lossless = 3
	};

    class Recording:
		public FileOutput,
		public OutputSignals
    {
        public:
        Recording() : FileOutput(), OutputSignals() {
			videoEncoder = nullptr;
			audioEncoder = nullptr;
			signals = {
				"start",
				"stop",
				"starting",
				"stopping",
                "wrote"
			};
			rescaling = false;
			outputWidth = 1280;
			outputHeight = 720;
			quality = RecQuality::Stream;
			lowCPU = false;
			mixer = 1 << 0;
			useStreamEncoders = true;
		}
        virtual ~Recording();

        public:
		obs_encoder_t* videoEncoder;
		obs_encoder_t* audioEncoder;
		RecQuality quality;
		bool lowCPU;
		uint32_t mixer;

		bool rescaling;
		uint32_t outputWidth;
		uint32_t outputHeight;
		bool useStreamEncoders;
    };

	class IRecording: public IFileOutput
	{
		public:
		static void GetVideoEncoder(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
		static void SetVideoEncoder(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);
		static void Query(
		    void*                          data,
		    const int64_t                  id,
		    const std::vector<ipc::value>& args,
		    std::vector<ipc::value>&       rval);

		static std::string GenerateSpecifiedFilename(
			const std::string& extension, bool noSpace, const std::string& format);
		static void FindBestFilename(std::string& strPath, bool noSpace);

		static obs_encoder_t* duplicate_encoder(obs_encoder_t* src, uint64_t trackIndex = 0);
	};
}
