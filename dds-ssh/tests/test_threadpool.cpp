/************************************************************************/
/**
 * @file test_config.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-06-07
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
//=============================================================================
// This is a simple test of threadPool.h
// We just start a thread pool and push simple tasks in it.
// Each task sleeps for a defined amount of time and records a thread id
// of the thread, which executed this task.
// At the end we check that each thread was used an equal amount of times.
//=============================================================================
// STD
#include <iostream>
#include <iterator>
#include <vector>
#include <unistd.h>
// MiscCommon
#include "SysHelper.h"
// pod-ssh
#include "version.h"
#include "threadPool.h"
//=============================================================================
using namespace std;
using namespace dds::ssh;
//=============================================================================
const size_t g_sleeptime = 1; // in secs.
const size_t g_numTasks = 32;
const size_t g_numThreads = 4;
//=============================================================================
enum EProc
{
    start,
    clean
};
class CTestTask : public CTaskImp<CTestTask, EProc>
{
  public:
    bool runTask(EProc _param)
    {
        m_tid = MiscCommon::gettid();
        sleep(g_sleeptime);
        return true;
    }
    unsigned long threadID() const
    {
        return m_tid;
    }

  private:
    unsigned long m_tid;
};
ostream& operator<<(ostream& _stream, const CTestTask& _task)
{
    _stream << _task.threadID();
    return _stream;
}
//=============================================================================
int main()
{
    CThreadPool<CTestTask, EProc> threadPool(4);
    vector<CTestTask> tasksList(g_numTasks);
    for (size_t i = 0; i < g_numTasks; ++i)
    {
        threadPool.pushTask(tasksList[i], start);
    }
    threadPool.stop(true);

    //    ostream_iterator<CTestTask> out_it( cout, "\n" );
    //    copy( tasksList.begin(), tasksList.end(),
    //          out_it );

    typedef map<unsigned long, size_t> counter_t;
    counter_t counter;
    {
        vector<CTestTask>::const_iterator iter = tasksList.begin();
        vector<CTestTask>::const_iterator iter_end = tasksList.end();
        for (; iter != iter_end; ++iter)
        {
            counter_t::iterator found = counter.find(iter->threadID());
            if (found == counter.end())
                counter.insert(counter_t::value_type(iter->threadID(), 1));
            else
            {
                found->second = found->second + 1;
            }
        }
    }

    counter_t::const_iterator iter = counter.begin();
    counter_t::const_iterator iter_end = counter.end();
    for (; iter != iter_end; ++iter)
    {
        cout << iter->first << " was used " << iter->second << " times\n";
        // each thread suppose to be used equal amount of time,
        // exactly (g_numTasks/g_numThreads) times
        if (iter->second != (g_numTasks / g_numThreads))
        {
            cerr << "ThreadPool: simple test - Failed" << endl;
            return 1;
        }
    }
    cout << "ThreadPool: simple test - OK" << endl;

    return 0;
}
