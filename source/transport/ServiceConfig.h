#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

class ServiceConfig {
public:
  explicit ServiceConfig(const std::string& filePath);
  const std::string& baseUrl() const { return _baseUrl; }
  const std::string& authType() const { return _authType; }
  std::string endpoint(const std::string& key) const;
  bool valid() const { return _valid; }

private:
  bool _valid = false;
  std::string _baseUrl;
  std::string _authType;
  std::unordered_map<std::string, std::string> _endpoints;
};