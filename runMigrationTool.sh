#!/bin/bash

export LD_LIBRARY_PATH=out/lib:out/lib64
out/bin/migrationTool -c conf/migration.conf -r prod $@
