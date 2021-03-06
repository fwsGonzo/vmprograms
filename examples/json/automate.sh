#!/usr/bin/env bash
set -e
file="${1-build/jsonmin}"
tenant="jpizza.com"
host="http://127.00.1:8080"
key="12daf155b8508edc4a4b8002264d7494"

echo "Sending $file to tenant $tenant at $host"
curl -H "X-PostKey: $key" -H "Host: $tenant" --data-binary "@$file" -X POST $host
