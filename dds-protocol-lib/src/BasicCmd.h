// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BasicCmd__
#define __DDS__BasicCmd__

// MiscCommon
#include "def.h"

namespace dds
{
    template <class _Owner>
    struct SBasicCmd
    {
        void convertFromData(const MiscCommon::BYTEVector_t& _data)
        {
            _Owner* p = reinterpret_cast<_Owner*>(this);
            p->_convertFromData(_data);
            p->normalizeToLocal();
        }
        void convertToData(MiscCommon::BYTEVector_t* _data)
        {
            _Owner* p = reinterpret_cast<_Owner*>(this);
            p->normalizeToRemote();
            p->_convertToData(_data);
            p->normalizeToLocal();
        }
    };
};

#endif /* defined(__DDS__BasicCmd__) */
