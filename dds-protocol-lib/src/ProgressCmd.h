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
    namespace protocol_api
    {
        struct SProgressCmd : public SBasicCmd<SProgressCmd>
        {
            SProgressCmd();
            SProgressCmd(uint16_t _srcCmd, uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time = 0);
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SProgressCmd& val) const;

            uint32_t m_completed;
            uint32_t m_total;
            uint32_t m_errors;
            uint32_t m_time;
            uint16_t m_srcCommand;
        };
        std::ostream& operator<<(std::ostream& _stream, const SProgressCmd& val);
        bool operator!=(const SProgressCmd& lhs, const SProgressCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__ProgressCmd__) */
