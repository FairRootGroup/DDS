// Copyright 2015 GSI, Inc. All rights reserved.
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
        dds_session_id    ///< associated with $DDS_SESSION_ID - session ID of the DDS.
    };

    /// \brief The function returns a value for a given environment property.
    /// \brief Example Usage:
    /// \code
    /// #include "DDSEnvProp.h"
    /// uint64_t val = env_prop<task_id>();
    /// \endcode
    /// \tparam T type one of the environment property listed in #EEnvProp.
    /// \return a numeric value for a given environment property.
    template <EEnvProp T>
    inline typename std::enable_if<T == task_id, uint64_t>::type env_prop()
    {
        size_t ret(0);
        std::string envName;
        switch (T)
        {
            case task_id:
                envName = "DDS_TASK_ID";
                break;
            default:
                return 0;
        }
        const char* env = std::getenv(envName.c_str());
        if (nullptr == env)
            return ret;

        try
        {
            ret = std::stoul(env);
        }
        catch (...)
        {
            return 0;
        }

        return ret;
    }

    /// \brief The function returns a value for a given environment property.
    /// \brief Example Usage:
    /// \code
    /// #include "DDSEnvProp.h"
    /// size_t val = env_prop<collection_index>();
    /// \endcode
    /// \tparam T type one of the environment property listed in #EEnvProp.
    /// \return a numeric value for a given environment property.
    template <EEnvProp T>
    inline typename std::enable_if<T == task_index || T == collection_index, size_t>::type env_prop()
    {
        size_t ret(0);
        std::string envName;
        switch (T)
        {
            case task_index:
                envName = "DDS_TASK_INDEX";
                break;
            case collection_index:
                envName = "DDS_COLLECTION_INDEX";
                break;
            default:
                return 0;
        }
        const char* env = std::getenv(envName.c_str());
        if (nullptr == env)
            return ret;

        try
        {
            ret = std::stoi(env);
        }
        catch (...)
        {
            return 0;
        }

        return ret;
    }

    /// \brief The function returns a value for a given environment property.
    /// \brief Example Usage:
    /// \code
    /// #include "DDSEnvProp.h"
    /// std::string val = env_prop<task_name>();
    /// \endcode
    /// \tparam T type one of the environment property listed in enum #EEnvProp.
    /// \return a string value for a given environment property.
    template <EEnvProp T>
    inline typename std::enable_if<T == task_name || T == collection_name || T == group_name || T == dds_location ||
                                       T == task_path || T == dds_session_id,
                                   std::string>::type
        env_prop()
    {
        std::string envName;
        switch (T)
        {
            case task_name:
                envName = "DDS_TASK_NAME";
                break;
            case collection_name:
                envName = "DDS_COLLECTION_NAME";
                break;
            case group_name:
                envName = "DDS_GROUP_NAME";
                break;
            case dds_location:
                envName = "DDS_LOCATION";
                break;
            case task_path:
                envName = "DDS_TASK_PATH";
                break;
            case dds_session_id:
                envName = "DDS_SESSION_ID";
                break;
            default:
                return 0;
        }
        const char* env = std::getenv(envName.c_str());
        if (nullptr == env)
            return "";

        return std::string(env);
    }
}; // namespace dds

#endif /*DDSENVPROP_H_*/
