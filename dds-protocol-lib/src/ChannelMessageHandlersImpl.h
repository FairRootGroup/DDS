// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelMessageHandlersImpl_h
#define DDS_ChannelMessageHandlersImpl_h
// DDS
#include "ProtocolCommands.h"

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
            T m_function;

            SHandlerHlpBaseFunc(T _function)
                : m_function(_function)
            {
            }
        };

        class CChannelMessageHandlersImpl
        {
          private:
            // Generic container of listeners for any type of function
            typedef std::multimap<ECmdType, std::unique_ptr<SHandlerHlpFunc>> Listeners_t;

          public:
            template <ECmdType _cmd, typename Func>
            void registerMessageHandler(Func _handler)
            {
                std::unique_ptr<SHandlerHlpFunc> func_ptr(new SHandlerHlpBaseFunc<Func>(_handler));
                m_registeredMessageHandlers.insert(Listeners_t::value_type(_cmd, std::move(func_ptr)));
            }

            template <class... Args>
            void dispatchMessageHandlers(ECmdType _cmd, Args&&... args)
            {
                typedef std::function<bool(Args...)> Func_t;
                auto functions = m_registeredMessageHandlers.equal_range(_cmd);
                for (auto it = functions.first; it != functions.second; ++it)
                {
                    const SHandlerHlpFunc& f = *it->second;
                    Func_t func = static_cast<const SHandlerHlpBaseFunc<Func_t>&>(f).m_function;
                    func(std::forward<Args>(args)...);
                }
            }

            template <ECmdType _cmd>
            size_t getNofMessageHandlers() const
            {
                return m_registeredMessageHandlers.count(_cmd);
            }

          private:
            Listeners_t m_registeredMessageHandlers;
        };
    }
}

#endif
