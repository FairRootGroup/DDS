// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef LOCAL_TYPES_H
#define LOCAL_TYPES_H
#include <boost/function.hpp>

typedef boost::function<void(const std::string&, const std::string&, bool)> log_func_t;

struct SWNOptions
{
    SWNOptions()
        : m_debug(false)
        , m_logs(false)
        , m_fastClean(false)
    {
    }

    bool m_debug;
    bool m_logs;
    bool m_fastClean;
    std::string m_scriptName;
};

#endif
