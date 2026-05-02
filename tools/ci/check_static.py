#!/usr/bin/env python3
"""Lightweight static checks for ReforgerInventoryPlus.

These checks are intentionally dependency-free so they can run in CI
without Arma Reforger Workbench.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def fail(msg: str) -> None:
    print(f"ERROR: {msg}")
    sys.exit(1)


def check_constants() -> None:
    constants_path = ROOT / "Scripts/Game/RIP_Constants.c"
    text = constants_path.read_text(encoding="utf-8")

    medium_match = re.search(r"CAPACITY_THRESHOLD_MEDIUM\s*=\s*([0-9.]+)", text)
    high_match = re.search(r"CAPACITY_THRESHOLD_HIGH\s*=\s*([0-9.]+)", text)

    if not medium_match or not high_match:
        fail("Could not parse capacity thresholds from RIP_Constants.c")

    medium = float(medium_match.group(1))
    high = float(high_match.group(1))

    if not (0.0 <= medium <= 1.0 and 0.0 <= high <= 1.0):
        fail("Capacity thresholds must stay within [0.0, 1.0]")

    if not (medium < high):
        fail("CAPACITY_THRESHOLD_MEDIUM must be lower than CAPACITY_THRESHOLD_HIGH")


def check_config_uses_constants() -> None:
    config_path = ROOT / "Scripts/Game/RIP_Config.c"
    text = config_path.read_text(encoding="utf-8")

    required = [
        "RIP_Constants.VICINITY_RANGE_DEFAULT",
        "RIP_Constants.VICINITY_REFRESH_RATE",
        "RIP_Constants.SEARCH_DEBOUNCE_MS",
        "RIP_Constants.TRANSFER_BATCH_SIZE",
    ]

    missing = [token for token in required if token not in text]
    if missing:
        fail(f"RIP_Config.c must use constant-backed defaults. Missing: {', '.join(missing)}")


def check_rip_prefix() -> None:
    script_dir = ROOT / "Scripts/Game"
    for path in sorted(script_dir.glob("*.c")):
        text = path.read_text(encoding="utf-8")
        for class_name in re.findall(r"\bclass\s+([A-Za-z_][A-Za-z0-9_]*)", text):
            if not class_name.startswith("RIP_"):
                fail(f"Class '{class_name}' in {path.relative_to(ROOT)} must use RIP_ prefix")


def main() -> None:
    check_constants()
    check_config_uses_constants()
    check_rip_prefix()
    print("All static checks passed.")


if __name__ == "__main__":
    main()
