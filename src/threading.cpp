/* Copyright (c) 2024 Jakob Maier. All rights reserved. */
#include "threading.h"

ThreadPool::ThreadPool(std::size_t thread_count) : m_terminate(false)
{
  for (std::size_t i = 0; i < thread_count; i++) {
    m_threads.emplace_back(std::thread(&ThreadPool::thread_loop, this));
  }
}

ThreadPool::~ThreadPool()
{
  terminate();

  for (auto& thread : m_threads) {
    thread.join();
  }
}

void ThreadPool::terminate()
{
  {
    std::unique_lock lock(m_mutex);
    m_terminate = true;
  }
  m_condition.notify_all();
}

void ThreadPool::push_task(const std::function<void()>& task)
{
  {
    std::unique_lock lock(m_mutex);
    m_tasks.push(task);
  }
  m_condition.notify_one();
}

void ThreadPool::thread_loop()
{
  while (true) {
    std::function<void()> task;

    {
      std::unique_lock lock(m_mutex);

      m_condition.wait(lock, [this]() -> bool { return !m_tasks.empty() || m_terminate; });

      if (m_terminate) {
        return;
      }

      task = std::move(m_tasks.front());
      m_tasks.pop();
    }

    task();
  }
}