// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLSPROTOCOLCORE_H
#define DDS_TOOLSPROTOCOLCORE_H

// STD
#include <string>
// BOOST
#include <boost/crc.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// DDS
#include "dds_intercom.h"

/*
{
    "dds": {
        "tools-api": {
            "message": {
                "msg": "string",
                "severity": 123,
                "requestID": 123
            },
            "done": {
                "requestID": 123
            },
            "progress":{
                "completed": 123,
                "total": 123,
                "errors": 123,
                "time": 123,
                "srcCommand": 123
            },
            "submit": {
                "rms": "string",
                "instances": 123,
                "config": "string",
                "pluginPath": "string",
                "requestID": 123
            },
            "topology": {
                "updateType": 123,
                "topologyFile": "string",
                "disableValidation": false,
                "requestID": 123
            },
            "getlog": {
                "requestID": 123
            },
            "commanderInfo":
            {
                "pid":123,
                "idleAgentsCount": 123,
                "requestID": 123
            },
            "agentInfo":
            {
                "activeAgentsCount": 123,
                "index": 123,
                "agentInfo": "string",
                "requestID": 123
            },

        }
    }
}
*/

namespace dds
{
    namespace tools_api
    {
        typedef uint64_t requestID_t;

        template <class T>
        struct SBaseData
        {
            requestID_t m_requestID = 0;

            /// \brief Fill structure into JSON.
            /// \return a JSON-formatted string
            std::string toJSON() const
            {
                boost::property_tree::ptree pt;

                toPT(pt);

                boost::property_tree::ptree ptParent;

                auto parentPtr = static_cast<const T*>(this);
                ptParent.put_child("dds.tools-api." + parentPtr->_protocolTag(), pt);

                std::stringstream json;
                boost::property_tree::write_json(json, ptParent);

                return json.str();
            }

            /// \brief Fill structure into boost's property tree.
            /// \param[out] _pt Property tree with structure details.
            void toPT(boost::property_tree::ptree& _pt) const
            {
                auto parentPtr = static_cast<const T*>(this);

                _pt.put<requestID_t>("requestID", m_requestID);

                return parentPtr->_toPT(_pt);
            }

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void fromPT(const boost::property_tree::ptree& _pt)
            {
                auto parentPtr = static_cast<T*>(this);

                m_requestID = _pt.get<uint64_t>("requestID", 0);

                return parentPtr->_fromPT(_pt);
            }

            /// \brief Equality operator.
            bool operator==(const T& _val) const
            {
                return (m_requestID == _val.m_requestID);
            }
        };

        template <class T>
        struct SBaseRequestData : public SBaseData<T>
        {
        };

        template <class T>
        struct SBaseResponseData : public SBaseData<T>
        {
        };

        struct SEmptyResponseData : SBaseResponseData<SEmptyResponseData>
        {
          private:
            friend SBaseResponseData<SEmptyResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt)
            {
                throw std::runtime_error("DDS Tools API: bad usage of SEmptyResponseData");
            }

            void _toPT(boost::property_tree::ptree& _pt) const
            {
                throw std::runtime_error("DDS Tools API: bad usage of SEmptyResponseData");
            }
            static std::string _protocolTag()
            {
                throw std::runtime_error("DDS Tools API: bad usage of SEmptyResponseData");
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SEmptyResponseData& _val) const
            {
                throw std::runtime_error("DDS Tools API: bad usage of SEmptyRequestData");
            }
        };

        // forward declaration
        struct SDoneResponseData;
        struct SMessageResponseData;
        struct SProgressResponseData;

        template <class TRequest, class TResponse>
        struct SBaseRequestImpl
        {
            using response_t = TResponse;
            using request_t = TRequest;

            typedef std::shared_ptr<SBaseRequestImpl> ptr_t;

            /// \brief Callback function for a Response notification.
            typedef std::function<void(const response_t&)> callbackResponse_t;
            /// \brief Callback function for progress notifications.
            typedef std::function<void(const SProgressResponseData&)> callbackProgress_t;
            /// \brief Callback function for message notifications.
            typedef std::function<void(const SMessageResponseData&)> callbackMessage_t;
            /// \brief Callback function for a done notification.
            typedef std::function<void()> callbackDone_t;

          public:
            static ptr_t makeRequest(const request_t& _request)
            {
                // we don't use make_shared here, becasue the constructor is private
                auto ptr = std::shared_ptr<SBaseRequestImpl>(new SBaseRequestImpl());
                requestID_t requestID = ptr->m_request.m_requestID;
                ptr->m_request = _request;
                ptr->m_request.m_requestID = requestID;
                return ptr;
            }

            void setResponseCallback(callbackResponse_t _callbackResponse)
            {
                m_callbackResponse = _callbackResponse;
            }

            void setProgressCallback(callbackProgress_t _callbackProgress)
            {
                m_callbackProgress = _callbackProgress;
            }

            void setMessageCallback(callbackMessage_t _callbackMessage)
            {
                m_callbackMessage = _callbackMessage;
            }

            void setDoneCallback(callbackDone_t _callbackDone)
            {
                m_callbackDone = _callbackDone;
            }

            /// For tests or internal use
            void execResponseCallback(const response_t& _arg)
            {
                m_callbackResponse(_arg);
            }

            void execProgressCallback(const SProgressResponseData& _arg)
            {
                m_callbackProgress(_arg);
            }

            void execMessageCallback(const SMessageResponseData& _arg)
            {
                m_callbackMessage(_arg);
            }

            void execDoneCallback()
            {
                m_callbackDone();
            }

            const request_t& getRequest() const
            {
                return m_request;
            }

          protected:
            SBaseRequestImpl()
            {
                const boost::uuids::uuid id = boost::uuids::random_generator()();
                std::stringstream strid;
                strid << id;

                m_request.m_requestID = crc64(strid.str());
            }
            SBaseRequestImpl(const SBaseRequestImpl&);

            uint64_t crc64(const std::string& _str)
            {
                boost::crc_optimal<64, 0x04C11DB7, 0, 0, false, false> crc;
                crc.process_bytes(_str.data(), _str.size());
                return crc.checksum();
            }

          private:
            callbackResponse_t m_callbackResponse;
            callbackProgress_t m_callbackProgress;
            callbackMessage_t m_callbackMessage;
            callbackDone_t m_callbackDone;

            request_t m_request;
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLSPROTOCOLCORE_H */
