// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOL_H_
#define PROTOCOL_H_
// DDS
#include "ProtocolMessage.h"
// MiscCommon
#include "def.h"
// BOOST
#include <boost/shared_ptr.hpp>

namespace dds
{

    //----------------------------------------------------------------------

    //   MiscCommon::BYTEVector_t createMsg(uint16_t _cmd, const MiscCommon::BYTEVector_t& _data);

    //   SMessageHeader parseMsg(MiscCommon::BYTEVector_t* _data, const MiscCommon::BYTEVector_t& _msg);

    //----------------------------------------------------------------------

    /**
     *
     * @brief The protocol low level class
     *
     */
    class CProtocol
    {
        /*    public:
              CProtocol();
              virtual ~CProtocol();

              typedef enum EStatus
              {
                  stOK = 0,
                  stDISCONNECT = 1,
                  stAGAIN = 2
              } EStatus_t;

              EStatus_t read(int _socket);
              void write(int _socket, uint16_t _cmd, const MiscCommon::BYTEVector_t& _data) const;
              void writeSimpleCmd(int _socket, uint16_t _cmd) const;
              SMessageHeader getMsg(MiscCommon::BYTEVector_t* _data) const;
              bool checkoutNextMsg();

            private:
              MiscCommon::BYTEVector_t m_buffer;
              SMessageHeader m_msgHeader;
              MiscCommon::BYTEVector_t m_curDATA;*/
    };

    //----------------------------------------------------------------------

    class session_participant
    {
      public:
        virtual ~session_participant()
        {
        }
        virtual void deliver(const CProtocolMessage& msg) = 0;
    };

    typedef boost::shared_ptr<session_participant> sessionParticipantPtr_t;

    //----------------------------------------------------------------------
}

#endif /* PROTOCOL_H_ */
