#include "ThreadPool.hpp"
#include "Task.hpp"
#include <cstdlib>
#include <ctime>
#include <unistd.h>

int main()
{
  ThreadPool<Task>* tp = new ThreadPool<Task>();
  tp->InitThreadPool();
  srand((unsigned long)time(nullptr));
  const char* op = "+-*/%";

  while(true)
  {
    int x = rand() % 100 + 1;
    int y = rand() % 100 + 1;

    Task t(x,y,op[x%5]);
    tp->Push(t);
    sleep(1);
  }

  return 0;
}
