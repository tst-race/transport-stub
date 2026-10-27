# Transport Stub (template)

A complete, **building, tested** raceboat transport component for a generic "containers of ordered messages" service, plus the integration-test harness around it. It is the starting point for every new indirect transport: you replace the service-specific parts and keep everything else.

```
PluginStub            ITransportComponent: parameters, links, action declaration, content queueing
 ├── Link             one service container + one worker thread (post / fetch, cursor persistence)
 ├── LinkAddress      the JSON address exchanged between nodes
 ├── LinkMap          thread-safe LinkID -> Link
 └── ServiceClient    abstract service interface (the only place that talks to the service)
      └── StubClient  reference HTTP implementation  <- replace with the real service
```

What is verified here (all commands in `INTEGRATION_TEST_GUIDE.md`):

| Check | Command |
|---|---|
| builds in the plugin-builder image | `./build_artifacts_in_docker_image.sh --image-tag <tag>` |
| 30 unit tests (gtest, in-memory fake service) | `./run_unit_tests.sh --image-tag <tag>` |
| 4 integration scenarios (send-only, send-recv, client-connect, mixed-client-connect) against `test/mock_service.py` — no credentials, no network | `./test/run_scenarios.py --image-tag <tag>` |

## Start a new transport

```bash
cd /home/ubuntu/racegen
plugins/transport-stub/new_transport.py --name Acme --out /home/ubuntu/racegen/acme-transport
cd acme-transport && ./run_unit_tests.sh --image-tag <tag>   # green before you change anything
```

`new_transport.py` renames everything consistently (plugin, library, composition id, client class, env vars, scenarios). Then follow `DEVELOPER_GUIDE.md`. Do not copy the directory by hand.

## Layout

```
CMakeLists.txt, CMakePresets.json        top level; -DBUILD_UNIT_TESTS=ON adds test/source
build_artifacts.sh                       runs INSIDE the builder image (presets); --unit-tests also builds the tests
build_artifacts_in_docker_image.sh       host entry point; takes --image-tag TAG (default $RACEBOAT_IMAGE_TAG, else main)
run_unit_tests.sh                        builds tests in the builder image, runs them in the runtime image
new_transport.py                         create a new transport from this template
config/                                  user-model action schedules shipped in the kit (post-only / fetch-only / combined)
source/manifest.json                     plugin + channel properties + composition
source/CMakeLists.txt                    PLUGIN_NAME, kit layout, setup_component_target()
source/common/                           log, JsonTypes (ActionJson), PersistentStorageHelpers
source/transport/                        the component (see above)
test/adapter.py                          scenario adapter: params, addresses, sidecar, action file per mode/role
test/scenarios/*.json                    the four standard scenarios
test/run_scenarios.py                    registers the plugin, installs scenarios, runs them, keeps logs
test/mock_service.py                     mock of the illustrative REST API (a docker-compose sidecar)
test/source/                             gtest: MockTransportSdk (real signatures), InMemoryServiceClient, tests
```

## Parameters

| `--param` key | Meaning |
|---|---|
| `stubComposition.server` | base URL of the service |
| `stubComposition.token` | access token (secret; pass as `${STUB_TOKEN}`) |
| `stubComposition.actionFile` | user-model schedule, absolute path `/kits/PluginStub/config/<file>` |

The prefix is the **composition id** from `source/manifest.json`, not the plugin name. With `STUB_SERVER` unset the adapter points at the mock sidecar; set `STUB_SERVER`/`STUB_TOKEN` to test a real service.

## Design decisions baked into the template

- **LD_CREATOR_TO_LOADER**, two independent unidirectional containers for a bidirectional exchange: each node creates and posts into its own container and loads its peer's; names are deterministic.
- Only the link that **created** a container deletes it; links rebuilt from an existing address reuse it.
- Creators never fetch; loaders never post.
- `doAction` only queues work; the link's worker thread talks to the service.
- Fetch cursors are persisted via the SDK file API and advanced only after delivery.
- Content is opaque bytes of MIME type `application/octet-stream`; encoding is a separate component (`noop` here).

See `../../RACEBOAT_PLUGIN_DEVELOPMENT_GUIDE.md` for the API reference behind each of these.
