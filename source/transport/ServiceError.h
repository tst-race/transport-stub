#pragma once
#include <string>

enum class ServiceErrorType {
  NETWORK,
  AUTH,
  RATE_LIMIT,
  BAD_REQUEST,
  SERVER,
  PARSE,
  UNKNOWN
};

struct ServiceError {
  ServiceErrorType type = ServiceErrorType::UNKNOWN;
  long httpCode = 0;
  bool retryable = false;
  std::string message;
  std::string detail;

  std::string toString() const {
    return "ServiceError(" + std::to_string(httpCode) + "): " + message +
           (detail.empty() ? "" : " | " + detail);
  }
};