// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__StatImpl__
#define __DDS__StatImpl__

#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/asio.hpp>
#pragma clang diagnostic pop

#include "ProtocolMessage.h"

namespace dds
{
    namespace protocol_api
    {
        typedef boost::accumulators::accumulator_set<double,
                                                     boost::accumulators::features<boost::accumulators::tag::mean,
                                                                                   boost::accumulators::tag::max,
                                                                                   boost::accumulators::tag::count,
                                                                                   boost::accumulators::tag::sum>>
            statsAccumulator_t;

        // Maps command to stats accumulator
        typedef std::map<uint16_t, statsAccumulator_t> statsAccumulatorMap_t;

        struct SStatParams;
        typedef std::map<uint16_t, SStatParams> statParamsMap_t;

        struct SStatParams
        {
            SStatParams()
                : m_mean(std::numeric_limits<double>::min())
                , m_max(std::numeric_limits<double>::min())
                , m_count(0)
                , m_sum(0)
            {
            }

            void fillFromAccumulator(const statsAccumulator_t& _accumulator);

            /// \brief Add statistics from another structure
            /// \note This function is not thread safe.
            void addFromStatParams(const SStatParams& _stat);

            /// \brief String representation of the object
            /// \note This function is not thread safe.
            std::string toString() const;

            double m_mean;
            double m_max;
            double m_count;
            double m_sum;
        };

        struct SWriteStat
        {
            SWriteStat()
                : m_messageBytes()
                , m_queueBytes()
                , m_queueMessages()
            {
            }

            /// \brief Add statistics from another structure
            /// \note This function is not thread safe.
            void addFromStat(const SWriteStat& _stat);

            /// \brief String representation of the object
            /// \note This function is not thread safe.
            std::string toString() const;

            SStatParams m_messageBytes;
            SStatParams m_queueBytes;
            SStatParams m_queueMessages;
            statParamsMap_t m_messageBytesMap;
        };

        struct SReadStat
        {
            SReadStat()
                : m_messageBytes()
            {
            }

            /// \brief Add statistics from another structure
            /// \note This function is not thread safe.
            void addFromStat(const SReadStat& _stat);

            /// \brief String representation of the object
            /// \note This function is not thread safe.
            std::string toString() const;

            SStatParams m_messageBytes;
            statParamsMap_t m_messageBytesMap;
        };

        class CStatImpl
        {
            typedef std::deque<CProtocolMessage::protocolMessagePtr_t> protocolMessagePtrQueue_t;

          public:
            CStatImpl(boost::asio::io_service& _service)
                : m_statEnabled(false)
                , m_logReadMutex()
                , m_readMessageBytesAccumulator()
                , m_readMessageBytesAccumulatorMap()
                , m_logWriteMutex()
                , m_writeMessageBytesAccumulator()
                , m_writeQueueBytesAccumulator()
                , m_writeQueueMessagesAccumulator()
                , m_writeMessageBytesAccumulatorMap()
                , m_io_service(_service)
            {
            }

            /// \brief Return read statistics
            SReadStat getReadStat() const;

            /// \brief Return write statistics
            SWriteStat getWriteStat() const;

            /// \brief Enable/disable statistics accumulation
            void setStatEnabled(bool _statEnabled);

            // TODO: We have to think if it would be better to use weak_ptr here instead of shared_ptr
            void logReadMessage(CProtocolMessage::protocolMessagePtr_t _message);
            void logWriteMessages(const protocolMessagePtrQueue_t& _messageQueue);

          private:
            // Enable/Disable statistics accumulation
            std::atomic<bool> m_statEnabled;

            // Read statistics
            mutable std::mutex m_logReadMutex;
            statsAccumulator_t m_readMessageBytesAccumulator;
            statsAccumulatorMap_t m_readMessageBytesAccumulatorMap;

            // Write statistics
            mutable std::mutex m_logWriteMutex;
            statsAccumulator_t m_writeMessageBytesAccumulator;
            statsAccumulator_t m_writeQueueBytesAccumulator;
            statsAccumulator_t m_writeQueueMessagesAccumulator;
            statsAccumulatorMap_t m_writeMessageBytesAccumulatorMap;

            boost::asio::io_service& m_io_service;
        };
    }
};

#endif /* defined(__DDS__StatImpl__) */
