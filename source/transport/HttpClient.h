#pragma once
#include <string>
#include <vector>

struct HttpHeader {
  std::string name;
  std::string value;
};

struct HttpRequest {
  std::string method = "GET";
  std::string url;
  std::string body;
  std::vector<HttpHeader> headers;
  long timeoutMs = 10000;
};

struct HttpResponse {
  long status = 0;
  std::string body;
  std::vector<HttpHeader> headers;
  std::string error; // curl / transport errors
};

class HttpClient {
public:
  virtual ~HttpClient() = default;
  virtual HttpResponse perform(const HttpRequest& req) = 0;
};