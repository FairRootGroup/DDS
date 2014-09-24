// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Task__
#define __DDS__Task__

// DDS
#include "TopoElement.h"
#include "TopoProperty.h"
// STD
#include <string>
#include <memory>

namespace dds
{
    class CTask : public CTopoElement
    {
      public:
        /// \brief Constructor.
        CTask();

        /// \brief Destructor.
        virtual ~CTask();

        /// Accessors
        const std::string& getExec() const;
        const std::string& getEnv() const;
        size_t getNofProperties() const;
        TopoPropertyPtr_t getProperty(size_t _i) const;
        const TopoPropertyPtrVector_t& getProperties() const;

        /// Modifiers
        void setExec(const std::string& _exec);
        void setEnv(const std::string& _env);
        void setProperties(const TopoPropertyPtrVector_t& _properties);
        void addProperty(TopoPropertyPtr_t& _property);

        /// \brief If parent is a group than return N, else return 1.
        size_t getTotalCounter() const;

        /// \brief Inherited from DDSTopoElement.
        virtual size_t getNofTasks() const;

        /// \brief Inherited from DDSTopoElement.
        virtual size_t getTotalNofTasks() const;

        /// \brief Inherited from DDSTopoElement.
        virtual void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

        /// \brief Returns string representation of an object.
        /// \return String representation of an object.
        virtual std::string toString() const;

        /// \brief Operator << for convenient output to ostream.
        /// \return Insertion stream in order to be able to call a succession of
        /// insertion operations.
        friend std::ostream& operator<<(std::ostream& _strm, const CTask& _task);

      private:
        std::string m_exec;                   ///> Path to executable
        std::string m_env;                    ///> Path to environmtnt file
        TopoPropertyPtrVector_t m_properties; ///> Properties
    };

    typedef std::shared_ptr<CTask> TaskPtr_t;
    typedef std::vector<TaskPtr_t> TaskPtrVector_t;
}
#endif /* defined(__DDS__Task__) */
