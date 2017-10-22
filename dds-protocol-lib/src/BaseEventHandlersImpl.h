// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_BaseEventHandlersImpl_h
#define DDS_BaseEventHandlersImpl_h

#include <boost/signals2/signal.hpp>

#define DDS_BEGIN_EVENT_HANDLERS(eventType)                                                                    \
  public:                                                                                                      \
    template <class... Args>                                                                                   \
    void dispatchHandlers(eventType _cmd, const SSenderInfo& _sender, Args&... args)                           \
    {                                                                                                          \
        CBaseEventHandlersImpl<eventType>::dispatchHandlersImpl<>(_cmd, _sender, std::forward<Args>(args)...); \
    }                                                                                                          \
                                                                                                               \
  public:                                                                                                      \
    bool handlerExists(eventType _cmd) const                                                                   \
    {                                                                                                          \
        return CBaseEventHandlersImpl<eventType>::handlerExistsImpl(_cmd);                                     \
    }

#define DDS_END_EVENT_HANDLERS

#define DDS_REGISTER_EVENT_HANDLER(eventType, eventID, funcType)                                               \
  public:                                                                                                      \
    template <eventType _cmd, typename func_t>                                                                 \
    void registerHandler(                                                                                      \
        func_t _handler,                                                                                       \
        typename std::enable_if<std::is_same<std::integral_constant<eventType, _cmd>,                          \
                                             std::integral_constant<eventType, eventID>>::value &&             \
                                std::is_convertible<func_t, std::function<funcType>>::value>::type* = nullptr) \
    {                                                                                                          \
        std::function<funcType> funcHandler(_handler);                                                         \
        CBaseEventHandlersImpl<eventType>::registerHandlerImpl<_cmd>(funcHandler);                             \
    }

#define DDS_DECLARE_EVENT_HANDLER_CLASS(theClass) \
    using theClass::registerHandler;              \
    using theClass::dispatchHandlers;             \
    using theClass::handlerExists;

namespace dds
{
    namespace protocol_api
    {
        struct SSenderInfo
        {
            SSenderInfo()
                : m_ID(0)
            {
            }

            uint64_t m_ID;
        };

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
            void dispatchHandlersImpl(Event_t _cmd, const SSenderInfo& _sender, Args&&... args)
            {
                typedef boost::signals2::signal<void(const SSenderInfo&, Args...)> signal_t;
                auto it = m_signals.find(_cmd);
                if (it != m_signals.end())
                {
                    const SHandlerHlpFunc& f = *it->second;
                    const signal_t& signal = static_cast<const SHandlerHlpBaseFunc<signal_t>&>(f).m_signal;
                    signal(_sender, std::forward<Args>(args)...);
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
