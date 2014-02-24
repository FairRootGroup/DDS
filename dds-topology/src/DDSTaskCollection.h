//
//  DDSTaskCollection.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTaskCollection__
#define __DDS__DDSTaskCollection__

// DDS
#include "DDSTask.h"
// STD
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class DDSTaskCollection
{
public:
    /**
     * \brief Constructor.
     */
    DDSTaskCollection()
        : m_name("")
        , m_tasks()
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskCollection()
    {
    }

    std::string getName() const
    {
        return m_name;
    }
    size_t getNofTasks() const
    {
        return m_tasks.size();
    }

    /**
     * \brief Return task by index.
     * \return Task collection by index.
     * \throw std::out_of_range
     */
    DDSTaskPtr_t getTask(size_t i) const
    {
        if (i >= getNofTasks())
            throw std::out_of_range("Out of range exception");
        return m_tasks[i];
    }

    const DDSTaskPtrVector_t& getTasks() const
    {
        return m_tasks;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }
    void setTasks(const DDSTaskPtrVector_t& tasks)
    {
        m_tasks = tasks;
    }
    void addTask(DDSTaskPtr_t task)
    {
        m_tasks.push_back(task);
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTaskCollection: m_name=" << m_name << " nofTasks=" << m_tasks.size() << " tasks:";
        for (const auto& task : m_tasks)
        {
            ss << " - " << task->toString() << std::endl;
        }
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskCollection& _taskCollection)
    {
        _strm << _taskCollection.toString();
        return _strm;
    }

private:
    std::string m_name;         ///> Name of task collection.
    DDSTaskPtrVector_t m_tasks; ///> Vector of tasks in collection.
};

typedef std::shared_ptr<DDSTaskCollection> DDSTaskCollectionPtr_t;
typedef std::vector<DDSTaskCollectionPtr_t> DDSTaskCollectionPtrVector_t;

#endif /* defined(__DDS__DDSTopology__) */
