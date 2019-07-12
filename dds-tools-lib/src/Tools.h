// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLS_H
#define DDS_TOOLS_H

// STD
#include <string>
// BOOST
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
// DDS
#include "ToolsProtocol.h"
#include "dds_intercom.h"

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
             session.stop();
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

         // Subscribe on Done evens.
         // Server will send Done when there it has finsihed proccessing a corresponding request.
         agentInfoRequestPtr->setDoneCallback([&session]() {
            session.stop();
         });

         // Subscribe on AgentInfo events
         agentInfoRequestPtr->setResponseCallback([&session](const SAgentInfoRequest::response_t& _info) {
             cout << _info.m_activeAgentsCount << endl;
             // Close communication channel
             session.stop();
         });

         // Send request to commander
         session.sendRequest<SAgentInfoRequest>(agentInfoRequestPtr);

         session.blockCurrentThread();
         * \endcode
         *
         */
        class CSession
        {
            typedef std::map<requestID_t, boost::any> requests_t;

          public:
            /// \brief Constructor of a DDS Session class.
            /// \param[in] _DDSLocation A full path to DDS directory
            CSession();
            /// \brief A destructor
            ~CSession();

          public:
            /// \brief Creates a new DDS session
            boost::uuids::uuid create();
            /// \brief Attaches to an existing DDS session
            /// \param[in] _sid A destination DDS session ID
            void attach(const std::string& _sid);
            /// \brief Attaches to an existing DDS session
            /// \param[in] _sid A destination DDS session ID
            void attach(const boost::uuids::uuid& _sid);
            /// \brief Shutdow currently attached DDS session
            void shutdown();
            /// \brief Unsibscribe from request notifications
            void unsubscribe();
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
            void stop();

          public:
            /// \brief Sends the request to DDS commander
            /// \param[in] _request Request object. If _request is nullptr than throws std::runtime_error
            template <class T>
            void sendRequest(typename T::ptr_t _request)
            {
                if (!_request)
                    throw std::runtime_error("sendRequest: argument can't be NULL");

                requestID_t reqID = _request->getRequest().m_requestID;
                m_requests.insert(std::make_pair(reqID, _request));

                m_customCmd.send(_request->getRequest().toJSON(), dds::intercom_api::g_sToolsAPISign);
            }

          protected:
            /// \brief Parse the input stream and notify subscribers.
            /// \param[in] _stream Stream with JSON data from DDS commander.
            void notify(std::istream& _stream);

          private:
            /// \brief Subscribe to custom commands.
            void subscribe();
            /// \brief Checks if DDS is available.
            /// \return True if DDS is available, otherwise False
            bool isDDSAvailable() const;

            template <class T>
            void processRequest(requests_t::mapped_type _request,
                                const boost::property_tree::ptree::value_type& _child,
                                std::function<void(typename T::ptr_t)> _processResponseCallback)
            {
                const std::string& tag = _child.first;

                auto request = boost::any_cast<typename T::ptr_t>(_request);
                try
                {
                    if (tag == "done")
                    {
                        request->execDoneCallback();
                    }
                    else if (tag == "message")
                    {
                        SMessageResponseData msg;
                        msg.fromPT(_child.second);
                        request->execMessageCallback(msg);
                    }
                    else if (tag == "progress")
                    {
                        SProgressResponseData progress;
                        progress.fromPT(_child.second);
                        request->execProgressCallback(progress);
                    }
                    else
                    {
                        if (_processResponseCallback)
                            _processResponseCallback(request);
                    }
                }
                catch (const std::exception& _e)
                {
                    throw std::runtime_error("DDS tools API: User's callback exception: " + std::string(_e.what()));
                }
            }

          private:
            boost::uuids::uuid m_sid;                      ///< Session ID.
            dds::intercom_api::CIntercomService m_service; ///< Intercom service.
            dds::intercom_api::CCustomCmd m_customCmd; ///< Custom commands API. Used for communication with commander.
            requests_t m_requests;                     ///< Array of requests.
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLS_H */
