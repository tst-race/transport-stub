# Developer Guide: turning the stub into a transport

Prerequisites: read `../../AGENTS.md` (rules) and keep `../../RACEBOAT_PLUGIN_DEVELOPMENT_GUIDE.md` open (signatures). Phases 1–2 (design) are done before this guide starts; their output (credentials, content types, direction, multicast/unicast, retention, deletion) drives the choices below.

## Step 0 — generate and prove the baseline

```bash
cd /home/ubuntu/racegen
plugins/transport-stub/new_transport.py --name Acme --out /home/ubuntu/racegen/acme-transport
cd acme-transport
# <tag>: same tag for the builder image, the runtime image and every --image-tag below
./run_unit_tests.sh --image-tag <tag>  # must pass before any edit
```

Names below use `Acme`: `PluginAcme`, `AcmeClient`, `acmeComposition`, transport id `acme`, `ACME_SERVER`/`ACME_TOKEN`.

## Step 1 — implement the service (Phase 3): `ServiceClient`

Edit `source/transport/AcmeClient.{h,cpp}` (and `ServiceClient.h` only if the model truly does not fit). Map the five operations onto the service:

| Operation | Contract | Typical mapping |
|---|---|---|
| `createContainer(id)` | idempotent | create repo/bucket/channel/folder |
| `containerExists(id)` | no side effects | HEAD/GET |
| `deleteContainer(id)` | called only by the creator that created it | delete the resource |
| `post(id, bytes)` → message id | throw `ServiceError(kind, …)` | upload object/commit/post |
| `fetchAfter(id, cursor)` | messages **newer than `cursor`**, oldest first; **missing container → empty list** | list + download |

Rules: no raceboat types in this layer; classify failures (`Transient`, `NotFound`, `Auth`, `RateLimited`, `Fatal`) — `Link` maps them to package status; message ids must sort/compare in post order or you must change how the cursor is advanced (`Link::fetchOnActionThread`); keep every service quirk (rate limits, size limits, required wrapping such as base64-in-text) inside this class. Add a unit test for it against a fake server — the template's client has none, and its only defect so far hid there.

Credentials: ask the developer / reuse earlier stub-test tokens; put them in `/home/ubuntu/racegen/.env` (gitignored). Never in code, manifests, scenario files, docs or logs.

Prove it with a real round trip of a few bytes (create → post → fetchAfter → delete) before going on.

## Step 2 — parameters: `PluginAcme.cpp` constructor / `onUserInputReceived` / `finishStartupIfInputsReceived`

Request exactly what the service needs with `sdk->requestPluginUserInput("<key>", "<prompt>", cache).handle`; wait for all answers; build the client; `updateState(COMPONENT_STATE_STARTED)`; `FAILED` if a required one is missing. The adapter's `--param` keys must be `acmeComposition.<key>` with the same `<key>`s. Update the README parameter table.

## Step 3 — addresses: `LinkAddress.{h,cpp}`

Decide what a peer needs to find the data (usually the container id; add fields only when required, e.g. a region or a start cursor). Keep `parseLinkAddress` strict about mandatory fields and lenient about unknown ones. Keep secrets out. Update `TestLinkAddress.cpp`.

## Step 4 — channel semantics: `source/manifest.json`

Set from the Phase 2 design: `linkDirection`, `transmissionType` (public container → `TT_MULTICAST`), `sendType`, `reliable`, `mtu` (service object-size limit, or -1), `maxLinks`, `duration_s`/`period_s` (retention), `supportedHints`, `creatorExpected`/`loaderExpected` (`bandwidth_bps`, `latency_ms`, `loss` — exact key names). Choose the composition's `usermodel` and `encodings`. `getTransportProperties` and `kContentType` (`Link.h`) must name the MIME type(s) the Phase 2 design lists; if the service needs several content types with different actions, extend `getTransportProperties`, `getActionParams`, `enqueueContent` and `Link` accordingly (the transport is carrier-agnostic: it uploads/downloads whatever bytes the encoding produced).

`linkDirection != LD_CREATOR_TO_LOADER` changes who may fetch/post: revisit `Link::fetchOnActionThread` (creator-skips-fetch), the invalid-role arguments in `PluginAcme::createLink*`/`loadLinkAddress`, and `enqueueContent`'s creator-only wildcard rule. For a bidirectional service you also need self-send detection (ignore your own posts when fetching).

## Step 5 — the rest of `Link` / `PluginAcme`

Usually unchanged: thread model, cursor persistence, event emission, ownership rule, wildcard handling. Review `destroyLink` for what "safe deletion" means for the service (Phase 2 item 7).

## Step 6 — tests

Unit: extend `test/source` (keep `InMemoryServiceClient` in sync with any `ServiceClient` change). Integration: edit `test/adapter.py` — credentials env names, `container_name` (service naming rules), whether the mock sidecar stays (delete `test/mock_service.py` and the mock branch once the real service is used), per-mode action files. Then:

```bash
./run_unit_tests.sh --image-tag <tag>
./build_artifacts_in_docker_image.sh --image-tag <tag>
set -a; . /home/ubuntu/racegen/.env; set +a
./test/run_scenarios.py --image-tag <tag>
```

All four scenarios must report PASSED in one run (`INTEGRATION_TEST_GUIDE.md` §3). Clean service-side state between runs (§4).

## Step 7 — deliverables

- Design document: functionality, security (credential handling, what an observer/the service sees), nuances (rate limits, retention, deletion).
- Component Integration document (template in `../../RACEBOAT_PLUGIN_DEVELOPMENT_GUIDE.md` §8).
- `test/logs/` from the passing run; `.env` and logs never committed.

## Rename map (what `new_transport.py` changes)

`PluginStub`→`PluginAcme` (class, files, CMake target and kit directory, `libPluginAcme.so`, manifest `file_path`/`shared_library_path`), `StubClient`→`AcmeClient`, `stubComposition`→`acmeComposition` (manifest, adapter, scenarios), transport id `"stub"`→`"acme"`, `STUB_*`→`ACME_*`, `stub-…` scenario ids/container prefix/service host, `transport-stub`→`acme-transport`. If you rename by hand, these must stay mutually consistent: manifest `shared_library_path` = `lib<CMake TARGET>.so`; manifest `file_path` = CMake `PLUGIN_NAME` = adapter `KIT_NAME`; composition id = scenario slot = adapter `DEFAULT_COMPOSITION` = `--param` prefix.
