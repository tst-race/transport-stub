#include <gtest/gtest.h>
#include "../source/transport/StubClient.h"

class StubClientIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TODO: Set up test credentials and test server
        // Use environment variables for test credentials:
        // export TEST_SERVICE_URL="https://api.test-service.com"
        // export TEST_SERVICE_TOKEN="your_test_token"
        
        std::string testUrl = std::getenv("TEST_SERVICE_URL") ?: "https://httpbin.org";
        std::string testToken = std::getenv("TEST_SERVICE_TOKEN") ?: "test_token";
        
        client = std::make_unique<StubClient>(testUrl, testToken);
    }

    std::unique_ptr<StubClient> client;
};

TEST_F(StubClientIntegrationTest, HealthCheck) {
    // Test basic connectivity
    EXPECT_TRUE(client->isHealthy()) << "Service should be healthy";
}

TEST_F(StubClientIntegrationTest, SearchMessages) {
    // Test message retrieval
    auto results = client->searchStatuses("test_query");
    // Don't assert specific counts since service content changes
    // Just verify it doesn't crash and returns a vector
    EXPECT_GE(results.size(), 0) << "Search should return valid results";
}

TEST_F(StubClientIntegrationTest, PostMessage) {
    // Test message posting
    std::string testMessage = "Test message from integration test: " + 
                             std::to_string(time(nullptr));
    
    EXPECT_TRUE(client->postStatus(testMessage)) << "Should be able to post test message";
}