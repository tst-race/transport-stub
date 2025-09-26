# Developer Guide: Extending the Stub Plugin

## Step-by-Step Integration Process

### Step 1: Service Research
Before coding, understand your target service:
- API documentation URL
- Authentication method (OAuth, API Key, Bearer Token)
- Rate limits and restrictions
- Message format and size limits
- Available endpoints for reading/writing

### Step 2: Update StubClient

#### 2.1 Authentication
```cpp
// In StubClient constructor, implement service-specific auth:
StubClient::StubClient(const std::string& serverUrl, const std::string& accessToken) {
    // Example for Twitter API v2:
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 
        curl_slist_append(nullptr, ("Authorization: Bearer " + accessToken).c_str()));
}
```

#### 2.2 Message Retrieval
```cpp
std::vector<std::string> StubClient::searchStatuses(const std::string& query) {
    // Build service-specific API URL
    std::string url = serverUrl + "/your/api/endpoint?query=" + encode(query);
    
    // Make HTTP request
    // Parse JSON response
    // Extract messages
    // Return message content
}
```

#### 2.3 Message Posting
```cpp
bool StubClient::postStatus(const std::string& content) {
    // Build POST request to service API
    // Set appropriate headers (Content-Type, etc.)
    // Send content in service-expected format
    // Handle response and errors
}
```

### Step 3: Update LinkAddress Format

```cpp
// In LinkAddress.cpp, define how addresses work for your service
// Examples:
// Twitter: hashtag="#news", username="@user123"  
// Discord: channel="123456789", server="987654321"
// Reddit: subreddit="r/programming", user="u/developer"
```

### Step 4: Configure Message Flow

#### 4.1 Receiving Messages (Link::fetch)
```cpp
ComponentStatus Link::fetch() {
    // Call StubClient to get new messages
    auto messages = stubClient->searchStatuses(address.query);
    
    // For each message, call sdk->onReceive()
    for (const auto& message : messages) {
        sdk->onReceive(linkId, {linkId, "*/*", false, {}}, message);
    }
}
```

#### 4.2 Sending Messages (Link::send)
```cpp  
ComponentStatus Link::send(const std::vector<uint8_t>& data) {
    // Convert data to string
    std::string content(data.begin(), data.end());
    
    // Post to service
    return stubClient->postStatus(content) ? COMPONENT_OK : COMPONENT_ERROR;
}
```

### Step 5: Testing Your Implementation

1. **Unit Tests**: Test individual methods with mock data
2. **Integration Tests**: Test with actual service (use test accounts)
3. **Rate Limit Testing**: Ensure you handle API limits gracefully
4. **Error Handling**: Test network failures, authentication errors

### Common Gotchas

- **Rate Limits**: Most services have rate limits - implement backoff
- **Authentication**: Tokens may expire - handle refresh if needed
- **Message Encoding**: Handle special characters and encoding properly
- **Error Responses**: Parse error responses from the service API
- **SSL/TLS**: Ensure proper certificate handling for HTTPS

### Debugging Tips

- Use `logDebug()`, `logInfo()` liberally
- Log HTTP requests/responses (sanitize sensitive data)
- Use curl command line to test API calls outside the plugin
- Check service API status pages for outages