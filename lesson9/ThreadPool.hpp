#pragma once

#include <iostream>
#include <queue>
#include <pthread.h>
#include "Task.hpp"
#define NUM 5

template<typename T>
class ThreadPool{
  private:
    int thread_num;
    std::queue<T> task_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
  public:
    ThreadPool(int _num = NUM)
      :thread_num(_num)
    {
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond,nullptr);
    }
    void LockQueue()
    {
      pthread_mutex_lock(&lock);
    }
    void UnlockQueue()
    {
      pthread_mutex_unlock(&lock);
    }
    bool IsQueueEmpty()
    {
      return task_queue.size() == 0 ? true : false;
    }
    void Wait()
    {
      pthread_cond_wait(&cond,&lock);
    }
    void Wakeup()
    {
      /* wake up the pthread which is waitting for cond.. */
      pthread_cond_signal(&cond);
    }
    static void* Routine(void* arg)
    {

      pthread_detach(pthread_self());
      ThreadPool* self = (ThreadPool*)arg;
      while(true)
      {
        self->LockQueue();
        while(self->IsQueueEmpty())
        {
          /* wait */
          self->Wait();
        }
        /* tasks must exist */
        T t;
        self->Pop(t);
        self->UnlockQueue();
        t.Run();
      }
        /* implement the tasks */

    }
    void InitThreadPool()
    {
      pthread_t tid;
      for(int i = 0; i < thread_num; i++)
      {
        pthread_create(&tid,nullptr,Routine,this);
      }
    }
    void Push(const T& in)
    {
      LockQueue();
      task_queue.push(in);

      UnlockQueue();
      Wakeup();

    }
    void Pop(T& out)
    {
      out = task_queue.front();
      task_queue.pop();
    }
    ~ThreadPool()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond);
    }

};
