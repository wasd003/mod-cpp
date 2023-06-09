.PHONY: build run gdb release

build:
	./scripts/build.sh Debug

release:
	./scripts/build.sh Release

gdb: build
	./scripts/gdb.sh

run: build
	./scripts/run.sh
