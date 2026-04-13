#!/usr/bin/env bash

set -euo pipefail

profile="dev"

if [[ "${1:-}" == "release" || "${1:-}" == "dev" ]]; then
  profile="$1"
  shift
fi

if [[ "${1:-}" == "--" ]]; then
  shift
fi

scripts/ensure-pebble-sdk.sh
bash scripts/prepare-package.sh "$profile"
pebble build "$@"

if [[ "$profile" == "dev" ]]; then
  # Dynamically find the .pbw file in the build directory
  generated_pbw=$(find build -maxdepth 1 -name "*.pbw" ! -name "*-dev.pbw" | head -n 1)
  if [[ -n "$generated_pbw" ]]; then
    cp "$generated_pbw" build/forecaswatch2-dev.pbw
  else
    echo "Error: No .pbw file found in build directory" >&2
    exit 1
  fi
fi
