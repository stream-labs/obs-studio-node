/*
 _____ _____ _____ _ _ _
|     | __  |     | | | |  Crow - a Sentry client for C++
|   --|    -|  |  | | | |  version 0.0.6
|_____|__|__|_____|_____|  https://github.com/nlohmann/crow

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2018 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef NLOHMANN_CROW_LOG4CPLUS_HPP
#define NLOHMANN_CROW_LOG4CPLUS_HPP

#include <string>
#include <unordered_map>
#include <crow/crow.hpp>
#include <crow/integrations/loggers.hpp>
#include <log4cplus/log4cplus.h>
#include <log4cplus/clogger.h>

/*!
 * @file log4cplus.hpp
 * @brief interface for the Log4cplus appender
 */

namespace nlohmann
{
namespace crow_integrations
{

/*!
 * @example log4cplus/example.cpp
 */

/*!
 * @brief appender for Log4cplus
 * @since 0.0.4
 */
class log4cplus_appender : public log4cplus::Appender
{
  public:
    /*!
     * @brief create the appender
     * @param[in] client the Crow client to use
     * @param[in] appender_name the name of the appender (used internally of Log4cplus, defaults to "sentry appender")
     * @param[in] config a configuration (see below)
     * @since 0.0.4
     */
    explicit log4cplus_appender(crow& client,
                                const std::string& appender_name = "sentry appender",
                                const std::unordered_map<log4cplus_loglevel_t, log_action>& config = std::unordered_map<log4cplus_loglevel_t, log_action>())
        : m_client(client)
    {
        setName(appender_name);
        m_log_actions[L4CP_FATAL_LOG_LEVEL] = log_action::message_fatal;
        m_log_actions[L4CP_ERROR_LOG_LEVEL] = log_action::message_error;
        m_log_actions[L4CP_WARN_LOG_LEVEL] = log_action::breadcrumb_warning;
        m_log_actions[L4CP_INFO_LOG_LEVEL] = log_action::breadcrumb_info;
        m_log_actions[L4CP_DEBUG_LOG_LEVEL] = log_action::breadcrumb_debug;
        m_log_actions[L4CP_TRACE_LOG_LEVEL] = log_action::breadcrumb_debug;

        for (const auto& config_entry : config)
        {
            m_log_actions[config_entry.first] = config_entry.second;
        }
    }

    /*!
     * @brief clean up appender state
     * @since 0.0.4
     */
    ~log4cplus_appender() override
    {
        // properly clean up appender
        destructorImpl();
    }

    /*!
     * @brief closing the appender
     * @since 0.0.4
     */
    void close() override {};

  protected:
    /*!
     * @brief the actual appender implementation
     * @param[in] event log event to append
     * @since 0.0.4
     */
    void append(const log4cplus::spi::InternalLoggingEvent& event) override
    {
        // look up action in configuration or use default action
        const auto lookup_action = m_log_actions.find(event.getLogLevel());
        const log_action action = (lookup_action != m_log_actions.end())
                                  ? lookup_action->second
                                  : message_warning;

        switch (action)
        {
            case message_fatal:
            case message_error:
            case message_warning:
            case message_info:
            case message_debug:
            {
                m_client.capture_message(event.getMessage(),
                {
                    {"logger", event.getLoggerName()},
                    {"level", log_action_level(action)},
                    {
                        "extra", {
                            {"location", event.getFile() + ":" + std::to_string(event.getLine())},
                            {"function", event.getFunction()}
                        }
                    }
                });
                return;
            }

            case breadcrumb_fatal:
            case breadcrumb_error:
            case breadcrumb_warning:
            case breadcrumb_info:
            case breadcrumb_debug:
            {
                m_client.add_breadcrumb(event.getMessage(),
                {
                    {"category", event.getLoggerName()},
                    {"level", log_action_level(action)},
                    {
                        "data", {
                            {"location", event.getFile() + ":" + std::to_string(event.getLine())},
                            {"function", event.getFunction()}
                        }
                    }
                });

                return;
            }

            case ignore:
                return;
        }
    }

  private:
    /// configuration of log actions per log level
    std::unordered_map<log4cplus_loglevel_t, log_action> m_log_actions;
    /// the associated Sentry client
    crow& m_client;
};


}
}

#endif
