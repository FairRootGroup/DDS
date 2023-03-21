// Copyright 2015-2023 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSENVPROP_H_
#define DDSENVPROP_H_

// STD
#include <cstdlib>
#include <string>

namespace dds
{
    /// a list of envioronment properties
    enum EEnvProp
    {
        task_id,    ///< associated with $DDS_TASK_ID environment variable.
        task_index, ///< associated with $DDS_TASK_INDEX environment variable.
        task_name,  ///< associated with $DDS_TASK_NAME - ID of the task.
        task_path,  ///< associated with $DDS_TASK_PATH - full path to the user task, for example,
        /// main/group1/collection_12/task_3.
        collection_index, ///< associated with $DDS_COLLECTION_INDEX environemnt variable.
        collection_name,  ///< associated with $DDS_COLLECTION_NAME - ID of the parent collection.
        group_name,       ///< associated with $DDS_GROUP_NAME - ID of the parent group.
        dds_location,     ///< associated with $DDS_LOCATION  environemnt variable.
        dds_session_id,   ///< associated with $DDS_SESSION_ID - session ID of the DDS.
        dds_slot_id       ///< associated with $DDS_SLOT_ID - slot ID
    };

    /// \brief The function returns a value for a given environment property.
    /// \brief Example Usage:
    /// \code
    /// #include "EnvProp.h"
    /// uint64_t val = env_prop<task_id>();
    /// std::string = env_prop<task_name>();
    /// \endcode
    /// \tparam T type one of the environment property listed in #EEnvProp.
    /// \return a numeric value for a given environment property.
    template <EEnvProp p>
    auto env_prop()
    {
        // assert(p >= task_id && p <= dds_slot_id);
        static constexpr const char* envNames[] = { "DDS_TASK_ID",    "DDS_TASK_INDEX",       "DDS_TASK_NAME",
                                                    "DDS_TASK_PATH",  "DDS_COLLECTION_INDEX", "DDS_COLLECTION_NAME",
                                                    "DDS_GROUP_NAME", "DDS_LOCATION",         "DDS_SESSION_ID",
                                                    "DDS_SLOT_ID" };
        static constexpr bool asInt[] = { true, true, false, false, true, false, false, false, false, true };
        const char* env = std::getenv(envNames[p]);
        if constexpr (asInt[p])
            return env ? std::stoull(env) : 0;
        else
            return env ? env : "";
    }

}; // namespace dds

#endif /*DDSENVPROP_H_*/
