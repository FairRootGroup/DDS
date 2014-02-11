//
//  DDSTopology.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

#include <iostream>
#include <sstream>
#include <string>

using std::string;
using std::stringstream;
using std::endl;
using std::ostream;

class DDSTopology
{
public:
    /**
     * \brief Constructor.
     */
    DDSTopology() {}
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTopology() {}
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    string toString() const {
        stringstream ss;
        ss << "DDSTopology" << endl;
        return ss.str();
    }
    
    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of insertion operations.
     */
    friend ostream& operator<<(ostream& strm, const DDSTopology& topology) {
        strm << topology.toString();
        return strm;
    }

private:
    
};

#endif /* defined(__DDS__DDSTopology__) */
