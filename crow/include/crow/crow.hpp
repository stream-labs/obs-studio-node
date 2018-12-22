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

/*!
 * @file crow.hpp
 * @brief main header for Crow
 */

#ifndef NLOHMANN_CROW_HPP
#define NLOHMANN_CROW_HPP

#include <vector> // vector
#include <future> // future
#include <mutex> // mutex
#include <string> //string
#include "..\thirdparty\json\json.hpp"

using json = nlohmann::json;

/*!
 * @brief namespace for Niels Lohmann
 */
namespace nlohmann
{
/*!
 * @brief a C++ client for Sentry
 */
class crow
{
  public:
    /*!
     * @brief create a client
     *
     * @param[in] dsn the DNS string
     * @param[in] context an optional attributes object
     * @param[in] sample_rate the sample rate (0.0 .. 1.0, default: 1.0)
     * @param[in] install_handlers whether to install a termination handler (default: off)
     *
     * @throw std::invalid_argument if DNS string is invalid
     * @throw std::invalid_argument if context object contains invalid key
     *
     * @note If @a dns is empty, the client is disabled.
     *
     * @note
     * In case @a install_handlers is set to `true` (default), the currently
     * installed termination handler is replaced by a new termination handler
     * that first reports possibly uncaught exceptions and then executes the
     * previously installed termination handler. Note that termination
     * handlers installed after creating this client would override this
     * termination behavior. The termination handler can be installed later
     * with function @ref install_handler().
     *
     * @since 0.0.1
     */
    explicit crow(const std::string& dsn,
				  const std::string& minidumpUrl,
                  const json& context = nullptr,
                  double sample_rate = 1.0,
                  bool install_handlers = false);

    /*!
     * @brief install termination handler to handle uncaught exceptions
     * @post uncaught exceptions are reported prior to executing existing termination handler
     *
     * @since 0.0.3
     */
    void install_handler();

    /*!
     * @name event capturing
     * @{
     */

    /*!
     * @brief capture a message
     *
     * @param[in] message the message to capture
     * @param[in] attributes an optional attributes object
     *
     * @throw std::invalid_argument if context object contains invalid key
     *
     * @since 0.0.1
     */
    void capture_message(const std::string& message,
                         const json& attributes = nullptr);

	/*!
	 * @brief capture a message synchronously
	 *
	 * @param[in] message the message to capture
	 * @param[in] attributes an optional attributes object
	 *
	 * @throw std::invalid_argument if context object contains invalid key
	 *
	 * @since 0.0.1
	 */
	void capture_message_sync(const std::string& message,
		const json& attributes = nullptr);

    /*!
     * @brief capture an exception
     *
     * @param[in] exception the passed exception
     * @param[in] context an optional context object
     * @param[in] handled whether the exception was handled and only reported
     *
     * @throw std::invalid_argument if context object contains invalid key
     *
     * @since 0.0.1, context added with 0.0.3
     */
    void capture_exception(const std::exception& exception,
                           const json& context = nullptr,
                           bool handled = true);

	/*!
	 * @brief capture an exception synchronously
	 *
	 * @param[in] message exception the passed exception
	 * @param[in] context an optional context object
	 * @param[in] handled whether the exception was handled and only reported
	 *
	 * @throw std::invalid_argument if context object contains invalid key
	 *
	 * @since 0.0.1, context added with 0.0.3
	 */
	void capture_exception_sync(const std::string exceptionName,
		const std::string exceptionMessage,
		const std::string exceptionModule,
		json backtrace,
		const json& context = nullptr,
		bool handled = true);

    /*!
     * @brief add a breadcrumb to the current context
     *
     * @param[in] message message for the breadcrumb
     * @param[in] attributes an optional attributes object
     *
     * @since 0.0.1
     */
    void add_breadcrumb(const std::string& message,
                        const json& attributes = nullptr);

    /*!
     * @brief return the id of the last reported event
     *
     * @return event id, or empty string, if no request has been made
     *
     * @since 0.0.2
     */
    std::string get_last_event_id() const;

    /*!
     * @}
     */

    /*!
     * @name context management
     * @{
     */

    /*!
     * @brief return current context
     *
     * @return current context
     *
     * @since 0.0.3
     */
    const json& get_context() const;

	/*!
	 * @brief add custom elements for future events
	 *
	 * @param[in] data data to add to the extra context
	 *
	 * @since 0.0.3
	 */
    void add_custom_context(const std::string tag, const json& data);

    /*!
     * @brief add elements to the "user" context for future events
     *
     * @param[in] data data to add to the extra context
     *
     * @since 0.0.3
     */
    void add_user_context(const json& data);

    /*!
     * @brief add elements to the "tags" context for future events
     *
     * @param[in] data data to add to the extra context
     *
     * @since 0.0.3
     */
    void add_tags_context(const json& data);

    /*!
     * @brief add elements to the "request" context for future events
     *
     * @param[in] data data to add to the extra context
     *
     * @since 0.0.3
     */
    void add_request_context(const json& data);

    /*!
     * @brief add elements to the "extra" context for future events
     *
     * @param[in] data data to add to the extra context
     *
     * @since 0.0.3
     */
    void add_extra_context(const json& data);

    /*!
     * @brief add context information to payload for future events
     *
     * @param[in] context the context to add
     *
     * @note @a context must be an object; allowed keys are "user", "request", "extea", or "tags"
     * @throw std::invalid_argument if context object contains invalid key
     *
     * @since 0.0.3
     */
    void merge_context(const json& context);

    /*!
     * @brief reset context for future events
     *
     * @post context is in the same state as it was after construction
     *
     * @since 0.0.3
     */
    void clear_context();

    /*!
     * @}
     */

	void register_minidump_upload(std::string filepath);
	void register_file_upload(std::string filepath);

  private:
    /*!
     * @brief POST the payload to the Sentry sink URL
     *
     * @param[in] payload payload to send
     * @param[in] synchronous whether the payload should be sent immediately
     * @return result
     */
    std::string post(json payload) const;

    void enqueue_post(bool sync = false);

    /*!
     * @brief termination handler that detects uncaught exceptions
     *
     * @post previously installed termination handler is executed
     *
     * @note The rethrowing of uncaught exceptions does not work with Microsoft Visual Studio 2017, see
     *       https://developercommunity.visualstudio.com/content/problem/135332/stdcurrent-exception-returns-null-in-a-stdterminat.html
     */
    static void new_termination_handler();

  private:
    /// the sample rate (as integer 0..100)
    const int m_sample_rate;

    /// whether the client is enabled
    const bool m_enabled = true;
    /// the public key to be used in requests
    std::string m_public_key;
    /// the secret key to be used in requests
    std::string m_secret_key;
    /// the URL to send events to
    std::string m_store_url;

    /// the payload of all events
    json m_payload = {};
    /// a mutex to make payload thread-safe
    std::mutex m_payload_mutex;

    /// a vector of POST jobs
    mutable std::vector<std::future<std::string>> m_jobs;
    /// a mutex to make m_jobs thread-safe
    mutable std::mutex m_jobs_mutex;
    /// a cache for the last event id
    mutable std::string m_last_event_id = "-1";
    /// whether a post has been made already
    bool m_posts = false;

    /// the termination handler installed before initializing the client
    std::terminate_handler existing_termination_handler = nullptr;
    /// a pointer to the last client (used for termination handling)
    static crow* m_client_that_installed_termination_handler;

    /// the maximal number of running jobs
    static constexpr std::size_t m_maximal_jobs = 10;

	std::string m_minidump_file;
	std::vector<std::string> m_file_uploads;
	std::string m_minidump_url;
};

}

#endif
