// Copyright 2014 GSI, Inc. All rights reserved.
//
// This file contains a number of helpers to calculate execution time of a function.
//
#ifndef CRC_H_
#define CRC_H_

// STD
#include <string>
// BOOST
#include <boost/crc.hpp>

namespace MiscCommon
{
    inline uint64_t crc64(const std::string& _str)
    {
        boost::crc_optimal<64, 0x04C11DB7, 0, 0, false, false> crc;
        crc.process_bytes(_str.data(), _str.size());
        return crc.checksum();
    }
}

#endif /*CRC_H_*/
