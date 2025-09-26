#pragma once
#include "HttpClient.h"
#include <memory>

class SimpleCurlHttpClient : public HttpClient {
public:
    HttpResponse perform(const HttpRequest& req) override;
};

std::unique_ptr<HttpClient> makeHttpClient();