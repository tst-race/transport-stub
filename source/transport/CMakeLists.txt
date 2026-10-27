# The target name determines the library file name: PluginStub -> libPluginStub.so.
# manifest.json "shared_library_path" must match that file name exactly.
set(STUB_TRANSPORT_SOURCES
    PluginStub.cpp
    Link.cpp
    LinkAddress.cpp
    LinkMap.cpp
    StubClient.cpp
    ../common/log.cpp
)

setup_component_target(
    TARGET PluginStub
    SOURCES ${STUB_TRANSPORT_SOURCES}
)
