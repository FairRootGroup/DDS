// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h
// DDS
#include "DDSTask.h"
#include "DDSTaskCollection.h"
// STD
#include <ostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class DDSTaskGroup
{
public:
    /**
     * \brief Constructor.
     */
    DDSTaskGroup()
        : m_name("")
        , m_taskCollections()
        , m_tasks()
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskGroup()
    {
    }

    void setName(const std::string& _name)
    {
        m_name = _name;
    }
    void setTaskCollections(const DDSTaskCollectionPtrVector_t& _taskCollections)
    {
        m_taskCollections = _taskCollections;
    }
    void addTaskCollection(DDSTaskCollectionPtr_t _taskCollection)
    {
        m_taskCollections.push_back(_taskCollection);
    }
    void setTasks(const DDSTaskPtrVector_t& _tasks)
    {
        m_tasks = _tasks;
    }
    void addTask(DDSTaskPtr_t _task)
    {
        m_tasks.push_back(_task);
    }

    const std::string& getName() const
    {
        return m_name;
    }

    /**
     * \brief Returns number of task collections.
     * \return Number of task collections.
     */
    size_t getNofTaskCollections() const
    {
        return m_taskCollections.size();
    }

    /**
     * \brief Returns task collection by index.
     * \return Task collection by index.
     * \throw std::out_of_range
     */
    DDSTaskCollectionPtr_t getTaskCollection(size_t _i) const
    {
        if (_i >= getNofTaskCollections())
            throw std::out_of_range("Out of range exception");
        return m_taskCollections[_i];
    }

    const DDSTaskCollectionPtrVector_t& getTaskCollections() const
    {
        return m_taskCollections;
    }

    size_t getNofTasks() const
    {
        return m_tasks.size();
    }

    DDSTaskPtr_t getTask(size_t _i) const
    {
        if (_i >= getNofTasks())
            throw std::out_of_range("Out of range exception");
        return m_tasks[_i];
    }

    const DDSTaskPtrVector_t& getTasks() const
    {
        return m_tasks;
    }

    size_t getTotalNofTasks() const
    {
        size_t counter = getNofTasks();
        for (const auto& tc : m_taskCollections)
        {
            counter += tc->getNofTasks();
        }
        counter += getNofTasks();
        return counter;
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTaskGroup: m_name=" << m_name << " nofTaskCollections=" << m_taskCollections.size() << " taskCollections:";
        for (const auto& taskCollection : m_taskCollections)
        {
            ss << taskCollection << std::endl;
        }
        ss << "nofTasks=" << m_tasks.size() << " tasks:";
        for (const auto& task : m_tasks)
        {
            ss << task << std::endl;
        }
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskGroup& _taskGroup)
    {
        _strm << _taskGroup.toString();
        return _strm;
    }

private:
    std::string m_name;                             ///> Name of task group.
    DDSTaskCollectionPtrVector_t m_taskCollections; ///> Vector of task collections.
    DDSTaskPtrVector_t m_tasks;                     ///> Vector of tasks
};

typedef std::shared_ptr<DDSTaskGroup> DDSTaskGroupPtr_t;
typedef std::vector<DDSTaskGroupPtr_t> DDSTaskGroupPtrVector_t;

#endif
