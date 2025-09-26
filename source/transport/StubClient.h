#pragma once

#include "ServiceClient.h"
#include "HttpClient.h"
#include "RetryPolicy.h"
#include "ServiceError.h"
#include "ServiceConfig.h"
#include <memory>
#include <optional>

/*
 * StubClient: generic template implementation.
 * TODO(SERVICE): Replace placeholder request construction and JSON parsing.
 */
class StubClient : public ServiceClient {
public:
  StubClient(std::unique_ptr<HttpClient> http,
             ServiceConfig config,
             std::string accessToken);

  std::vector<Message> fetch(const std::string& query,
                              std::optional<std::string> cursor = std::nullopt) override;

  PostResult post(const std::string& content) override;

  bool isHealthy() override;

private:
  std::unique_ptr<HttpClient> _http;
  ServiceConfig               _cfg;
  std::string                 _token;
  RetryPolicy                 _retry;

  HttpResponse doWithRetry(const HttpRequest& req, bool idempotent);
  std::vector<Message> parsefetchResponse(const HttpResponse& r);
  PostResult parsePostResponse(const HttpResponse& r);
  std::vector<HttpHeader> authHeaders() const;
};
