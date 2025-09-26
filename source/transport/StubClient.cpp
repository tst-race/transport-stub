#include "StubClient.h"
#include "log.h"
#include <nlohmann/json.hpp>
#include <thread>  // needed for std::this_thread::sleep_for
#include <chrono>  // needed for std::chrono::milliseconds

StubClient::StubClient(std::unique_ptr<HttpClient> http,
                       ServiceConfig config,
                       std::string accessToken)
  : _http(std::move(http)),
    _cfg(std::move(config)),
    _token(std::move(accessToken)) {}

std::vector<HttpHeader> StubClient::authHeaders() const {
  if (_token.empty()) return {};
  // Generic bearer pattern (override if service differs).
  return {{"Authorization", "Bearer " + _token},
          {"Content-Type", "application/json"}};
}

HttpResponse StubClient::doWithRetry(const HttpRequest& baseReq, bool idempotent) {
  for (int attempt = 0; ; ++attempt) {
    auto resp = _http->perform(baseReq);
    bool retryable = !resp.error.empty() || (resp.status >= 500 && resp.status < 600);
    if (!retryable || !_retry.shouldRetry(attempt, retryable) || !idempotent)
      return resp;
    int delayMs = _retry.computeDelayMs(attempt + 1);
    logDebug("StubClient retry attempt=" + std::to_string(attempt+1) +
             " delayMs=" + std::to_string(delayMs));
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }
}

std::vector<Message> StubClient::parsefetchResponse(const HttpResponse& r) {
  std::vector<Message> out;
  if (!r.error.empty() || r.status < 200 || r.status >= 300) return out;
  try {
    auto j = nlohmann::json::parse(r.body);
    // TODO(SERVICE): adapt field paths
    if (j.contains("items") && j["items"].is_array()) {
      for (auto& it : j["items"]) {
        Message m;
        if (it.contains("id")) m.id = it["id"].get<std::string>();
        if (it.contains("text")) m.text = it["text"].get<std::string>();
        if (it.contains("author")) m.author = it["author"].get<std::string>();
        m.rawJson = it.dump();
        out.push_back(std::move(m));
      }
    }
  } catch (...) {
    logWarning("StubClient parsefetchResponse: JSON parse failed");
  }
  return out;
}

PostResult StubClient::parsePostResponse(const HttpResponse& r) {
  PostResult pr;
  if (!r.error.empty()) {
    pr.error = r.error;
    return pr;
  }
  if (r.status >= 200 && r.status < 300) {
    pr.success = true;
    try {
      auto j = nlohmann::json::parse(r.body);
      if (j.contains("id")) pr.id = j["id"].get<std::string>();
    } catch (...) {}
  } else {
    pr.error = "HTTP " + std::to_string(r.status);
  }
  return pr;
}

std::vector<Message> StubClient::fetch(const std::string& query,
                                        std::optional<std::string> cursor) {
  HttpRequest req;
  req.method = "GET";
  std::string endpoint = _cfg.endpoint("fetch");
  req.url = _cfg.baseUrl() + endpoint + "?q=" + query;
  if (cursor) req.url += "&cursor=" + *cursor;
  for (auto& h : authHeaders()) req.headers.push_back(h);
  auto resp = doWithRetry(req, true);
  return parsefetchResponse(resp);
}

PostResult StubClient::post(const std::string& content) {
  HttpRequest req;
  req.method = "POST";
  req.url = _cfg.baseUrl() + _cfg.endpoint("post");
  nlohmann::json body;
  // TODO(SERVICE): adjust field name
  body["text"] = content;
  req.body = body.dump();
  for (auto& h : authHeaders()) req.headers.push_back(h);
  auto resp = doWithRetry(req, false); // usually not idempotent
  return parsePostResponse(resp);
}

bool StubClient::isHealthy() {
  HttpRequest req;
  req.method = "GET";
  auto health = _cfg.endpoint("health");
  if (health.empty()) return true; // no endpoint defined
  req.url = _cfg.baseUrl() + health;
  for (auto& h : authHeaders()) req.headers.push_back(h);
  auto resp = _http->perform(req);
  return resp.error.empty() && resp.status >= 200 && resp.status < 300;
}
