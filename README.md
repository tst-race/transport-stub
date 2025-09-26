# Transport Stub Plugin

This is a template/stub for creating transport plugins that integrate with online services like Twitter, Discord, Reddit, etc.

## Architecture Overview

```
PluginStub (main plugin interface)
    ├── StubClient (service API client)
    ├── Link (manages connections to service)
    ├── LinkAddress (service-specific addressing)
    └── MessageHashQueue (prevents duplicate messages)
```

## Quick Start for New Services

1. **Configure Service Details** in `StubClient.cpp`:
   - Update API endpoints
   - Modify authentication method
   - Implement service-specific message format

2. **Update LinkAddress** for service-specific addressing (usernames, channels, etc.)

3. **Implement Core Methods**:
   - `StubClient::fetch()` - Retrieve messages from service
   - `StubClient::post()` - Send messages to service
   - `Link::fetch()` - Periodic message retrieval
   - `Link::send()` - Message transmission
   - `PluginStub::getTransportProperties()` - specify the right MIME types for content-using actions (e.g. `"post", {jpeg/image}`)

## Service Integration Examples

### Twitter Integration
- **Authentication**: OAuth 2.0 Bearer Token
- **Fetch Messages**: GET /2/tweets/search/recent
- **Post Messages**: POST /2/tweets
- **Address Format**: @username or #hashtag

### Discord Integration  
- **Authentication**: Bot Token
- **Fetch Messages**: GET /channels/{channel.id}/messages
- **Post Messages**: POST /channels/{channel.id}/messages
- **Address Format**: channel_id

## Configuration

The plugin requests two user inputs during initialization:
1. **Server URL** - Base API endpoint (e.g., "https://api.twitter.com")
2. **Access Token** - Authentication token for the service

## Testing

Use the test framework in `test/` directory to validate your implementation.
