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

#ifndef NLOHMANN_CROW_LOGGERS_HPP
#define NLOHMANN_CROW_LOGGERS_HPP

/*!
 * @file loggers.hpp
 * @brief common types for logger integrations
 */

namespace nlohmann
{
/*!
 * @brief integrations of Crow into other frameworks
 */
namespace crow_integrations
{

/*!
 * @brief different actions to be executed for a log event
 * @since 0.0.4
 */
enum log_action
{
    message_fatal,       ///< send a message with level "fatal"
    message_error,       ///< send a message with level "error"
    message_warning,     ///< send a message with level "warning"
    message_info,        ///< send a message with level "info"
    message_debug,       ///< send a message with level "debug"
    breadcrumb_fatal,    ///< add a breadcrumb with level "fatal"
    breadcrumb_error,    ///< add a breadcrumb with level "error"
    breadcrumb_warning,  ///< add a breadcrumb with level "warning"
    breadcrumb_info,     ///< add a breadcrumb with level "info"
    breadcrumb_debug,    ///< add a breadcrumb with level "debug"
    ignore               ///< do nothing
};

/*!
 * @brief get Sentry level for each log action

 * @param action log action
 * @return string representation of Sentry level

 * @since 0.0.4
 */
const char* log_action_level(const log_action action) noexcept
{
    switch (action)
    {
        case message_fatal:
        case breadcrumb_fatal:
            return "fatal";
        case message_error:
        case breadcrumb_error:
            return "error";
        case message_warning:
        case breadcrumb_warning:
            return "warning";
        case message_info:
        case breadcrumb_info:
            return "info";
        case message_debug:
        case breadcrumb_debug:
            return "debug";
        default:
            return "";
    }
}

}
}

#endif
