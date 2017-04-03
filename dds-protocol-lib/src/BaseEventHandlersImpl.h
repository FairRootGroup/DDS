// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_BaseEventHandlersImpl_h
#define DDS_BaseEventHandlersImpl_h
// DDS

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

        template <typename Event_t>
        class CBaseEventHandlersImpl
        {
          private:
            // Generic container of listeners for any type of function
            typedef std::multimap<Event_t, std::unique_ptr<SHandlerHlpFunc>> Listeners_t;

          public:
            template <typename Func>
            void registerHandler(Event_t _cmd, Func _handler)
            {
                std::unique_ptr<SHandlerHlpFunc> func_ptr(new SHandlerHlpBaseFunc<Func>(_handler));
                m_registeredMessageHandlers.insert(std::make_pair(_cmd, std::move(func_ptr)));
            }

            template <class... Args>
            void dispatchHandlers(Event_t _cmd, Args&&... args)
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

            size_t getNofHandlers(Event_t _cmd) const
            {
                return m_registeredMessageHandlers.count(_cmd);
            }

          private:
            Listeners_t m_registeredMessageHandlers;
        };
    }
}

#endif
