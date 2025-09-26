//
// Copyright 2023 Two Six Technologies
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "Link.h"
#include "ServiceClient.h"
#include "log.h"

Link::Link(const LinkID& id,
           const LinkAddress& addr,
           const LinkProperties& props,
           ITransportSdk* sdk,
           ServiceClient* serviceClient)
    : linkId(id),
      address(addr),
      properties(props),
      sdk(sdk),
      serviceClient(serviceClient),
      logPrefix("[Link " + id + "] ") {
    this->properties.linkAddress = nlohmann::json(this->address).dump();
}

void Link::start() {
    // Any startup logic if needed
}

void Link::shutdown() {
    // Any shutdown/cleanup logic if needed
}

LinkID Link::getId() const {
    return linkId;
}


const LinkProperties& Link::getProperties() const {
    return properties;
}

ComponentStatus Link::enqueueContent(uint64_t actionId, const std::vector<uint8_t>& content, const std::string& contentType) {
    if (contentType == "text/plain") {
        contentQueue[actionId].textContent = content;
        contentQueue[actionId].hasText = true;
        logDebug(logPrefix + "Enqueued text content for action " + std::to_string(actionId));
    } else if (contentType == "image/jpeg") {
        contentQueue[actionId].imageContent = content;
        contentQueue[actionId].hasImage = true;
        logDebug(logPrefix + "Enqueued image content for action " + std::to_string(actionId));
    } else {
        logError(logPrefix + "Unknown content type: " + contentType);
        return COMPONENT_ERROR;
    }
    return COMPONENT_OK;
}

ComponentStatus Link::dequeueContent(uint64_t actionId) {
    contentQueue.erase(actionId);
    return COMPONENT_OK;
}

ComponentStatus Link::post(const std::vector<RaceHandle>& handles, uint64_t actionId) {
    TRACE_METHOD(linkId, handles, actionId);

    auto iter = contentQueue.find(actionId);
    if (iter == contentQueue.end()) {
        logInfo(logPrefix + "No enqueued content for action ID: " + std::to_string(actionId));
        updatePackageStatus(handles, PACKAGE_FAILED_GENERIC);
        return COMPONENT_OK;
    }

    const ActionContent& content = iter->second;
    std::string hashtag = "#" + address.hashtag;
    
    if (!serviceClient) {
        logError(logPrefix + "ServiceClient is null");
        updatePackageStatus(handles, PACKAGE_FAILED_GENERIC);
        return COMPONENT_ERROR;
    }

    // Prepare content string based on what we have
    std::string postContent;
    
    if (content.hasText && content.hasImage) {
        // For mixed content, post text with a note about image
        std::string textStr(content.textContent.begin(), content.textContent.end());
        postContent = textStr + " [Image content: " + std::to_string(content.imageContent.size()) + " bytes] " + hashtag;
        logDebug(logPrefix + "Posting mixed content (text + image) as text with image note");
    } else if (content.hasImage) {
        // For image only, post a placeholder with size info
        postContent = "[Image: " + std::to_string(content.imageContent.size()) + " bytes] " + hashtag;
        logDebug(logPrefix + "Posting image content as text placeholder");
    } else if (content.hasText) {
        // For text only, post the text with hashtag
        std::string textStr(content.textContent.begin(), content.textContent.end());
        postContent = textStr + " " + hashtag;
        logDebug(logPrefix + "Posting text content");
    } else {
        logError(logPrefix + "No content to post for action ID: " + std::to_string(actionId));
        updatePackageStatus(handles, PACKAGE_FAILED_GENERIC);
        return COMPONENT_ERROR;
    }

    // Use ServiceClient's generic post method
    PostResult result = serviceClient->post(postContent);
    
    if (result.success) {
        logInfo(logPrefix + "Posted content successfully, ID: " + result.id);
        updatePackageStatus(handles, PACKAGE_SENT);
        contentQueue.erase(actionId);
        return COMPONENT_OK;
    } else {
        logError(logPrefix + "Failed to post content: " + result.error);
        updatePackageStatus(handles, PACKAGE_FAILED_GENERIC);
        return COMPONENT_ERROR;
    }
}

ComponentStatus Link::fetch() {
    TRACE_METHOD(linkId);

    if (!serviceClient) {
        logError(logPrefix + "ServiceClient is null");
        return COMPONENT_ERROR;
    }
    
    // Use the hashtag as search query
    std::string query = address.hashtag;
    auto messages = serviceClient->search(query);

    logInfo(logPrefix + "Fetched " + std::to_string(messages.size()) + " messages for query: " + query);

    // For simplicity, treat all fetched content as text
    for (const auto& message : messages) {
        if (!message.text.empty()) {
            std::vector<uint8_t> textData(message.text.begin(), message.text.end());
            logInfo(logPrefix + "Fetched message from " + message.author + ", size: " + std::to_string(textData.size()));
            
            // Send as text content
            sdk->onReceive(linkId, {linkId, "text/plain", false, {}}, textData);
        }
    }

    return COMPONENT_OK;
}

void Link::updatePackageStatus(const std::vector<RaceHandle>& handles, PackageStatus status) {
    for (const auto& handle : handles) {
        sdk->onPackageStatusChanged(handle, status);
    }
}
