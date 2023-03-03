#include "threadpool.h"

static void* ThreadPool_Worker(void* pArg)
{
	ThreadPool* pTP = (ThreadPool*) pArg;
	while(1)
	{
		for(pTP->LockTaskQueue();
		    0 == pTP->GetTaskQueueCount() && pTP->GetIsRunning();)
		{
			if(0 != pTP->WaitOnQueueCond())
			{
				//Error condition
				pTP->UnlockTaskQueue();
				return 0;
			}
		}

		if(!pTP->GetIsRunning())
		{
			//Threadpool is shutting down
			break;
		}

		/* Get next queued task */
		Task* pTask = pTP->GetNextQueuedTask();
		pTP->UnlockTaskQueue();

		if(pTask)
		{
			//Run the task
			(*pTask)();
			delete pTask;
		}


	}
	pTP->UnlockTaskQueue();
	return 0;
}

ThreadPool::ThreadPool()
{
    /* A robust program would have error checking and handling */
    pthread_cond_init(&m_wakecond, 0);
    pthread_mutex_init(&m_taskqueuemutex, 0);
    m_bIsRunning = true;
    m_thread_count = get_nprocs();
    m_pthreads.resize(m_thread_count);
    for(unsigned int idx = 0; idx < m_thread_count; ++idx)
    {
        pthread_create(&(m_pthreads[idx]), 0, ThreadPool_Worker, this);
    }
}

ThreadPool::~ThreadPool()
{
    StopThreadPool();
    for(unsigned int idx = 0; idx < m_thread_count; ++idx)
    {
        pthread_join(m_pthreads[idx], 0);
    }
    pthread_cond_destroy(&m_wakecond);
    pthread_mutex_destroy(&m_taskqueuemutex);
}
