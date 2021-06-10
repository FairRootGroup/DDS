// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "ChannelId.h"
#include "CRC.h"
// BOOST
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// STD
#include <mutex>
#include <set>
#include <sstream>

using namespace dds;
using namespace commander_cmd;
using namespace std;

uint64_t DDSChannelId::getChannelId()
{
    // TODO: We have to check that ID was not assigned to some other agent.
    // Think about if it is possible to remove this lock from here?

    // Set stores all generated agent ID in order to be able to detect collisions
    static set<uint64_t> m_agentIdSet;
    static mutex m_agentIdSetMutex;

    do
    {
        boost::uuids::uuid id = boost::uuids::random_generator()();
        stringstream strid;
        strid << id;
        uint64_t crc = MiscCommon::crc64(strid.str());

        {
            lock_guard<mutex> lock(m_agentIdSetMutex);

            bool condition = (crc != 0 && m_agentIdSet.find(crc) == m_agentIdSet.end());
            if (condition)
            {
                m_agentIdSet.insert(crc);
                return crc;
            }
        }
    } while (true);
    return 0;
}
