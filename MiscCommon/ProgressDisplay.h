// Copyright 2014 GSI, Inc. All rights reserved.
//
// This file contains a number of helpers to calculate execution time of a function.
//
#ifndef TIMEMEASURE_H_
#define TIMEMEASURE_H_

#include <chrono>

namespace MiscCommon
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
} // namespace MiscCommon

#endif /*TIMEMEASURE_H_*/
