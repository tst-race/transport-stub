#pragma once
#include <chrono>
#include <random>

struct RetryPolicy {
  int   maxAttempts = 3;
  int   baseDelayMs = 400;
  bool  jitter = true;
  float backoffMultiplier = 2.0f;

  int computeDelayMs(int attempt) const {
    if (attempt <= 0) return 0;
    double d = baseDelayMs;
    for (int i = 1; i < attempt; ++i) d *= backoffMultiplier;
    if (jitter) {
      static thread_local std::mt19937 rng {std::random_device{}()};
      std::uniform_int_distribution<int> dist(static_cast<int>(d * 0.5), static_cast<int>(d * 1.2));
      return dist(rng);
    }
    return static_cast<int>(d);
  }

  bool shouldRetry(int attempt, bool retryable) const {
    return retryable && attempt < maxAttempts;
  }
};