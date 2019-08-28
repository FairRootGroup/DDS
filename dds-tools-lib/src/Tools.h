// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLS_H
#define DDS_TOOLS_H

// DDS
#include "ToolsProtocol.h"
// STD
#include <iostream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>

namespace dds
{
    namespace tools_api
    {
        /**
         *
         * \brief The main class of the Tools API. It represets a DDS session.
         * \details
         *        It can be used to create new DDS sessions or attach to existing ones.
         *        Also, this class can be used to send and recieve Tools commands.
         *        Currently the following commands are \link ToolsProtocol.h \endlink
         *
         *        Please note, when you send a requests, server will respond with a corresponding reply with a following
         Done event. But the server can also send an error message. Once you receive either Done or an error, don't
         expect the server to send anything else in the regards of the request. So, you can then stop the API or send
         another request.
         *
         *
         * \par Example1: Create a new DDS session, spawn 10 localhost agents
         * \code
         CSession session;
         // Create a DDS session
         boost::uuids::uuid sessionID = session.create();

         // Create submit request to spawn agents
         SSubmitRequest::request_t submitInfo;
         submitInfo.m_config = "";
         submitInfo.m_rms = "localhost";
         submitInfo.m_instances = "10";
         submitInfo.m_pluginPath = "";
         SSubmitRequest::ptr_t submitRequestPtr = SSubmitRequest::makeRequest(submitInfo);

         // Subscribe on text messages from DDS server
         submitRequestPtr->setMessageCallback([&session](const SMessageResponseData& _message) {
            cout << "Server reports: " << _message.m_msg << endl;
         });

         // Subscribe on Done evens.
         // Server will send Done when there it has finsihed proccessing a corresponding request.
         submitRequestPtr->setDoneCallback([&session, &start]() {
             auto end = chrono::high_resolution_clock::now();
             chrono::duration<double, std::milli> elapsed = end - start;
             cout << "Submission took: " << elapsed.count() << " ms\n";
             session.unblockCurrentThread();
         });

         // Send request to commander
         session.sendRequest<SSubmitRequest>(submitRequestPtr);

         // Start API processor and block.
         // Blocking is optional, if you have your own internal loop in the app to keep the session object alive
         session.blockCurrentThread();
         * \endcode
         *
         *
         * \par Example2: Attach to an existing DDS session and request the number of running agent
         * \code
         CSession session;
         // Attach to a DDS sesion with sessionID = 446b4183-1313-4648-99aa-4f8fae81311c
         session.attach("446b4183-1313-4648-99aa-4f8fae81311c");

         SAgentInfoRequest::ptr_t agentInfoRequestPtr = SAgentInfoRequest::makeRequest();

         // Subscribe on text messages from DDS server
         agentInfoRequestPtr->setMessageCallback([&session](const SMessageResponseData& _message) {
            cout << "Server reports: " << _message.m_msg << endl;
         });

         // Subscribe on Done event.
         // Server sends Done when it has finsihed proccessing the request.
         agentInfoRequestPtr->setDoneCallback([&session]() {
            session.unblockCurrentThread();
         });

         // Subscribe on AgentInfo events
         agentInfoRequestPtr->setResponseCallback([&session](const SAgentInfoRequest::response_t& _info) {
             cout << _info.m_activeAgentsCount << endl;
         });

         // Send request to commander
         session.sendRequest<SAgentInfoRequest>(agentInfoRequestPtr);

         session.blockCurrentThread();
         * \endcode
         *
         *
         * \par Example3: Sync Tools API example
         * \code
         const string topoFile("property_test.xml");
         const std::chrono::seconds timeout(20);
         const std::chrono::milliseconds requestInterval(500);
         const size_t maxRequests(10);

         CSession session;
         boost::uuids::uuid sid = session.create();

         CTopology topo(topoFile);
         size_t numAgents = topo.getRequiredNofAgents();

         SSubmitRequest::request_t submitInfo;
         submitInfo.m_rms = "localhost";
         submitInfo.m_instances = numAgents;
         session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout);

         session.waitForNumAgents<CSession::EAgentState::idle>(numAgents, timeout, requestInterval, maxRequests,
         &std::cout);

         STopologyRequest::request_t topoInfo;
         topoInfo.m_topologyFile = topoFile;
         topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
         session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout);

         session.waitForNumAgents<CSession::EAgentState::idle>(numAgents, timeout, requestInterval, maxRequests,
         &std::cout);

         session.shutdown();
         * \endcode
         *
         */
        class CSession
        {
          public:
            enum class EAgentState
            {
                active = 0,
                idle,
                executing
            };

