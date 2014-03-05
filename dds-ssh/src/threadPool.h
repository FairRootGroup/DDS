/************************************************************************/
/**
 * @file threadPool.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-06-23
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef THREADPOOL_H_
#define THREADPOOL_H_
// STD
#include <queue>
// BOOST
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
//=============================================================================
//TODO: Move it to MiscCommon
//=============================================================================
template <class _T, class _P>
class CTaskImp
{
    public:
        bool run( _P &_param )
        {
            _T *pThis = reinterpret_cast<_T *>( this );
            return pThis->runTask( _param );
        }
};
//=============================================================================
template <class _T, class _P>
class CTask
{
    public:
        typedef CTaskImp<_T, _P> task_t;

    public:
        CTask( task_t &_task, _P &_param ): m_task( _task ), m_taskParam( _param )
        {
        }
        bool run()
        {
            return m_task.run( m_taskParam );
        }

    private:
        task_t &m_task;
        _P m_taskParam;
};
//=============================================================================
template <class _T, class _P>
class CThreadPool
{
        typedef CTask<_T, _P> task_t;
        typedef std::queue<task_t*> taskqueue_t;
    public:
        CThreadPool( size_t _threadsCount ):
            m_stopped( false ),
            m_stopping( false ),
            m_successfulTasks( 0 ),
            m_tasksCount( 0 )

        {
            for( size_t i = 0; i < _threadsCount; ++i )
                m_threads.create_thread( boost::bind( &CThreadPool::execute, this ) );
        }

        ~CThreadPool()
        {
            stop();
        }

        void pushTask( typename CTask<_T, _P>::task_t &_task, _P _param )
        {
            boost::mutex::scoped_lock lock( m_mutex );
            task_t *task = new task_t( _task, _param );
            m_tasks.push( task );
            m_threadNeeded.notify_all();
            ++m_tasksCount;
        }
        void execute()
        {
            do
            {
                task_t* task = NULL;

                {
                    // Find a job to perform
                    boost::mutex::scoped_lock lock( m_mutex );
                    if( m_tasks.empty() && !m_stopped )
                    {
                        m_threadNeeded.wait( lock );
                    }
                    if( !m_stopped && !m_tasks.empty() )
                    {
                        task = m_tasks.front();
                        m_tasks.pop();
                    }
                }
                //Execute job
                if( task )
                {
                    if( task->run() )
                    {
                        boost::mutex::scoped_lock lock( m_mutex );
                        ++m_successfulTasks;
                    }
                    delete task;
                    task = NULL;
                }
                m_threadAvailable.notify_all();
            }
            while( !m_stopped );
        }
        void stop( bool processRemainingJobs = false )
        {
            {
                //prevent more jobs from being added to the queue
                boost::mutex::scoped_lock lock( m_mutex );
                if( m_stopped ) return;
                m_stopping = true;
            }
            if( processRemainingJobs )
            {
                boost::mutex::scoped_lock lock( m_mutex );
                //wait for queue to drain.
                while( !m_tasks.empty() && !m_stopped )
                {
                    m_threadAvailable.wait( lock );
                }
            }
            //tell all threads to stop
            {
                boost::mutex::scoped_lock lock( m_mutex );
                m_stopped = true;
            }
            m_threadNeeded.notify_all();

            m_threads.join_all();
        }
        size_t tasksCount() const
        {
            return m_tasksCount;
        }
        size_t successfulTasks() const
        {
            return m_successfulTasks;
        }

    private:
        boost::thread_group m_threads;
        taskqueue_t m_tasks;
        boost::mutex m_mutex;
        boost::condition m_threadNeeded;
        boost::condition m_threadAvailable;
        bool m_stopped;
        bool m_stopping;
        size_t m_successfulTasks;
        size_t m_tasksCount;
};

#endif
