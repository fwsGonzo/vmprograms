#!/usr/bin/env bash
set -e
tmpfile=$(mktemp /tmp/abc-script.XXXXXX)

source="$1"
tenant="${2-xpizza.com}"

gcc -static -O2 $source -o "$tmpfile"

for i in {1..5000}
do
	./upload.sh "$tmpfile" "$tenant"
done

rm -f "$tmpfile"
