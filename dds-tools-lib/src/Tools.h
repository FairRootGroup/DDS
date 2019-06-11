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
         Done event. But the server can also send an error message. Once you receive wither Done or an error, don't
         expect the server to send anything else in the regards of the request. So, you can then stop the API or send
         another request.
         *
         *
         * \par Example1: Create a new DDS session, spawn 10 localhost agents
         * \code
         CSession session;
         // Create a DDS session
         boost::uuids::uuid sessionID = session.creat();

         // Subscribe on text messages from DDS server
         session.onResponse<SMessage>([&session](const SMessage& _message) {
                cout << "Server reports: " << _message.m_msg << endl;

                // stop communication on errors
                if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
                    session.stop();
                return true;
         });

         // Subscribe on Done evens.
         // Server will send Done when there it has finsihed proccessing a corresponding request.
         // If you have send multiple requests, then you can check for m_requestID of the Done message.
         session.onResponse<SDone>([&session, &start](const SDone& _message) {
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, std::milli> elapsed = end - start;
                cout << "Submission took: " << elapsed.count() << " ms\n";

                session.stop();
                return true;
         });

         // Create a submit request to spawn agents
         SSubmit submitInfo;
         submitInfo.m_config = "";
         submitInfo.m_rms = "localhost";
         submitInfo.m_instances = "10";
         submitInfo.m_pluginPath = "";
         session.sendRequest(submitInfo);

         // Start API processor and block.
         // Blocking is optional, if you have your own internal loop in the app to keep the session object alive
         session.blockCurrentThread();
         * \endcode
         *
         *
         * \par Example2: Attache to an existing DDS session and request the number of running agent
         * \code
         CSession session;
         // Attach to a DDS sesion with sessionID = 446b4183-1313-4648-99aa-4f8fae81311c
         session.attach("446b4183-1313-4648-99aa-4f8fae81311c");

         // Subscribe on text messages from DDS server
         session.onResponse<SMessage>([&session](const SMessage& _message) {
            cout << "Server reports: " << _message.m_msg << endl;

            // stop communication on errors
            if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
                session.stop();
            return true;
         });

         // Subscribe on Done evens.
         // Server will send Done when there it has finsihed proccessing a corresponding request.
         // If you have send multiple requests, then you can check for m_requestID of the Done message.
         session.onResponse<SDone>([&session](const SDone& _message) {
            session.stop();
            return true;
         });

         // Subscribe on AgentInfo events
         session.onResponse<SAgentInfo>([&session, &options](const SAgentInfo& _info) {
                 cout << _info.m_activeAgentsCount << endl;
                 // Close communication channel
                 session.stop();
                 return true;
         });

         // Request information about current agents
         SAgentInfo agentInfo;
         session.sendRequest(agentInfo);

         session.blockCurrentThread();
         * \endcode
         *
         *
         *
         * \par Example3: Attache to an existing DDS session and request a full list of agents with details
         * \code
         CSession session;
         // Attach to a DDS sesion with sessionID = 446b4183-1313-4648-99aa-4f8fae81311c
         session.attach("446b4183-1313-4648-99aa-4f8fae81311c");

         // Subscribe on text messages from DDS server
         session.onResponse<SMessage>([&session](const SMessage& _message) {
         cout << "Server reports: " << _message.m_msg << endl;

         // stop communication on errors
         if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
            session.stop();
            return true;
         });

         // Subscribe on Done evens.
         // Server will send Done when there it has finsihed proccessing a corresponding request.
         // If you have send multiple requests, then you can check for m_requestID of the Done message.
         session.onResponse<SDone>([&session](const SDone& _message) {
                session.stop();
                return true;
         });

         // Subscribe on AgentInfo events
         session.onResponse<SAgentInfo>([&session](const SAgentInfo& _info) {
                if (!_info.m_agentInfo.empty())
                {
                    // std::lock_guard<std::mutex> lock(mutexCounter);
                    // ++nCounter;
                    cout << _info.m_agentInfo << endl;
                }
                else
                    session.stop();

                return true;
         });

         // Request information about current agents
         SAgentInfo agentInfo;
         session.blockCurrentThread();

         session.start(true);
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

            void unsubscribe();
            bool IsRunning() const;
            boost::uuids::uuid getSessionID() const;
            /// \brief blockCurrentThread requests and listen for notifications.
            /// \brief param[in] _block If true than we stop the main thread.
            ///
            /// If _block is true function stops the thread and waits until one of the conditions is applied:
            /// 1. 10 minutes timeout;
            /// 2. Failed connection to DDS commander or disconnection from DDS commander;
            /// 3. Explicit call of stop() function.
            ///
            /// \note If there are no subscribers function doesn't wait.
            void blockCurrentThread();
            void stop();

          public:
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
                catch (const std::bad_weak_ptr& /*_we*/)
                {
                    throw std::runtime_error("DDS tools API: User's request object is out of scope");
                }
                catch (const std::exception& _e)
                {
                    throw std::runtime_error("DDS tools API: User's callback exception: " + std::string(_e.what()));
                }
            }

          private:
            boost::uuids::uuid m_sid;                      ///< Session ID
            dds::intercom_api::CIntercomService m_service; ///< Intercom service.
            dds::intercom_api::CCustomCmd m_customCmd; ///< Custom commands API. Used for communication with commander.

            requests_t m_requests;
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLS_H */
