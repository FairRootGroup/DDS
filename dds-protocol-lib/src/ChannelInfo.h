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
            SChannelInfo() = default;

            SChannelInfo(typename T::connectionPtr_t _channel, uint64_t _protocolHeaderID, bool _isSlot)
                : m_channel(_channel)
                , m_protocolHeaderID(_protocolHeaderID)
                , m_isSlot(_isSlot)
            {
            }
            typename T::connectionPtr_t m_channel;
            uint64_t m_protocolHeaderID{ 0 };
            bool m_isSlot{ false };

            typedef std::vector<SChannelInfo<T>> container_t;
        };

        template <class T>
        bool operator==(const SChannelInfo<T>& lhs, const SChannelInfo<T>& rhs)
        {
            return lhs.m_channel.get() == rhs.m_channel.get();
        }

        template <class T>
        struct SWeakChannelInfo
        {
            SWeakChannelInfo() = default;

            SWeakChannelInfo(typename T::weakConnectionPtr_t _channel, uint64_t _protocolHeaderID, bool _isSlot)
                : m_channel(_channel)
                , m_protocolHeaderID(_protocolHeaderID)
                , m_isSlot(_isSlot)
            {
            }
            typename T::weakConnectionPtr_t m_channel;
            uint64_t m_protocolHeaderID{ 0 };
            bool m_isSlot{ false };

            typedef std::vector<SWeakChannelInfo> container_t;
        };
    } // namespace protocol_api
} // namespace dds
#endif /* __DDS__ChannelInfo_h */
