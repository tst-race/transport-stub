#include "ServiceConfig.h"
#include "log.h"
#include <fstream>

ServiceConfig::ServiceConfig(const std::string& filePath) {
  std::ifstream in(filePath);
  if (!in.is_open()) {
    logWarning("ServiceConfig: cannot open " + filePath);
    return;
  }
  nlohmann::json j;
  try { in >> j; } catch (...) {
    logWarning("ServiceConfig: JSON parse error");
    return;
  }
  if (j.contains("base_url")) _baseUrl = j["base_url"].get<std::string>();
  if (j.contains("auth_type")) _authType = j["auth_type"].get<std::string>();
  if (j.contains("endpoints") && j["endpoints"].is_object()) {
    for (auto& [k,v] : j["endpoints"].items())
      _endpoints[k] = v.get<std::string>();
  }
  _valid = !_baseUrl.empty();
}

std::string ServiceConfig::endpoint(const std::string& key) const {
  auto it = _endpoints.find(key);
  return it == _endpoints.end() ? "" : it->second;
}