// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef _DDS_PROGRESSDISPLAY_H_
#define _DDS_PROGRESSDISPLAY_H_

#include <chrono>

namespace dds::misc
{
    inline std::string getProgressDisplayString(int _completed, int _total)
    {
        float progress = (_total != 0) ? (float)_completed / (float)_total : 0;
        if (progress > 1.)
            progress = 1.;

        const unsigned int barWidth = 50;

        std::stringstream ss;
        ss << "[";
        unsigned int pos = progress * barWidth;

        for (unsigned int i = 0; i < barWidth; ++i)
        {
            if (i <= pos)
                ss << "=";
            else
                ss << " ";
        }
        ss << "] " << int(progress * 100.0) << " % (" << _completed << "/" << _total << ")\r";

        return ss.str();
    }
} // namespace dds::misc

#endif /*_DDS_PROGRESSDISPLAY_H_*/
