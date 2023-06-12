#ifndef _API_TEST_HANDLERS_HPP
#define _API_TEST_HANDLERS_HPP

#include <api/api.hpp>
#include <api/catalog/catalog.hpp>
#include <router/router.hpp>
#include <store/drivers/fileDriver.hpp>

namespace api::test::handlers
{

constexpr auto MINIMUM_PRIORITY = 255;                          ///< Minimum priority allowed for a route
constexpr auto MAXIMUM_PRIORITY = 0;                            ///< Maximum priority allowed for a route
constexpr auto TEST_DEFAULT_PROTOCOL_QUEUE {49};                ///< Default protocol queue

constexpr auto DEFAULT_POLICY_FULL_NAME = "policy/wazuh/0";     ///< Default policy full name

constexpr auto SESSIONS_TABLE_NAME = "internal/api_sessions/0"; ///< Name of the sessions table in the store

constexpr auto ASSET_NAME_FIELD_FORMAT = R"("name":"{}")"; ///< JSON name field format, where '{}' is the asset name
constexpr auto FILTER_CONTENT_FORMAT =
    R"({{"name": "{}", "check":[{{"~TestSessionName":"{}"}}]}})";   ///< Filter content format
constexpr auto TEST_FILTER_FULL_NAME_FORMAT = "filter/{}_filter/0"; ///< Filter name format, '{}' is the session name
constexpr auto TEST_POLICY_FULL_NAME_FORMAT = "policy/{}_policy/0"; ///< Policy name format, '{}' is the session name
constexpr auto TEST_ROUTE_NAME_FORMAT = "{}_route";                 ///< Route name format, '{}' is the session name
constexpr auto TEST_EVENT_CONTENT_FORMAT =
    R"({{"wazuh":{{"queue": {}, "location": "{}", "message": "{}"}}, "~TestSessionName": "{}"}})"; ///< Event content
                                                                                                   ///< format
constexpr auto TEST_DEFAULT_PROTOCOL_LOCATION = "api.test";           ///< Default protocol location

constexpr auto TEST_DELETE_SESSIONS_API_CMD = "test.sessions/delete"; ///< API command to delete sessions
constexpr auto TEST_GET_SESSION_DATA_API_CMD = "test.session/get";    ///< API command to get a session data
constexpr auto TEST_GET_SESSIONS_LIST_API_CMD = "test.sessions/get";  ///< API command to get the list of sessions
constexpr auto TEST_POST_SESSION_API_CMD = "test.session/post";       ///< API command to create a new session
constexpr auto TEST_RUN_API_CMD = "test.run/post";                    ///< API command to test an event in a session

/**
 * @brief Test configuration parameters.
 *
 */
struct Config
{
    std::shared_ptr<::router::Router> router;
    std::shared_ptr<catalog::Catalog> catalog;
    std::shared_ptr<store::IStore> store;
};

/**
 * @brief Loads the sessions from a JSON string.
 *
 * @param json JSON string.
 */
std::optional<base::Error> loadSessionsFromJson(const json::Json jsonSessions);

/**
 * @brief Get the sessions as a JSON object.
 *
 * @return json::Json
 */
json::Json getSessionsAsJson(void);

/**
optional<base::Error> saveSessionsToStore(std::shared_ptr<store::IStore> store);

/**
 * @brief Get the minimum available priority for a route.
 *
 * @param router Router instance.
 * @return int32_t Minimum available priority. If no priority is available, returns -1.
 */
inline int32_t getMinimumAvailablePriority(const std::shared_ptr<::router::Router>& router);

/**
 * @brief Add an asset to the catalog.
 *
 * @param catalog Catalog instance.
 * @param assetName Asset name.
 * @param assetContent Asset content.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> addAssetToCatalog(std::shared_ptr<catalog::Catalog> catalog,
                                                    const std::string& assetType,
                                                    const std::string& assetContent);

/**
 * @brief Add the test's filter to the catalog.
 *
 * @param catalog Catalog instance.
 * @param sessionName Session name.
 * @param filterName Filter name.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> addTestFilterToCatalog(std::shared_ptr<catalog::Catalog> catalog,
                                                         const std::string& sessionName,
                                                         const std::string& filterName);

/**
 * @brief Add the test's policy to the catalog.
 *
 * @param catalog Catalog instance.
 * @param sessionName Session name.
 * @param policyName Policy name.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> addTestPolicyToCatalog(std::shared_ptr<catalog::Catalog> catalog,
                                                         const std::string& sessionName,
                                                         const std::string& policyName);

/**
 * @brief Delete a route from the router.
 *
 * @param routeName Route name.
 * @param router Router instance.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> deleteRouteFromRouter(std::shared_ptr<::router::Router> router,
                                                        const std::string& routeName);

/**
 * @brief Delete an asset from the catalog.
 *
 * @param assetName Asset name.
 * @param catalog Catalog instance.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> deleteAssetFromStore(std::shared_ptr<catalog::Catalog> catalog,
                                                       const std::string& assetName);

/**
 * @brief Delete a session and the resources created along with it.
 *
 * @param sessionName Session name.
 * @param router Router instance.
 * @param catalog Catalog instance.
 * @return std::optional<base::Error> If an error occurs, returns the error. Otherwise, returns std::nullopt.
 */
inline std::optional<base::Error> deleteSession(std::shared_ptr<::router::Router> router,
                                                std::shared_ptr<catalog::Catalog> catalog,
                                                const std::string& sessionName);

/**
 * @brief API command handler to get the parameters of an active session.
 *
 * @return api::Handler
 */
api::Handler sessionGet(void);

/**
 * @brief API command handler to create a new session.
 *
 * @param catalog Catalog instance.
 * @param router Router instance.
 * @param store Store instance.
 * @return api::Handler
 */
api::Handler sessionPost(std::shared_ptr<catalog::Catalog> catalog,
                         std::shared_ptr<::router::Router> router,
                         std::shared_ptr<store::IStore> store);

/**
 * @brief API command handler to delete a session or all the sessions.
 *
 * @param catalog Catalog instance.
 * @param router Router instance.
 * @param store Store instance.
 * @return api::Handler
 */
api::Handler sessionsDelete(std::shared_ptr<catalog::Catalog> catalog,
                            std::shared_ptr<::router::Router> router,
                            std::shared_ptr<store::IStore> store);

/**
 * @brief API command handler to get the list of active sessions.
 *
 * @return api::Handler
 */
api::Handler sessionsGet(void);

/**
 * @brief API command handler to test an event in a certain session.
 *
 * @param router Router instance.
 * @return api::Handler
 */
api::Handler runPost(std::shared_ptr<::router::Router> router);

/**
 * @brief Register all handlers for the test API.
 *
 * @param config Test configuration.
 * @param api API instance.
 * @throw std::runtime_error If the command registration fails for any reason and at any
 */
void registerHandlers(const Config& config, std::shared_ptr<api::Api> api);

} // namespace api::test::handlers

#endif // _API_TEST_HANDLERS_HPP
