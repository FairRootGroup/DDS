// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLS_H
#define DDS_TOOLS_H

// STD
#include <string>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// DDS
#include "CRC.h"
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
         session.start(true);
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

         session.start(true);
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
         session.sendRequest(agentInfo);

         session.start(true);
         * \endcode
         *
         */
        class CSession
        {
          public:
            /// \brief Callback function for message notifications.
            typedef boost::signals2::signal<void(const SMessage&)> signalMessage_t;
            /// \brief Callback function for submit notifications.
            typedef boost::signals2::signal<void(const SDone&)> signalDone_t;
            /// \brief Callback function for progress notifications.
            typedef boost::signals2::signal<void(const SProgress&)> signalProgress_t;
            /// \brief Callback function for submit notifications.
            typedef boost::signals2::signal<void(const SSubmit&)> signalSubmit_t;
            /// \brief Callback function for topology notifications.
            typedef boost::signals2::signal<void(const STopology&)> signalTopology_t;
            /// \brief Callback function for getlog notifications.
            typedef boost::signals2::signal<void(const SGetLog&)> signalGetLog_t;
            /// \brief Callback function for commanderInfo notifications.
            typedef boost::signals2::signal<void(const SCommanderInfo&)> signalCommanderInfo_t;
            /// \brief Callback function for agentInfo notifications.
            typedef boost::signals2::signal<void(const SAgentInfo&)> signalAgentInfo_t;

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
            /// \brief Send requests and listen for notifications.
            /// \brief param[in] _block If true than we stop the main thread.
            ///
            /// If _block is true function stops the thread and waits until one of the conditions is applied:
            /// 1. 10 minutes timeout;
            /// 2. Failed connection to DDS commander or disconnection from DDS commander;
            /// 3. Explicit call of stop() function.
            ///
            /// \note If there are no subscribers function doesn't wait.
            void start(bool _block = true);
            void stop();

          public:
            template <class T>
            uint64_t sendRequest(SBaseMessageImpl<T>& _msg)
            {
                const boost::uuids::uuid id = boost::uuids::random_generator()();
                std::stringstream strid;
                strid << id;
                const uint64_t requestID = MiscCommon::crc64(strid.str());
                _msg.setRequestID(requestID);
                m_customCmd.send(_msg.toJSON(), dds::intercom_api::g_sToolsAPISign);
                return requestID;
            }

            /// \brief Subscribe for message notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SMessage, T>::value>::type* = nullptr>
            void onResponse(typename signalMessage_t::slot_function_type _subscriber)
            {
                m_signalMessage.connect(_subscriber);
            }

            /// \brief Subscribe for done notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SDone, T>::value>::type* = nullptr>
            void onResponse(typename signalDone_t::slot_function_type _subscriber)
            {
                m_signalDone.connect(_subscriber);
            }

            /// \brief Subscribe for progress notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SProgress, T>::value>::type* = nullptr>
            void onResponse(typename signalProgress_t::slot_function_type _subscriber)
            {
                m_signalProgress.connect(_subscriber);
            }

            /// \brief Subscribe for submit notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SSubmit, T>::value>::type* = nullptr>
            void onResponse(typename signalSubmit_t::slot_function_type _subscriber)
            {
                m_signalSubmit.connect(_subscriber);
            }

            /// \brief Subscribe for topology notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<STopology, T>::value>::type* = nullptr>
            void onResponse(typename boost::signals2::signal<void(const T&)>::slot_function_type _subscriber)
            {
                m_signalTopology.connect(_subscriber);
            }

            /// \brief Subscribe for getlog notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SGetLog, T>::value>::type* = nullptr>
            void onResponse(typename boost::signals2::signal<void(const T&)>::slot_function_type _subscriber)
            {
                m_signalGetLog.connect(_subscriber);
            }

            /// \brief Subscribe for commanderInfo notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SCommanderInfo, T>::value>::type* = nullptr>
            void onResponse(typename boost::signals2::signal<void(const T&)>::slot_function_type _subscriber)
            {
                m_signalCommanderInfo.connect(_subscriber);
            }

            /// \brief Subscribe for agentInfo notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            template <class T, typename std::enable_if<std::is_same<SAgentInfo, T>::value>::type* = nullptr>
            void onResponse(typename boost::signals2::signal<void(const T&)>::slot_function_type _subscriber)
            {
                m_signalAgentInfo.connect(_subscriber);
            }

          protected:
            /// \brief Parse the input stream and notify subscribers.
            /// \param[in] _stream Stream with JSON data from DDS commander.
            void notify(std::istream& _stream);

          private:
            bool isDDSAvailable() const;

          private:
            boost::uuids::uuid m_sid;                      ///< Session ID
            dds::intercom_api::CIntercomService m_service; ///< Intercom service.
            dds::intercom_api::CCustomCmd m_customCmd; ///< Custom commands API. Used for communication with commander.

            signalMessage_t m_signalMessage;             ///< Message signal.
            signalDone_t m_signalDone;                   ///< Done signal.
            signalProgress_t m_signalProgress;           ///< Progress signal.
            signalSubmit_t m_signalSubmit;               ///< Submit signal.
            signalTopology_t m_signalTopology;           ///< Topology signal.
            signalGetLog_t m_signalGetLog;               ///< GetLog signal.
            signalCommanderInfo_t m_signalCommanderInfo; ///< CommanderInfo singal
            signalAgentInfo_t m_signalAgentInfo;         ///< AgentInfo singal
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLS_H */
