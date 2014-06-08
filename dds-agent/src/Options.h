// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H

// STD
#include <string>

// MiscCommon
//#include "PARes.h"
//#include "PoDUserDefaultsOptions.h"

namespace dds
{
    namespace commander
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            typedef enum ECommand
            { Start,
              Stop,
              Status } ECommand_t;

            SOptions()
                : m_Command(Start)
            //    , m_bDaemonize(false)
            //    , m_bValidate(false)
            //    , m_agentMode(Server)
            //    , m_numberOfPROOFWorkers(1)
            {
            }

            // std::string m_sConfigFile;
            ECommand_t m_Command;
            // bool m_bDaemonize;
            // bool m_bValidate;
            // EAgentMode_t m_agentMode; //!< A mode of PROOFAgent, defined by ::EAgentMode_t.
            // std::string m_serverInfoFile;
            // unsigned int m_numberOfPROOFWorkers;

            // PoD::SPoDUserDefaultsOptions_t m_podOptions;
        } SOptions_t;
    }
}

#endif
