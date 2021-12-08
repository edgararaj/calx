#!/bin/sh

set -xe
build_debug/calx.exe > calx.dot
dot -Tsvg calx.dot > calx.svg
