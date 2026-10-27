#!/usr/bin/env python3
"""
Creates a new transport plugin from this template with every name consistently replaced, so the
manifest, CMake target, library file name, composition id, adapter and scenarios cannot drift apart.

    ./new_transport.py --name Acme --out /home/ubuntu/racegen/acme-transport

--name is the CamelCase service/transport name. Derived names (for --name Acme):
    plugin / kit directory / library   PluginAcme  (libPluginAcme.so)
    transport id in manifest           acme
    composition id (= channel gid)     acmeComposition
    service client class               AcmeClient
    environment variables              ACME_SERVER, ACME_TOKEN
    scenario ids / container prefix    acme-send-only, acme-send-recv, ...

Place --out next to the other plugins (a direct child of the racegen directory, or of plugins/):
the adapter and run_scenarios.py find raceboat/ by searching parent directories.
"""

import argparse
import re
import shutil
import sys
from pathlib import Path

TEMPLATE = Path(__file__).resolve().parent
SKIP_DIRS = {"build", "kit", "logs", ".git", "__pycache__"}
SKIP_FILES = {"new_transport.py", "README.md", "DEVELOPER_GUIDE.md"}
TEXT_SUFFIXES = {".h", ".cpp", ".txt", ".json", ".py", ".sh", ".cmake", ".md", ""}


def substitutions(name: str):
    lower = name[0].lower() + name[1:]
    # Order matters: longest/most specific first.
    return [
        ("PluginStub", f"Plugin{name}"),
        ("StubClient", f"{name}Client"),
        ("stubComposition", f"{lower}Composition"),
        ("transport-stub", f"{lower}-transport"),
        ("STUB_", f"{name.upper()}_"),
        ("stub/cursor", f"{lower}/cursor"),
        ("stub-service", f"{lower}-mock-service"),
        ('"stub"', f'"{lower}"'),
        ("stub-", f"{lower}-"),
    ]


def rename(path_name: str, subs) -> str:
    for old, new in subs:
        path_name = path_name.replace(old, new)
    return path_name


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--name", required=True, help="CamelCase transport name, e.g. Acme")
    parser.add_argument("--out", required=True, type=Path, help="new plugin directory (must not exist)")
    args = parser.parse_args()

    if not re.fullmatch(r"[A-Z][A-Za-z0-9]*", args.name):
        sys.exit("--name must be CamelCase, e.g. Acme")
    if args.out.exists():
        sys.exit(f"{args.out} already exists")
    subs = substitutions(args.name)

    for source in sorted(TEMPLATE.rglob("*")):
        relative = source.relative_to(TEMPLATE)
        if (set(relative.parts) & SKIP_DIRS or not source.is_file() or source.name in SKIP_FILES
                or source.suffix not in TEXT_SUFFIXES and source.name not in {".gitignore", "LICENSE"}):
            continue
        destination = args.out / Path(*[rename(part, subs) for part in relative.parts])
        destination.parent.mkdir(parents=True, exist_ok=True)
        try:
            destination.write_text(rename(source.read_text(), subs))
        except UnicodeDecodeError:
            shutil.copy(source, destination)
        shutil.copymode(source, destination)

    (args.out / "README.md").write_text(
        f"# {args.name} transport\n\nGenerated from plugins/transport-stub. See `AGENTS.md`, "
        "`INTEGRATION_TEST_GUIDE.md` and `plugins/transport-stub/DEVELOPER_GUIDE.md` (Step 1 onward) "
        "for what to implement next.\n")
    print(f"created {args.out}")
    print("next: implement the service in source/transport/*Client.cpp, then see DEVELOPER_GUIDE.md")
    return 0


if __name__ == "__main__":
    sys.exit(main())
