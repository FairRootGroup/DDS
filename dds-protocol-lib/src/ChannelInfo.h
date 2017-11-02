// Copyright 2017 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ChannelInfo_h
#define __DDS__ChannelInfo_h

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        struct SChannelInfo
        {
            SChannelInfo()
                : m_protocolHeaderID(0)
            {
            }

            SChannelInfo(typename T::connectionPtr_t _channel, uint64_t _protocolHeaderID)
                : m_channel(_channel)
                , m_protocolHeaderID(_protocolHeaderID)
            {
            }
            typename T::connectionPtr_t m_channel;
            uint64_t m_protocolHeaderID;

            typedef std::vector<SChannelInfo<T>> container_t;
        };

        template <class T>
        bool operator<(const SChannelInfo<T>& lhs, const SChannelInfo<T>& rhs)
        {
            return lhs.m_channel.get() < rhs.m_channel.get();
        }

        template <class T>
        struct SWeakChannelInfo
        {
            SWeakChannelInfo()
                : m_protocolHeaderID(0)
            {
            }

            SWeakChannelInfo(typename T::weakConnectionPtr_t _channel, uint64_t _protocolHeaderID)
                : m_channel(_channel)
                , m_protocolHeaderID(_protocolHeaderID)
            {
            }
            typename T::weakConnectionPtr_t m_channel;
            uint64_t m_protocolHeaderID;

            typedef std::vector<SWeakChannelInfo> container_t;
        };
    }
}
#endif /* __DDS__ChannelInfo_h */
