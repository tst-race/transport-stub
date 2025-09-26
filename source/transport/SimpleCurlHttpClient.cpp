#include "SimpleCurlHttpClient.h"
#include "log.h"
#include <curl/curl.h>
#include <memory>

namespace {
size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}
}

HttpResponse SimpleCurlHttpClient::perform(const HttpRequest& req) {
    HttpResponse resp;
    CURL* curl = curl_easy_init();
    if (!curl) {
      resp.error = "curl_easy_init failed";
      return resp;
    }
    std::string bodyBuf;
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyBuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, req.timeoutMs);
    if (req.method == "POST" || req.method == "PUT") {
      if (req.method == "POST")
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
      else
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req.body.size());
    }
    struct curl_slist* hdrs = nullptr;
    for (auto& h : req.headers) {
      std::string line = h.name + ": " + h.value;
      hdrs = curl_slist_append(hdrs, line.c_str());
    }
    if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
      resp.error = curl_easy_strerror(code);
    } else {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status);
      resp.body = std::move(bodyBuf);
    }
    if (hdrs) curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return resp;
  }

// Factory function (optional)
std::unique_ptr<HttpClient> makeHttpClient() {
    return std::make_unique<SimpleCurlHttpClient>();
}