#pragma once

#include <http_request_params.hpp>
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <ctime>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace http_client
{
    /// @brief Interface for HTTP client implementations
    ///
    /// This interface provides a standard way of performing HTTP requests and
    /// retrieving the response.
    class IHttpClient
    {
    public:
        /// @brief Destroys the IHttpClient
        virtual ~IHttpClient() = default;

        /// @brief Create an HTTP request
        /// @param params The parameters for the request
        /// @return The created request
        virtual boost::beast::http::request<boost::beast::http::string_body>
        CreateHttpRequest(const HttpRequestParams& params) = 0;

        /// @brief Coroutine to perform an HTTP request
        /// @param token A shared pointer to the authentication token
        /// @param params The parameters for the request
        /// @param messageGetter Function to retrieve messages
        /// @param onUnauthorized Action to take on unauthorized access
        /// @param connectionRetry Time to wait before retrying the connection
        /// @param batchSize The minimum number of bytes of messages to batch
        /// @param onSuccess Action to take on successful request
        /// @param loopRequestCondition Condition to continue the request loop
        /// @return Awaitable task for the HTTP request
        virtual boost::asio::awaitable<void> Co_PerformHttpRequest(
            std::shared_ptr<std::string> token,
            HttpRequestParams params,
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> messageGetter,
            std::function<void()> onUnauthorized,
            std::time_t connectionRetry,
            size_t batchSize,
            std::function<void(const int, const std::string&)> onSuccess = {},
            std::function<bool()> loopRequestCondition = {}) = 0;

        /// @brief Perform an HTTP request and receive the response
        /// @param params The parameters for the request
        /// @return The response
        virtual boost::beast::http::response<boost::beast::http::dynamic_body>
        PerformHttpRequest(const HttpRequestParams& params) = 0;

        /// @brief Authenticate with a UUID and key
        /// @param serverUrl The URL of the server
        /// @param userAgent The user agent header
        /// @param uuid The UUID
        /// @param key The key
        /// @param verificationMode The verification mode
        /// @return The authentication token
        virtual std::optional<std::string> AuthenticateWithUuidAndKey(const std::string& serverUrl,
                                                                      const std::string& userAgent,
                                                                      const std::string& uuid,
                                                                      const std::string& key,
                                                                      const std::string& verificationMode) = 0;

        /// @brief Authenticate with a user and password
        /// @param serverUrl The URL of the server
        /// @param userAgent The user agent header
        /// @param user The user
        /// @param password The password
        /// @param verificationMode The verification mode
        /// @return The authentication token
        virtual std::optional<std::string> AuthenticateWithUserPassword(const std::string& serverUrl,
                                                                        const std::string& userAgent,
                                                                        const std::string& user,
                                                                        const std::string& password,
                                                                        const std::string& verificationMode) = 0;
    };
} // namespace http_client
