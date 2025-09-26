# 
# Copyright 2023 Two Six Technologies
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

################################################################################
# Transport component
################################################################################

set(LIB_DEPS
    curl
    nlohmann_json::nlohmann_json
    xml2
    # Add other dependencies here if needed
)

set(SOURCE_FILES
    PluginStub.cpp
    StubClient.cpp
    Link.cpp
    LinkAddress.cpp
    LinkMap.cpp
    MessageHashQueue.cpp
)

set(HEADER_FILES
    PluginStub.h
    StubClient.h
    Link.h
    LinkAddress.h
    LinkMap.h
    MessageHashQueue.h
    curlwrap.h
)

set(STUB_SOURCES
    PluginStub.cpp
    StubClient.cpp
    ServiceConfig.cpp
    SimpleCurlHttpClient.cpp
    Link.cpp
    LinkAddress.cpp
    LinkMap.cpp
    MessageHashQueue.cpp
)

set(STUB_HEADERS
    PluginStub.h
    StubClient.h
    ServiceClient.h
    ServiceConfig.h
    ServiceError.h
    HttpClient.h
    SimpleCurlHttpClient.h
    RetryPolicy.h
)

setup_component_target(
    TARGET PluginStub
    SOURCES
	../common/base64.cpp
        Link.cpp
        LinkAddress.cpp
        LinkMap.cpp
        MessageHashQueue.cpp
        StubClient.cpp
        PluginStub.cpp
        ServiceConfig.cpp
        SimpleCurlHttpClient.cpp
        ../common/log.cpp
)

# Add libxml2 include directories
target_include_directories(PluginStub PRIVATE
    ${libxml2_SOURCE_DIR}/include
    ${libxml2_BINARY_DIR}
)
