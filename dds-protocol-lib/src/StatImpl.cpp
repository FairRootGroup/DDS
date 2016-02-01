// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "StatImpl.h"
#include "Logger.h"
#include "ProtocolCommands.h"

#include <sstream>

// using namespace std;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;
using namespace MiscCommon;

std::string formatBytes(double _size)
{
    stringstream ss;

    static double c_k = 1. / 1024.0;
    static double c_m = 1. / 1048576.0;
    static double c_g = 1. / 1073741824.0;
    static double c_t = 1. / 1099511627776.0;

    double b = _size;
    double k = _size * c_k;
    double m = _size * c_m;
    double g = _size * c_g;
    double t = _size * c_t;

    if (t > 1)
    {
        ss << fixed << setprecision(2) << t << " TB";
    }
    else if (g > 1)
    {
        ss << fixed << setprecision(2) << g << " GB";
    }
    else if (m > 1)
    {
        ss << fixed << setprecision(2) << m << " MB";
    }
    else if (k > 1)
    {
        ss << fixed << setprecision(2) << k << " KB";
    }
    else
    {
        ss << (uint64_t)b << " B";
    }
    return ss.str();
}

std::string stringFromStat(const string _title, const SStatParams& _statParams, const statParamsMap_t& _statParamsMap)
{
    stringstream ss;

    int colwcmd = 33;
    int colw = 15;
    int totalw = colwcmd + 4 * colw;
    ss << setw(totalw) << left << _title << endl;
    ss << setw(colwcmd) << "cmd" << setw(colw) << "mean" << setw(colw) << "max" << setw(colw) << "sum" << setw(colw)
       << "count" << endl;
    for (const auto& v : _statParamsMap)
    {
        uint16_t cmd = v.first;
        const auto& stat = v.second;
        ss << setw(colwcmd) << g_cmdToString[cmd] << setw(colw) << formatBytes(stat.m_mean) << setw(colw)
           << formatBytes(stat.m_max) << setw(colw) << formatBytes(stat.m_sum) << setw(colw) << stat.m_count << endl;
    }
    ss << setw(colwcmd) << "Total" << setw(colw) << formatBytes(_statParams.m_mean) << setw(colw) << (_statParams.m_max)
       << setw(colw) << formatBytes(_statParams.m_sum) << setw(colw) << _statParams.m_count << endl;

    return ss.str();
}

/////////////////////////////////////////
/// SStatParams
/////////////////////////////////////////
void SStatParams::addFromStatParams(const SStatParams& _stat)
{
    m_mean = (m_mean == numeric_limits<double>::min()) ? _stat.m_mean : (m_mean + _stat.m_mean) / 2;
    m_max = std::max(m_max, _stat.m_max);
    m_count += _stat.m_count;
    m_sum += _stat.m_sum;
}

void SStatParams::fillFromAccumulator(const statsAccumulator_t& _accumulator)
{
    m_count = boost::accumulators::count(_accumulator);
    m_mean = (m_count != 0) ? boost::accumulators::mean(_accumulator) : 0;
    m_max = (m_count != 0) ? boost::accumulators::max(_accumulator) : 0;
    m_sum = (m_count != 0) ? boost::accumulators::sum(_accumulator) : 0;
}

/////////////////////////////////////////
/// SWriteStat
/////////////////////////////////////////
void SWriteStat::addFromStat(const dds::protocol_api::SWriteStat& _stat)
{
    m_messageBytes.addFromStatParams(_stat.m_messageBytes);
    m_queueBytes.addFromStatParams(_stat.m_queueBytes);
    m_queueMessages.addFromStatParams(_stat.m_queueMessages);
    for (auto& v : _stat.m_messageBytesMap)
    {
        m_messageBytesMap[v.first].addFromStatParams(v.second);
    }
}

std::string SWriteStat::toString() const
{
    stringstream ss;
    int colw = 12;
    ss << stringFromStat("Write (message size)", m_messageBytes, m_messageBytesMap) << endl
       << "Write (message queue size - bytes)" << endl
       << setw(colw) << "mean" << setw(colw) << "max" << setw(colw) << "sum" << setw(colw) << "count" << endl
       << setw(colw) << formatBytes(m_queueBytes.m_mean) << setw(colw) << formatBytes(m_queueBytes.m_max) << setw(colw)
       << formatBytes(m_queueBytes.m_sum) << setw(colw) << m_queueBytes.m_count << endl
       << endl
       << "Write (message queue size - messages)" << endl
       << setw(colw) << "mean" << setw(colw) << "max" << setw(colw) << "sum" << setw(colw) << "count" << endl
       << setw(colw) << fixed << setprecision(2) << m_queueMessages.m_mean << setw(colw)
       << (uint64_t)m_queueMessages.m_max << setw(colw) << (uint64_t)m_queueMessages.m_sum << setw(colw)
       << (uint64_t)m_queueMessages.m_count << endl;

    return ss.str();
}

