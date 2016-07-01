// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__KeyValueManager__
#define __DDS__KeyValueManager__

// DDS
#include "DeleteKeyCmd.h"
#include "Topology.h"
#include "UpdateKeyCmd.h"
// STD
#include <map>
#include <mutex>

namespace dds
{
    namespace commander_cmd
    {
        ///
        /// TODO: Key-Value has to provide a possibility to properly react on the errors.
        /// If Key-Value was not saved by the Commander because of the version mismatch, we have to send back the
        /// information about it to the user, providing error code, error message, user record and server record.
        /// Based on this information user can properly construct a new record and save it back to server.

        /// \struct SKeyValueRecord
        /// \brief Represents a single key-value record.
        ///
        /// This object is thread safe, i.e. key-value updates and deletes can be called from different threads.
        ///
        struct SKeyValueRecord
        {
            typedef std::shared_ptr<SKeyValueRecord> ptr_t;
            typedef std::weak_ptr<SKeyValueRecord> wptr_t;
            // task ID --> STaskKeyValue
            typedef std::multimap<uint64_t, SKeyValueRecord::ptr_t> taskMap_t;
            // key --> STaskKeyValue
            // typedef std::map<std::string, SKeyValueRecord::ptr_t> keyMap_t;

            SKeyValueRecord();
            ~SKeyValueRecord();

            void updateKeyValue(const protocol_api::SUpdateKeyCmd& _cmd);
            void deleteKeyValue();
            std::string getKeyValueString() const;

            friend std::ostream& operator<<(std::ostream& _stream, const SKeyValueRecord& _value);

          private:
            mutable std::mutex m_mutex;
            protocol_api::SUpdateKeyCmd m_keyValue;
            bool m_deleted;
        };

        inline std::ostream& operator<<(std::ostream& _stream, const SKeyValueRecord& _value)
        {
            std::lock_guard<std::mutex> lock(_value.m_mutex);

            return _stream << "SKeyValueRecord cmd: " << _value.m_keyValue << " deleted: " << _value.m_deleted;
        }

        /// \struct SPropertyRecord
        /// \brief Container for the key-value records with the same property ID.
        struct SPropertyRecord
        {
            typedef std::shared_ptr<SPropertyRecord> ptr_t;
            typedef std::weak_ptr<SPropertyRecord> wptr_t;
            // property --> SPropertyRecord
            typedef std::map<std::string, SPropertyRecord::ptr_t> propertyMap_t;

            SPropertyRecord();
            ~SPropertyRecord();

            void addKeyValueRecord(uint64_t _taskID, SKeyValueRecord::ptr_t _keyValueRecord);

            void updateKeyValue(const protocol_api::SUpdateKeyCmd& _cmd);

            std::string getKeyValueString() const;

            friend std::ostream& operator<<(std::ostream& _stream, const SPropertyRecord& _value);

          private:
            SKeyValueRecord::taskMap_t m_taskMap;
        };

        inline std::ostream& operator<<(std::ostream& _stream, const SPropertyRecord& _value)
        {
            _stream << "SPropertyRecord\n";
            for (const auto& v : _value.m_taskMap)
            {
                _stream << "   " << v.first << " --> " << *(v.second) << "\n";
            }
            return _stream;
        }

        /// \class CKeyValueManager
        /// \brief Key-value manager for the DDS commander.
        ///
        /// In order to keep a thread safety of the key-value manager we don't add and remove elements in the data
        /// containers on the fly. All key-value records are added during the initialization of the manager based on the
        /// information from the topology. When record is deleted it is only marked as deleted without actual deleting
        /// it from the container. This approach allows us to have a fast updates and deletes of the records without
        /// locking the whole container, only the records which are modifying now are locked.
        ///
        class CKeyValueManager
        {
          public:
            CKeyValueManager();
            ~CKeyValueManager();

            void initWithTopology(const topology_api::CTopology& _topology);
            void updateKeyValue(const protocol_api::SUpdateKeyCmd& _cmd);
            void deleteKeyValue(uint64_t _taskID);

            std::string getKeyValueString(const std::string _propertyID) const;
            std::string getPropertyString() const;

            friend std::ostream& operator<<(std::ostream& _stream, const CKeyValueManager& _value);

          private:
            SPropertyRecord::propertyMap_t m_propertyMap;
            // The map is used to query specific records by task ID, for example, to fetch or delete all records for a
            // specific task ID.
            SKeyValueRecord::taskMap_t m_taskMap;
        };

        inline std::ostream& operator<<(std::ostream& _stream, const CKeyValueManager& _value)
        {
            _stream << "CKeyValueManager\n";
            _stream << "[Property map]\n";
            for (const auto& v : _value.m_propertyMap)
            {
                _stream << "   " << v.first << " --> " << *(v.second) << "\n";
            }
            _stream << "[Task map]\n";
            for (const auto& v : _value.m_taskMap)
            {
                _stream << "   " << v.first << " --> " << *(v.second) << "\n";
            }

            return _stream;
        }
    }
}

#endif /* defined(__DDS__KeyValueManager__) */
