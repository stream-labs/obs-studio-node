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

    class Recording:
        public FileOutput,
        public OutputSignals
    {
        public:
        Recording() : FileOutput(), OutputSignals() {
            videoEncoder = nullptr;
            signals = {
                "start",
                "stop",
                "starting",
                "stopping",
                "wrote"
            };
        }
        virtual ~Recording();

        public:
        obs_encoder_t* videoEncoder;
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
