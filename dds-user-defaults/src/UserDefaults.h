// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#ifndef DDS_USERDEFAULTS_H_
#define DDS_USERDEFAULTS_H_
// STD
#include <string>

namespace DDS
{
    class CUserDefaults
    {
      public:
        std::string currentUDFile() const;
    };
}

#endif /* DDS_USERDEFAULTS_H_ */
