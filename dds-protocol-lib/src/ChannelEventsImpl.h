// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelEventsImpl_h
#define DDS_ChannelEventsImpl_h
// BOOST
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace protocol_api
    {
        // Channel events, which channels and users of channel objects can subscribe on.
        enum EChannelEvents
        {
            OnConnected,
            OnFailedToConnect,
            OnRemoteEndDissconnected,
            OnHandshakeOK,
            OnHandshakeFailed
        };

        /// This class implements slots subscription and slots calls associated with certain channel events
        template <class T>
        class CChannelEventsImpl
        {
          public:
            typedef boost::signals2::signal<void(T*)> signal_t;
            typedef boost::signals2::connection connection_t;

          private:
            typedef boost::ptr_map<EChannelEvents, signal_t> signalsContainer_t;

          public:
            CChannelEventsImpl<T>()
            {
            }
            ~CChannelEventsImpl<T>()
            {
            }

          public:
            connection_t subscribeOnEvent(EChannelEvents _type, typename signal_t::slot_function_type _subscriber)
            {
                return m_sig[_type].connect(_subscriber);
            }

            void onEvent(EChannelEvents _type)
            {
                try
                {
                    T* pThis = static_cast<T*>(this);
                    m_sig[_type](pThis);
                }
                catch (...)
                {
                }
            }

          private:
            signalsContainer_t m_sig;
        };
    }
}

#endif
