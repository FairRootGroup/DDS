//
//  ProgressCmd.h
//  DDS
//
//  Created by Andrey Lebedev on 27/01/15.
//
//

#ifndef __DDS__ProgressCmd__
#define __DDS__ProgressCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SProgressCmd : public SBasicCmd<SProgressCmd>
    {
        SProgressCmd()
            : m_completed(0)
            , m_total(0)
            , m_errors(0)
            , m_time(0)
        {
        }
        SProgressCmd(uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time = 0)
            : m_completed(_completed)
            , m_total(_total)
            , m_errors(_errors)
            , m_time(_time)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return sizeof(m_completed) + sizeof(m_total) + sizeof(m_errors) + sizeof(m_time);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SProgressCmd& val) const
        {
            return (m_completed == val.m_completed && m_total == val.m_completed && m_errors == val.m_errors &&
                    m_time == val.m_time);
        }

        mutable uint32_t m_completed;
        mutable uint32_t m_total;
        mutable uint32_t m_errors;
        mutable uint32_t m_time;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SProgressCmd& val)
    {
        return _stream << "source command: completed=" << val.m_completed << " total=" << val.m_total
                       << " errors=" << val.m_errors << " time=" << val.m_time;
    }
    inline bool operator!=(const SProgressCmd& lhs, const SProgressCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__ProgressCmd__) */