/////////////////////////////////////////
/// CReadStat
/////////////////////////////////////////
void SReadStat::addFromStat(const dds::protocol_api::SReadStat& _stat)
{
    m_messageBytes.addFromStatParams(_stat.m_messageBytes);
    for (auto& v : _stat.m_messageBytesMap)
    {
        m_messageBytesMap[v.first].addFromStatParams(v.second);
    }
}

std::string SReadStat::toString() const
{
    stringstream ss;

    ss << stringFromStat("Read (message size)", m_messageBytes, m_messageBytesMap) << endl;

    return ss.str();
}

/////////////////////////////////////////
/// CStatImpl
/////////////////////////////////////////
SReadStat CStatImpl::getReadStat() const
{
    SReadStat local;
    {
        std::lock_guard<std::mutex> lock(m_logReadMutex);
        local.m_messageBytes.fillFromAccumulator(m_readMessageBytesAccumulator);
        for (const auto& v : m_readMessageBytesAccumulatorMap)
        {
            local.m_messageBytesMap[v.first].fillFromAccumulator(v.second);
        }
    }
    return local;
}

SWriteStat CStatImpl::getWriteStat() const
{
    SWriteStat local;
    {
        std::lock_guard<std::mutex> lock(m_logWriteMutex);
        local.m_messageBytes.fillFromAccumulator(m_writeMessageBytesAccumulator);
        local.m_queueBytes.fillFromAccumulator(m_writeQueueBytesAccumulator);
        local.m_queueMessages.fillFromAccumulator(m_writeQueueMessagesAccumulator);
        for (const auto& v : m_writeMessageBytesAccumulatorMap)
        {
            local.m_messageBytesMap[v.first].fillFromAccumulator(v.second);
        }
    }
    return local;
}

void CStatImpl::setStatEnabled(bool _statEnabled)
{
    m_statEnabled.store(_statEnabled);
}

void CStatImpl::logReadMessage(CProtocolMessage::protocolMessagePtr_t _message)
{
    if (!m_statEnabled.load())
        return;

    // Store local copy of message header.
    // Message can be destroyed or cleared by external functions.
    SMessageHeader header(_message->header());

    m_io_service.post(
        [this, header]
        {
            std::lock_guard<std::mutex> lock(m_logReadMutex);
            m_readMessageBytesAccumulator(header.m_len + CProtocolMessage::header_length);
            m_readMessageBytesAccumulatorMap[header.m_cmd](header.m_len + CProtocolMessage::header_length);
        });
}

void CStatImpl::logWriteMessages(const protocolMessagePtrQueue_t& _messageQueue)
{
    if (!m_statEnabled.load())
        return;

    size_t numberOfMessagesInQueue = _messageQueue.size();
    m_io_service.post([this, numberOfMessagesInQueue]
                      {
                          std::lock_guard<std::mutex> lock(m_logWriteMutex);
                          m_writeQueueMessagesAccumulator(numberOfMessagesInQueue);
                      });

    uint64_t numberOfBytes = 0;
    for (const auto& message : _messageQueue)
    {
        // Store local copy of message header.
        // Message can be destroyed or cleared by external functions.
        SMessageHeader header(message->header());

        numberOfBytes += (header.m_len + CProtocolMessage::header_length);

        m_io_service.post(
            [this, header]
            {
                std::lock_guard<std::mutex> lock(m_logWriteMutex);
                m_writeMessageBytesAccumulator(header.m_len + CProtocolMessage::header_length);
                m_writeMessageBytesAccumulatorMap[header.m_cmd](header.m_len + CProtocolMessage::header_length);
            });
    }

    m_io_service.post([this, numberOfBytes]
                      {
                          std::lock_guard<std::mutex> lock(m_logWriteMutex);
                          m_writeQueueBytesAccumulator(numberOfBytes);
                      });
}
