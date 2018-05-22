// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoElement__
#define __DDS__TopoElement__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        class CTopoElement : public CTopoBase
        {
          public:
            /// \brief Return number of all tasks including daughter elements.
            virtual size_t getNofTasks() const = 0;

            /// \brief Return total number of tasks, i.e. number of tasks multiplied by n.
            virtual size_t getTotalNofTasks() const = 0;

          protected:
            /// \brief Constructor.
            CTopoElement();

            /// \brief Destructor.
            virtual ~CTopoElement();

            /// \brief If parent is a group than return N, else return 1.
            /// Default implementation for Task::getTotalCounter and TaskCollection::getTotalCounter.
            size_t getTotalCounterDefault() const;

          private:
            std::string m_path; // Full path to element, including element name
        };

        typedef std::shared_ptr<CTopoElement> TopoElementPtr_t;
        typedef std::vector<TopoElementPtr_t> TopoElementPtrVector_t;
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoElement__) */