            /// \brief Constructor of a DDS Session class.
            CSession();
            /// \brief A destructor
            ~CSession();
            /// \brief Creates a new DDS session
            boost::uuids::uuid create();
            /// \brief Attaches to an existing DDS session
            /// \param[in] _sid A destination DDS session ID
            void attach(const std::string& _sid);
            /// \brief Attaches to an existing DDS session
            /// \param[in] _sid A destination DDS session ID
            void attach(const boost::uuids::uuid& _sid);
            /// \brief Shutdown currently attached DDS session
            void shutdown();
            /// \brief Check if DDS session is running
            bool IsRunning() const;
            /// \brief Returns DDS session ID
            /// \return DDS session ID
            boost::uuids::uuid getSessionID() const;
            /// \brief blockCurrentThread Blocks current thread.
            ///
            /// The function stops the thread and waits until one of the conditions is applied:
            /// 1. 10 minutes timeout;
            /// 2. Failed connection to DDS commander or disconnection from DDS commander;
            /// 3. Explicit call of stop() function.
            ///
            /// \note If there are no subscribers function doesn't wait.
            void blockCurrentThread();
            /// \brief Stop DDS session
            void unblockCurrentThread();

            /// \brief Sends the async request to DDS commander
            /// \param[in] _request Request object. If _request is nullptr than throws std::runtime_error
            template <class T>
            void sendRequest(typename T::ptr_t _request);

            /// \brief Sends the sync request to DDS commander.
            /// \param[in] _requestData Request data object.
            /// \param[in] _timeout Timeout in seconds. Timeout of 0 means no timeout is applied (default).
            /// \param[in] _out Pointer to output stream. nullptr means no output to stream (default).
            /// \throw std::runtime_error
            template <class Request_t>
            void syncSendRequest(const typename Request_t::request_t& _requestData,
                                 const std::chrono::seconds& _timeout = std::chrono::seconds(0),
                                 std::ostream* _out = nullptr);

            /// \brief Sends the sync request to DDS commander.
            /// \param[in] _requestData Request data object.
            /// \param[out] _responseData Response data object.
            /// \param[in] _timeout Timeout in seconds. Timeout of 0 means no timeout is applied (default).
            /// \param[in] _out Pointer to output stream. nullptr means no output to stream (default).
            /// \throw std::runtime_error
            template <class Request_t>
            void syncSendRequest(const typename Request_t::request_t& _requestData,
                                 typename Request_t::response_t& _responseData,
                                 const std::chrono::seconds& _timeout = std::chrono::seconds(0),
                                 std::ostream* _out = nullptr);

            /// \brief Sends the sync request to DDS commander.
            /// \param[in] _requestData Request data object.
            /// \param[out] _responseDataVector Vector of response data object.
            /// \param[in] _timeout Timeout in seconds. Timeout of 0 means no timeout is applied (default).
            /// \param[in] _out Pointer to output stream. nullptr means no output to stream (default).
            /// \throw std::runtime_error
            template <class Request_t>
            void syncSendRequest(const typename Request_t::request_t& _requestData,
                                 typename Request_t::responseVector_t& _responseDataVector,
                                 const std::chrono::seconds& _timeout = std::chrono::seconds(0),
                                 std::ostream* _out = nullptr);

            /// \brief Wait for the required number of agents with a certain state.
            /// \param[in] _numAgents Required number of agents. Must be > 0.
            /// \param[in] _timeout Timeout per each request in seconds. Timeout of 0 means no timeout is applied
            /// (default).
            /// \param[in] _requestInterval Interval between SAgentCountRequest requests in milliseconds.
            /// \param[in] _maxRequests Maximum number of requests. 0 means no limits (default).
            /// \param[in] _out Pointer to output stream. nullptr means no output to stream (default).
            /// \throw std::runtime_error
            template <EAgentState _state>
            void waitForNumAgents(size_t _numAgents,
                                  const std::chrono::seconds& _timeout = std::chrono::seconds(0),
                                  const std::chrono::milliseconds& _requestInterval = std::chrono::milliseconds(500),
                                  size_t _maxRequests = 0,
                                  std::ostream* _out = nullptr);

          private:
            /// \brief Subscribe to custom commands.
            void subscribe();
            /// \brief Checks if DDS is available.
            /// \return True if DDS is available, otherwise False
            bool isDDSAvailable() const;

            void notify(std::istream& _stream);

            // Map request ID to the actual request object
            typedef std::map<requestID_t, boost::any> requests_t;

            template <class T>
            void processRequest(requests_t::mapped_type _request,
                                const boost::property_tree::ptree::value_type& _child,
                                std::function<void(typename T::ptr_t)> _processResponseCallback);

            struct SImpl;
            std::shared_ptr<SImpl> m_impl;
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLS_H */
