// Copyright 2014 GSI, Inc. All rights reserved.
//
// This file contains a number of helpers to calculate execution time of a function.
//
#ifndef _DDS_CRC_H_
#define _DDS_CRC_H_

// STD
#include <fstream>
#include <string>
// BOOST
#include <boost/crc.hpp>

namespace dds::misc
{
    using crc_optimal_64_t = boost::crc_optimal<64, 0x04C11DB7, 0, 0, false, false>;

    template <class ret_t, class crc_t>
    ret_t crc(const std::string& _str)
    {
        crc_t crc;
        crc.process_bytes(_str.data(), _str.size());
        return crc.checksum();
    }

    inline uint32_t crc32(const std::string& _str)
    {
        return crc<uint32_t, boost::crc_32_type>(_str);
    }

    inline uint64_t crc64(const std::string& _str)
    {
        return crc<uint64_t, crc_optimal_64_t>(_str);
    }

    template <class ret_t, class crc_t>
    ret_t crc(std::istream& _stream)
    {
        char buf[4096];
        crc_t result;

        do
        {
            _stream.read(buf, sizeof buf);
            result.process_bytes(buf, _stream.gcount());
        } while (_stream);

        if (_stream.eof())
        {
            return result.checksum();
        }
        else
        {
            throw std::runtime_error("Failed to calculate CRC: file read failed");
        }
    }

    inline uint32_t crc32(std::istream& _stream)
    {
        return crc<uint32_t, boost::crc_32_type>(_stream);
    }

    inline uint64_t crc64(std::istream& _stream)
    {
        return crc<uint64_t, crc_optimal_64_t>(_stream);
    }
} // namespace dds::misc

#endif /*_DDS_CRC_H_*/
