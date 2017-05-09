// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_BaseEventHandlersImpl_h
#define DDS_BaseEventHandlersImpl_h

#include "ProtocolCommands.h"
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace protocol_api
    {
        /// Helpers for event dispatching
        struct SHandlerHlpFunc
        {
        };
        template <typename T>
        struct SHandlerHlpBaseFunc : SHandlerHlpFunc
        {
            T m_signal;

            SHandlerHlpBaseFunc()
                : m_signal()
            {
            }
        };

        template <typename Event_t>
        class CBaseEventHandlersImpl
        {
          private:
            typedef std::map<Event_t, std::unique_ptr<SHandlerHlpFunc>> signalsContainer_t;

          protected:
            template <Event_t _cmd, typename R, typename... Args>
            void registerHandlerImpl(std::function<R(Args...)> _handler)
            {
                typedef boost::signals2::signal<R(Args...)> signal_t;

                auto it = m_signals.find(_cmd);
                if (it == m_signals.end())
                {
                    std::unique_ptr<SHandlerHlpBaseFunc<signal_t>> signal(new SHandlerHlpBaseFunc<signal_t>());
                    signal->m_signal.connect(_handler);
                    m_signals.insert(std::make_pair(_cmd, std::move(signal)));
                }
                else
                {
                    SHandlerHlpFunc& f = *it->second;
                    signal_t& signal = static_cast<SHandlerHlpBaseFunc<signal_t>&>(f).m_signal;
                    signal.connect(_handler);
                }
            }

          protected:
            template <class... Args>
            void dispatchHandlersImpl(Event_t _cmd, Args&&... args)
            {
                typedef boost::signals2::signal<void(Args...)> signal_t;
                auto it = m_signals.find(_cmd);
                if (it != m_signals.end())
                {
                    const SHandlerHlpFunc& f = *it->second;
                    const signal_t& signal = static_cast<const SHandlerHlpBaseFunc<signal_t>&>(f).m_signal;
                    signal(std::forward<Args>(args)...);
                }
            }

          protected:
            bool handlerExistsImpl(Event_t _cmd) const
            {
                return (m_signals.find(_cmd) != m_signals.end());
            }

          private:
            signalsContainer_t m_signals;
        };
    }
}

#endif
