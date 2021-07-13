// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BasicCmd__
#define __DDS__BasicCmd__

// dds::misc
#include "INet.h"
#include "def.h"
// STD
#include <sstream>
#include <string>
// BOOST
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace inet = dds::misc::INet;

namespace dds
{
    namespace protocol_api
    {
        ///
        /// All vectors (except uint8_t) have a maximum size of uint16_t i.e. 2^16.
        /// All vector<uint8_t>'s have a maximum size of uint32_t i.e. 2^32.
        /// All std::string's have a maximum size of uint16_t i.e. 2^16.
        ///
        ///
        /// \brief Helper template function calculating size of the variable.
        ///
        template <typename T>
        size_t dsize(const T& _value);

        /// \brief Helper function calculating size of uint8_t
        template <>
        inline size_t dsize<uint8_t>(const uint8_t& /*_value*/)
        {
            return sizeof(uint8_t);
        }

        /// \brief Helper function calculating size of uint16_t
        template <>
        inline size_t dsize<uint16_t>(const uint16_t& /*_value*/)
        {
            return sizeof(uint16_t);
        }

        /// \brief Helper function calculating size of uint32_t
        template <>
        inline size_t dsize<uint32_t>(const uint32_t& /*_value*/)
        {
            return sizeof(uint32_t);
        }

        /// \brief Helper function calculating size of uint64_t
        template <>
        inline size_t dsize<uint64_t>(const uint64_t& /*_value*/)
        {
            return sizeof(uint64_t);
        }

        /// \brief Helper function calculating size of std::string
        template <>
        inline size_t dsize<std::string>(const std::string& _value)
        {
            return _value.size() + sizeof(uint16_t);
        }

        /// \brief Helper function calculating size of the vector of uint8_t
        template <>
        inline size_t dsize<std::vector<uint8_t>>(const std::vector<uint8_t>& _value)
        {
            return _value.size() * sizeof(uint8_t) + sizeof(uint32_t);
        }

        /// \brief Helper function calculating size of the vector of uint16_t
        template <>
        inline size_t dsize<std::vector<uint16_t>>(const std::vector<uint16_t>& _value)
        {
            return _value.size() * sizeof(uint16_t) + sizeof(uint16_t);
        }

        /// \brief Helper function calculating size of the vector of uint32_t
        template <>
        inline size_t dsize<std::vector<uint32_t>>(const std::vector<uint32_t>& _value)
        {
            return _value.size() * sizeof(uint32_t) + sizeof(uint16_t);
        }

        /// \brief Helper function calculating size of the vector of uint64_t
        template <>
        inline size_t dsize<std::vector<uint64_t>>(const std::vector<uint64_t>& _value)
        {
            return _value.size() * sizeof(uint64_t) + sizeof(uint16_t);
        }

        /// \brief Helper function calculating size of the vector of std::string
        template <>
        inline size_t dsize<std::vector<std::string>>(const std::vector<std::string>& _value)
        {
            size_t sum(sizeof(uint16_t));
            for (const auto& v : _value)
                sum += (v.size() + sizeof(uint16_t));
            return sum;
        }

        /// \brief Helper function calculating size of the boost::uuids::uuid
        template <>
        inline size_t dsize<boost::uuids::uuid>(const boost::uuids::uuid& /*_value*/)
        {
            return boost::uuids::uuid::static_size();
        }

        ///
        /// \brief Helper template function reading data from byte array.
        ///
        template <typename T>
        void readData(T* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos);

        template <>
        inline void readData<uint8_t>(uint8_t* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            *_value = (*_data)[*_nPos];

            ++(*_nPos);
        }

        template <>
        inline void readData<uint16_t>(uint16_t* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            *_value = (*_data)[*_nPos];
            *_value += ((*_data)[++(*_nPos)] << 8);

            *_value = inet::normalizeRead(*_value);

            ++(*_nPos);
        }

        template <>
        inline void readData<uint32_t>(uint32_t* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            *_value = (*_data)[*_nPos];
            *_value += ((*_data)[++(*_nPos)] << 8);
            *_value += ((*_data)[++(*_nPos)] << 16);
            *_value += ((*_data)[++(*_nPos)] << 24);

            *_value = inet::normalizeRead(*_value);

            ++(*_nPos);
        }

