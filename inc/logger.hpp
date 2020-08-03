#ifndef OTUS_LOGGER_HPP
#define OTUS_LOGGER_HPP

#include <atomic>
#include <shared_mutex>
#include <queue>

#include "worker.hpp"

namespace otus {
  class ILogger {
  public:
    virtual ~ILogger() { };
    virtual void print(std::string const &, unsigned) = 0;
    virtual void setMainStatistics(std::string_view) = 0;
  };

  // TODO Testing
  class Logger: public ILogger {
  public:
    Logger() {
      workers.emplace_back( new WorkerStdout(
            "log",
            stdstreamMutex,
            stdoutInputQueue,
            done));

      for (int i { 1 }; i < 3; ++i)
        workers.emplace_back(new WorkerFile(
              "file" + std::to_string(i),
              fileMutex,
              fileInputQueue,
              done));
    }

    ~Logger() override {
      while (true) {
        std::shared_lock lock { stdstreamMutex };
        if (stdoutInputQueue.empty()) break;
      }
      while (true) {
        std::shared_lock lock { fileMutex };
        if (fileInputQueue.empty()) break;
      }
      done = true;
      {
        std::unique_lock lock { stdstreamMutex };
        std::cerr << mainStatistics;
        for (auto const &worker: workers) {
          std::cerr << std::string(*worker);
        }
      }
    }

    void print(std::string const &block, unsigned blockSize) override {
      {
        std::unique_lock lock { stdstreamMutex };
        stdoutInputQueue.push(std::make_pair(block, blockSize));
      }
      {
        std::unique_lock lock { fileMutex };
        fileInputQueue.push(std::make_pair(block, blockSize));
      }
    }

    void setMainStatistics(std::string_view mainStatistics) override {
      this->mainStatistics = mainStatistics;
    }

  private:
    std::atomic_bool done { };
    std::vector<std::unique_ptr<Worker>> workers { };
    Worker::QueueType stdoutInputQueue { };
    Worker::QueueType fileInputQueue { };
    std::shared_mutex stdstreamMutex { };
    std::shared_mutex fileMutex { };
    std::string mainStatistics { };
  };
}

#endif
