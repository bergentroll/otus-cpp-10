#ifndef OTUS_WORKER
#define OTUS_WORKER

#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>

namespace otus {
  // TODO Conditional variable.
  class Worker {
  public:
    using QueueType = std::queue<std::pair<std::string, unsigned>>;

    Worker(
        std::string const &name,
        std::shared_mutex &mutex,
        QueueType &queue,
        std::atomic_bool const &endFlag):
        name(name), mutex(mutex), queue(queue), endFlag(endFlag) {
      std::thread thread { [this]() { run(); } };
      thread.detach();
    }

    virtual ~Worker() = default;

    virtual operator std::string () const {
      std::stringstream ss;
      ss
        << "Thread "
        << name
        << ": "
        << blocksCounter
        << " blocks, "
        << commandsCounter
        << " commands."
        << std::endl;
      return ss.str();
    };

    void run() {
      while (!endFlag) {
        bool hasWork { };
        if (mutex.try_lock()) {
          if (!queue.empty()) {
            hasWork = true;
            execCritical();
            ++blocksCounter;
            commandsCounter += queue.front().second;
            queue.pop();
          }
          mutex.unlock();
          if (hasWork) execPostCritical();
        }
      }
    }

    virtual void execCritical() = 0;

    virtual void execPostCritical() { }

  protected:
    std::string name;
    std::shared_mutex &mutex;
    QueueType &queue;
    std::atomic_bool const &endFlag;
    unsigned blocksCounter { };
    unsigned commandsCounter { };
  };

  class WorkerStdout: public Worker {
    using Worker::Worker;

    void execCritical() override {
      std::cout << queue.front().first;
    }
  };

  class WorkerFile: public Worker {
    using Worker::Worker;

    std::string buf { };
    static inline std::atomic_uint index { 1 };

    void execCritical() override {
      buf = queue.front().first;
      commandsCounter += queue.front().second;
    }

    void execPostCritical() override {
      std::ofstream file { generateFileName(), std::ios_base::app };
      file << buf;
      file.close();
    }

    std::string generateFileName() {
      auto now {
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
      };

      std::stringstream path { };
      path
        << "bulk"
        << std::to_string(now)
        << '-'
        << std::setfill('0') << std::setw(5) << std::to_string(index)
        << ".log";
      ++index;
      return path.str();
    }
  };
}

#endif