        template <>
        inline void readData<uint64_t>(uint64_t* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            *_value = (*_data)[*_nPos];
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 8);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 16);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 24);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 32);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 40);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 48);
            *_value += ((uint64_t)(*_data)[++(*_nPos)] << 56);

            *_value = inet::normalizeRead(*_value);

            ++(*_nPos);
        }

        template <>
        inline void readData<std::string>(std::string* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            if (_value->size() > std::numeric_limits<uint16_t>::max())
                throw std::invalid_argument("String size can't exceed 2^16 symbols. String size: " +
                                            std::to_string(_value->size()));

            // Read number of elements in the string
            uint16_t n = 0;
            readData(&n, _data, _nPos);

            dds::misc::BYTEVector_t::const_iterator iter = _data->begin();
            std::advance(iter, *_nPos);
            dds::misc::BYTEVector_t::const_iterator iter_end = _data->begin();
            std::advance(iter_end, (*_nPos + n));
            std::copy(iter, iter_end, back_inserter(*_value));

            *_nPos += n;
        }

        template <>
        inline void readData<boost::uuids::uuid>(boost::uuids::uuid* _value,
                                                 const dds::misc::BYTEVector_t* _data,
                                                 size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            dds::misc::BYTEVector_t::const_iterator iter = _data->begin();
            std::advance(iter, *_nPos);
            dds::misc::BYTEVector_t::const_iterator iter_end = _data->begin();
            std::advance(iter_end, (*_nPos + boost::uuids::uuid::static_size()));
            copy(iter, iter_end, _value->begin());
            (*_nPos) += boost::uuids::uuid::static_size();
        }

        template <typename T>
        inline void readDataVector(std::vector<T>* _value, const dds::misc::BYTEVector_t* _data, size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            if (_value->size() > std::numeric_limits<uint16_t>::max())
                throw std::invalid_argument("Vector size can't exceed 2^16 symbols. Vector size: " +
                                            std::to_string(_value->size()));

            // Read number of elements in the vector
            uint16_t n = 0;
            readData(&n, _data, _nPos);

            _value->reserve(n);
            for (size_t i = 0; i < n; ++i)
            {
                T v;
                readData(&v, _data, _nPos);
                _value->push_back(v);
            }
        }

        template <>
        inline void readData<std::vector<uint8_t>>(std::vector<uint8_t>* _value,
                                                   const dds::misc::BYTEVector_t* _data,
                                                   size_t* _nPos)
        {
            if (_data == nullptr || _nPos == nullptr || _value == nullptr)
                throw std::invalid_argument("readDataFromContainer");

            if (_value->size() > std::numeric_limits<uint32_t>::max())
                throw std::invalid_argument("Vector<uint8_t> size can't exceed 2^32 symbols. Vector size: " +
                                            std::to_string(_value->size()));

            uint32_t n = 0;
            readData(&n, _data, _nPos);

            dds::misc::BYTEVector_t::const_iterator iter = _data->begin();
            std::advance(iter, *_nPos);
            dds::misc::BYTEVector_t::const_iterator iter_end = _data->begin();
            std::advance(iter_end, (*_nPos + n));
            std::copy(iter, iter_end, back_inserter(*_value));

            *_nPos += n;
        }

        template <>
        inline void readData<std::vector<uint16_t>>(std::vector<uint16_t>* _value,
                                                    const dds::misc::BYTEVector_t* _data,
                                                    size_t* _nPos)
        {
            readDataVector<uint16_t>(_value, _data, _nPos);
        }

        template <>
        inline void readData<std::vector<uint32_t>>(std::vector<uint32_t>* _value,
                                                    const dds::misc::BYTEVector_t* _data,
                                                    size_t* _nPos)
        {
            readDataVector<uint32_t>(_value, _data, _nPos);
        }

        template <>
        inline void readData<std::vector<uint64_t>>(std::vector<uint64_t>* _value,
                                                    const dds::misc::BYTEVector_t* _data,
                                                    size_t* _nPos)
        {
            readDataVector<uint64_t>(_value, _data, _nPos);
        }

        template <>
        inline void readData<std::vector<std::string>>(std::vector<std::string>* _value,
                                                       const dds::misc::BYTEVector_t* _data,
                                                       size_t* _nPos)
        {
            readDataVector<std::string>(_value, _data, _nPos);
        }

        ///
        /// \fn pushData
        /// \brief Helper template function pushing data to byte array.
        ///
        template <typename T>
        void pushData(const T& _value, dds::misc::BYTEVector_t* _data);

        template <>
        inline void pushData<uint8_t>(const uint8_t& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            _data->push_back(_value);
        }

        template <>
        inline void pushData<uint16_t>(const uint16_t& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            uint16_t value = inet::normalizeWrite(_value);
            _data->push_back(value & 0xFF);
            _data->push_back(value >> 8);
        }

        template <>
        inline void pushData<uint32_t>(const uint32_t& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            uint32_t value = inet::normalizeWrite(_value);
            _data->push_back(value & 0xFF);
            _data->push_back((value >> 8) & 0xFF);
            _data->push_back((value >> 16) & 0xFF);
            _data->push_back((value >> 24) & 0xFF);
        }

        template <>
        inline void pushData<uint64_t>(const uint64_t& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            uint64_t value = inet::normalizeWrite(_value);
            _data->push_back(value & 0xFF);
            _data->push_back((value >> 8) & 0xFF);
            _data->push_back((value >> 16) & 0xFF);
            _data->push_back((value >> 24) & 0xFF);
            _data->push_back((value >> 32) & 0xFF);
            _data->push_back((value >> 40) & 0xFF);
            _data->push_back((value >> 48) & 0xFF);
            _data->push_back((value >> 56) & 0xFF);
        }

        template <>
        inline void pushData<std::string>(const std::string& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            if (_value.size() > std::numeric_limits<uint16_t>::max())
                throw std::invalid_argument("String size can't exceed 2^16 symbols. String size: " +
                                            std::to_string(_value.size()));

            uint16_t n = _value.size();
            pushData(n, _data);
            copy(_value.begin(), _value.end(), back_inserter(*_data));
        }

        template <>
        inline void pushData<boost::uuids::uuid>(const boost::uuids::uuid& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            copy(_value.begin(), _value.end(), back_inserter(*_data));
        }

        template <typename T>
        inline void pushDataVector(const std::vector<T>& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            if (_value.size() > std::numeric_limits<uint16_t>::max())
                throw std::invalid_argument("Vector size can't exceed 2^16 symbols. Vector size: " +
                                            std::to_string(_value.size()));

            // Read number of elements in the vector
            uint16_t n = _value.size();
            pushData(n, _data);

            for (const T& v : _value)
            {
                pushData(v, _data);
            }
        }

        template <>
        inline void pushData<std::vector<uint8_t>>(const std::vector<uint8_t>& _value, dds::misc::BYTEVector_t* _data)
        {
            if (_data == nullptr)
                throw std::invalid_argument("pushDataFromContainer");

            if (_value.size() > std::numeric_limits<uint32_t>::max())
                throw std::invalid_argument("Vector<uint32_t> size can't exceed 2^32 symbols. Vector size: " +
                                            std::to_string(_value.size()));

            uint32_t n = _value.size();
            pushData(n, _data);
            copy(_value.begin(), _value.end(), back_inserter(*_data));
        }

        template <>
        inline void pushData<std::vector<uint16_t>>(const std::vector<uint16_t>& _value, dds::misc::BYTEVector_t* _data)
        {
            pushDataVector<uint16_t>(_value, _data);
        }

        template <>
        inline void pushData<std::vector<uint32_t>>(const std::vector<uint32_t>& _value, dds::misc::BYTEVector_t* _data)
        {
            pushDataVector<uint32_t>(_value, _data);
        }

        template <>
        inline void pushData<std::vector<uint64_t>>(const std::vector<uint64_t>& _value, dds::misc::BYTEVector_t* _data)
        {
            pushDataVector<uint64_t>(_value, _data);
        }

        template <>
        inline void pushData<std::vector<std::string>>(const std::vector<std::string>& _value,
                                                       dds::misc::BYTEVector_t* _data)
        {
            pushDataVector<std::string>(_value, _data);
        }

        struct SAttachmentDataProvider
        {
            SAttachmentDataProvider(dds::misc::BYTEVector_t* _data)
                : m_data(_data)
                , m_pos(0)
            {
            }

            SAttachmentDataProvider(const dds::misc::BYTEVector_t& _data)
                : m_data(const_cast<dds::misc::BYTEVector_t*>(&_data))
                , m_pos(0)
            {
            }

            template <typename T>
            SAttachmentDataProvider& get(T& _value)
            {
                readData(&_value, m_data, &m_pos);
                return *this;
            }

            template <typename T>
            const SAttachmentDataProvider& put(const T& _value) const
            {
                pushData(_value, m_data);
                return *this;
            }

          private:
            dds::misc::BYTEVector_t* m_data;
            size_t m_pos;
        };

        template <class _Owner>
        struct SBasicCmd
        {
            void convertFromData(const dds::misc::BYTEVector_t& _data)
            {
                _Owner* p = reinterpret_cast<_Owner*>(this);
                if (_data.size() < p->size())
                {
                    std::stringstream ss;
                    ss << "Protocol message data is too short, expected " << p->size() << " received " << _data.size();
                    throw std::runtime_error(ss.str());
                }
                p->_convertFromData(_data);
            }
            void convertToData(dds::misc::BYTEVector_t* _data) const
            {
                const _Owner* p = reinterpret_cast<const _Owner*>(this);
                p->_convertToData(_data);
            }
        };
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__BasicCmd__) */
