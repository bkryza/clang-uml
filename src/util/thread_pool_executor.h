/**
 * @file src/util/thread_pool_executor.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <deque>
#include <future>
#include <thread>
#include <vector>

namespace clanguml::util {

/**
 * @brief Simple thread pool executor for parallelizing diagram generation.
 */
class thread_pool_executor {
public:
    /**
     * @brief Constructor
     *
     * @param pool_size Number of threads in the pool
     */
    explicit thread_pool_executor(unsigned int pool_size);

    thread_pool_executor(const thread_pool_executor &) = delete;
    thread_pool_executor(thread_pool_executor &&) = delete;
    thread_pool_executor &operator=(const thread_pool_executor &) = delete;
    thread_pool_executor &operator=(thread_pool_executor &&) = delete;

    ~thread_pool_executor();

    /**
     * @brief Add a task to run on the pool.
     *
     * @param task Function to execute
     * @return Future, allowing awaiting the result
     */
    std::future<void> add(std::function<void()> &&task);

    /**
     * @brief Join all active threads in the pool
     */
    void stop();

private:
    /**
     * @brief Main worker pool thread method - take task from queue and execute
     */
    void worker();

    std::packaged_task<void()> get();

    std::atomic_bool done_;
    std::deque<std::packaged_task<void()>> tasks_;
    std::mutex tasks_mutex_;
    std::condition_variable tasks_cond_;
    std::vector<std::thread> threads_;
};
} // namespace clanguml::util