// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H

// STD
#include <string>

namespace dds
{
    namespace commander
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            enum ECommands
            {
                cmd_unknown,
                cmd_start,
                cmd_stop,
                cmd_status
            };
            SOptions()
                : m_Command(cmd_start)
            {
            }

            static ECommands getCommandByName(const std::string& _name)
            {
                if ("start" == _name)
                    return cmd_start;
                if ("stop" == _name)
                    return cmd_stop;
                if ("status" == _name)
                    return cmd_status;

                return cmd_unknown;
            }

            ECommands m_Command;
        } SOptions_t;
    }
}

#endif
