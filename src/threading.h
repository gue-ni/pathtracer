/* Copyright (c) 2024 Jakob Maier. All rights reserved. */
#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool
{
 public:
  ThreadPool(std::size_t thread_count);
  ~ThreadPool();
  void push_task(const std::function<void()>& task);
  void terminate();

 private:
  bool m_terminate;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::vector<std::thread> m_threads;
  std::queue<std::function<void()>> m_tasks;

  void thread_loop();
};