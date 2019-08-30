// Performance/Harness.h - C++ performance harness declaration
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
// -----------------------------------------------------------------------------
///
/// Declaration for the C++ performance harness class.
///
// -----------------------------------------------------------------------------

#ifndef HARNESS_H
#define HARNESS_H

#import <chrono>
#import <functional>
#import <iostream>
#import <map>
#import <string>
#import <type_traits>
#import <vector>

/**
 * Harness used for performance tests.
 *
 * The generator script will generate an extension to this class that adds a
 * run() method, which the main.swift file calls.
 */
class Harness {
public:
  /**
   * Creates a new harness that writes visualization output to the given
   * output stream.
   */
  Harness(std::ostream* results_stream);

  /**
   * Runs the test harness. This function is generated by the script into
   * _generated/Harness+Generated.cc.
   */
  void run();

private:
  /** Microseconds unit for representing times. */
  typedef std::chrono::duration<double, std::micro> microseconds_d;

  /**
   * Statistics representing the mean and standard deviation of all measured
   * attempts.
   */
  struct Statistics {
    double mean;
    double stddev;
  };

  /** The output stream to which visualization results will be written. */
  std::ostream* results_stream;

  /**
   * The number of times to loop the body of the run() method.
   * Increase this for better precision.
   */
  int run_count() const;

  /** The number of times to measure the function passed to measure(). */
  int measurement_count;

  /** The number of times to add values to repeated fields. */
  int repeated_count;

  /** Ordered list of task names */
  std::vector<std::string> subtask_names;

  /** The times taken by subtasks during each measured attempt. */
  std::map<std::string, std::vector<microseconds_d>> subtask_timings;

  /** Times for the subtasks in the current attempt. */
  std::map<std::string, std::chrono::steady_clock::duration> current_subtasks;

  /**
   * Measures the time it takes to execute the given function. The function is
   * executed five times and the mean/standard deviation are computed.
   */
  template <typename Function>
  void measure(const Function& func);

  /**
   * Measure an individual subtask whose timing will be printed separately
   * from the main results.
   */
  template <typename Function>
  typename std::result_of<Function()>::type measure_subtask(
      const std::string& name, Function&& func);

  /**
   * Writes the given subtask's name and timings to the visualization log.
   */
  void write_to_log(const std::string& name,
                    const std::vector<microseconds_d>& timings) const;

  /**
   * Compute the mean and standard deviation of the given time points.
   */
  Statistics compute_statistics(
      const std::vector<std::chrono::steady_clock::duration>& timings) const;
};

template <typename Function>
void Harness::measure(const Function& func) {
  using std::chrono::duration_cast;
  using std::chrono::steady_clock;
  using std::vector;

  vector<steady_clock::duration> timings;
  subtask_timings.clear();
  bool displayed_titles = false;

  printf("Running each check %d times, times in µs\n", run_count());

  // Do each measurement multiple times and collect the means and standard
  // deviation to account for noise.
  for (int attempt = 1; attempt <= measurement_count; attempt++) {
    current_subtasks.clear();

    auto start = steady_clock::now();
    for (auto i = 0; i < run_count(); i++) {
        subtask_names.clear();
        func();
    }
    auto end = steady_clock::now();
    auto duration = end - start;
    timings.push_back(duration);

    if (!displayed_titles) {
        auto names = std::vector<std::string>(subtask_names);
        printf("%3s", "");
        for (int i = 0; i < names.size(); i += 2) {
            printf("%-18s", names[i].c_str());
        }
        printf("\n");
        printf("%3s", "");
        printf("%9s", "");
        for (int i = 1; i < names.size(); i += 2) {
            printf("%-18s", names[i].c_str());
        }
        printf("\n");
        displayed_titles = true;
    }

    printf("%3d", attempt);
    for (const auto& name : subtask_names) {
      const auto& total_interval = current_subtasks[name];
      auto micros = duration_cast<microseconds_d>(total_interval);
      printf("%9.3f", micros.count() / run_count());
      subtask_timings[name].push_back(micros);
    }
    printf("\n");
  }

  for (const auto& entry : subtask_timings) {
    write_to_log(entry.first, entry.second);
  }

  auto stats = compute_statistics(timings);
  printf("Relative stddev = %.1f%%\n", stats.stddev / stats.mean * 100.0);
}

template <typename Function>
typename std::result_of<Function()>::type Harness::measure_subtask(
  const std::string& name, Function&& func) {
  subtask_names.push_back(name);
  using std::chrono::steady_clock;

  auto start = steady_clock::now();
  auto result = func();
  auto end = steady_clock::now();
  auto diff = end - start;
  current_subtasks[name] += diff;
  return result;
}

#endif // HARNESS_H
