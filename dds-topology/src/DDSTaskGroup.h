//
//  DDSTaskGroup.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h

// STD
#include <ostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

class DDSTaskGroup
{
   public:
    /**
     * \brief Constructor.
     */
    DDSTaskGroup() : m_name(""), m_taskCollections(), m_tasks() {}

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskGroup() {}

    void setName(const std::string& _name) { m_name = _name; }
    void setTaskCollections(const std::vector<std::string>& _taskCollections)
    {
        m_taskCollections = _taskCollections;
    }
    void setTasks(const std::vector<std::string>& _tasks) { m_tasks = _tasks; }

    const std::string& getName() const { return m_name; }

    /**
     * \brief Returns number of task collections.
     * \return Number of task collections.
     */
    size_t getNofTaskCollections() const { return m_taskCollections.size(); }

    /**
     * \brief Returns task collection by index.
     * \return Task collection by index.
     * \throw std::out_of_range
     */
    const std::string& getTaskCollection(size_t _i) const
    {
        if (_i >= getNofTaskCollections())
            throw std::out_of_range("Out of range exception");
        return m_taskCollections[_i];
    }

    const std::vector<std::string>& getTaskCollections() const
    {
        return m_taskCollections;
    }

    size_t getNofTasks() const { return m_tasks.size(); }

    const std::string& getTask(size_t _i) const
    {
        if (_i >= getNofTasks())
            throw std::out_of_range("Out of range exception");
        return m_tasks[_i];
    }

    const std::vector<std::string>& getTasks() const { return m_tasks; }

    size_t getTotalNofTasks() const
    {
        size_t counter = getNofTasks();
        // for_each(m_taskCollections.begin(), m_taskCollections.end(),
        // [&counter] () { counter++; });
        return counter;
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTaskGroup: m_name=" << m_name
           << " nofCollections=" << m_taskCollections.size()
           << " collections=|";
        std::for_each(m_taskCollections.begin(),
                      m_taskCollections.end(),
                      [&ss](const std::string& _v) mutable
                      { ss << _v << " "; });
        ss << "|";
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm,
                                    const DDSTaskGroup& _taskGroup)
    {
        _strm << _taskGroup.toString();
        return _strm;
    }

   private:
    std::string m_name;  ///> Name of task group.
    std::vector<std::string>
        m_taskCollections;             ///> Vector of task collections.
    std::vector<std::string> m_tasks;  ///> Vector of tasks
};

#endif
