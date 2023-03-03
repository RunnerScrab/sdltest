#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
#include <sys/sysinfo.h>
#include <deque>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include <atomic>

class Task
{
public:
	Task(){}
	virtual ~Task(){}
	virtual void operator()()
	{
	}
	virtual bool ShouldDelete()
	{
		return false;
	}
};

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	unsigned int GetTaskQueueCount()
	{
		return m_taskqueue.size();
	}
	void LockTaskQueue()
	{
		pthread_mutex_lock(&m_taskqueuemutex);
	}

	void UnlockTaskQueue()
	{
		pthread_mutex_unlock(&m_taskqueuemutex);
	}

	void StopThreadPool()
	{
		m_bIsRunning = 0;
		pthread_cond_broadcast(&m_wakecond);
	}

	bool GetIsRunning() const
	{
		return m_bIsRunning;
	}

	int WaitOnQueueCond()
	{
		return pthread_cond_wait(&m_wakecond, &m_taskqueuemutex);
	}

	Task* GetNextQueuedTask()
	{
		Task* pTask =  m_taskqueue.front();
		m_taskqueue.pop_front();
		return pTask;
	}

	void WaitForAllCurrentTasks(unsigned int count)
	{
		while(true)
		{
			for(LockResultMutex();
			    count > m_result_count && GetIsRunning();)
			{
				if(0 != pthread_cond_wait(&m_resultcond, &m_resultmutex))
				{
					UnlockResultMutex();
					return;
				}
			}
			if(!GetIsRunning())
				break;
			unsigned int newcount = m_result_count;
			UnlockResultMutex();

			if(newcount == count)
			{
				break;
			}
		}
	}

	void SignalWakeCond()
	{
		pthread_cond_signal(&m_wakecond);
	}

	void AddTask(Task* pTask)
	{
		LockTaskQueue();
		m_taskqueue.push_back(pTask);
		UnlockTaskQueue();
		SignalWakeCond();
	}

	void LockResultMutex()
	{
		pthread_mutex_lock(&m_resultmutex);
	}

	void UnlockResultMutex()
	{
		pthread_mutex_unlock(&m_resultmutex);
	}

	void SignalResultCond()
	{
		pthread_cond_signal(&m_resultcond);
	}

	void ResetResultCount()
	{
		LockResultMutex();
		m_result_count = 0;
		UnlockResultMutex();
	}
	void IncrementResultCount()
	{
		LockResultMutex();
		++m_result_count;
		UnlockResultMutex();
	}
	int GetMaxThreads() const
	{
		return m_thread_count;
	}
private:
	bool m_bIsRunning;

	unsigned int m_result_count;
	pthread_cond_t m_resultcond;
	pthread_mutex_t m_resultmutex;

	unsigned int m_thread_count;
	std::vector<pthread_t> m_pthreads;
	pthread_cond_t m_wakecond;

	pthread_mutex_t m_taskqueuemutex;
	std::deque<Task*> m_taskqueue;
};

class HelloTask : public Task
{
	public:
	HelloTask(){}
	void operator()()
	{
		printf("Hello!\n");
	}
};

#endif /* THREADPOOL_H_ */
