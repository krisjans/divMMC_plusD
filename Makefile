export PATH := $(PATH):$(HOME)/retroPc/src/z80/z88dk/bin
export ZCCCFG ?= $(HOME)/retroPc/src/z80/z88dk/lib/config

SHARED_C_FILES=plusd.c fdd_fs.c
CAT_C_FILES=catfdd.c $(SHARED_C_FILES)
LOAD_C_FILES=loadfdd.c $(SHARED_C_FILES)
CP_C_FILES=cpfdd.c $(SHARED_C_FILES)

all: clean catfdd_dot catfdd_tap loadfdd_dot loadfdd_tap cpfdd_dot cpfdd_tap

clean:
	rm -rf build

catfdd_tap: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		-DNO_ARGC_ARGV\
		$(CAT_C_FILES) -o build/catfdd -create-app
	mv *.lis build/
	ls -lah build/catfdd_CODE.bin

catfdd_wav: catfdd_tap
	tape2wav build/catfdd.tap build/catfdd.wav

catfdd_play: catfdd_wav
	play build/catfdd.wav

catfdd_dot: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		$(CAT_C_FILES) -o build/catfdd -subtype=dot -create-app
	mv *.lis build/
	ls -lah build/catfdd_CODE.bin
	ls -lah build/CATFDD

loadfdd_tap: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		-DNO_ARGC_ARGV\
		$(LOAD_C_FILES) -o build/loadfdd -create-app
	mv *.lis build/
	ls -lah build/loadfdd_CODE.bin

loadfdd_wav: loadfdd_tap
	tape2wav build/loadfdd.tap build/loadfdd.wav

loadfdd_play: loadfdd_wav
	play build/loadfdd.wav

loadfdd_dot: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		$(LOAD_C_FILES) -o build/loadfdd -subtype=dot -create-app
	mv *.lis build/
	ls -lah build/loadfdd_CODE.bin
	ls -lah build/LOADFDD

cpfdd_tap: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		-DNO_ARGC_ARGV\
		$(CP_C_FILES) -o build/cpfdd -create-app
	mv *.lis build/
	ls -lah build/cpfdd_CODE.bin

cpfdd_wav: cpfdd_tap
	tape2wav build/cpfdd.tap build/cpfdd.wav

cpfdd_play: cpfdd_wav
	play build/cpfdd.wav

cpfdd_dot: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		$(CP_C_FILES) -o build/cpfdd -subtype=dot -create-app
	mv *.lis build/
	ls -lah build/cpfdd_CODE.bin
	ls -lah build/CPFDD

build:
	mkdir -p build
