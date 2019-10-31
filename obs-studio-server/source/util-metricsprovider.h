/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>

#undef strtoll  
#include "nlohmann/json.hpp"

namespace util
{
    class MetricsProvider
    {
        const static int StringSize = 64;

        enum class MessageType
        {
            Pid,
            Tag,
            Status,
            Blame, 
            Shutdown
        };

        struct MetricsMessage
        {
            MessageType type;
            char        param1[StringSize];
            char        param2[StringSize];
        };

        public:
        ~MetricsProvider();

        // Connect this client with the crash handler pipe indicated by 'pipe_name', optionally
        // you can pass the current SLOBS version (parameter 'current_version') and set if the
        // IPC messages should be send synchronously or asynchronously (parameter 'send_messages_async')
        bool Initialize(std::string pipe_name, std::string current_version = "unknown", bool send_messages_async = true);

        // Send an status to the crash handler, if SLOBS crashes unexpectedly this status will be
        // the same of the report.
        // Blaming someone (see below) will ignore any status sent.
        void SendStatus(std::string status);
        
        // Blame methods will make a report be generated even if SLOBS closes successfully, they
        // are a good choice when some handled error happened but the log is still wanted.
        // They do not prevent a real report from being generated even if we blamed someone, if
        // SLOBS fails to shutdown correctly the correct report will be made on the crash handler
        // and the blame target will be ignored.
        void BlameServer();
        void BlameUser();
        void BlameFrontend();

        private:
        void StartPolling(bool send_messages_async = true);
        void SendTag(std::string tag, std::string value);
        void PrepareMessage(MetricsMessage& message);
        bool SendPipeMessage(MetricsMessage& message);

        private:
        void*                      m_Pipe;
        bool                       m_PipeIsOpen        = false;
        bool                       m_StopPolling       = false;
        bool                       m_SendMessagesAsync = true;
        std::thread                m_PollingThread;
        std::mutex                 m_PollingMutex;
        std::queue<MetricsMessage> m_AsyncData;

        bool m_BlameServer   = false;
        bool m_BlameFrontend = false;
        bool m_BlameUser     = false;

    };

}; // namespace util
