#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

struct Message {
  std::string id;
  std::string text;
  std::string author;
  uint64_t    timestamp = 0;
  std::string rawJson;
};

struct PostResult {
  bool success = false;
  std::string id;
  std::string error;
};

class ServiceClient {
public:
  virtual ~ServiceClient() = default;

  // Fetch messages matching a query; cursor supports pagination.
  virtual std::vector<Message> fetch(const std::string& query,
                                      std::optional<std::string> cursor = std::nullopt) = 0;

  // Post content (text or already-encoded payload).
  virtual PostResult post(const std::string& content) = 0;

  // Health (auth + reachability).
  virtual bool isHealthy() = 0;
};