export PATH := $(PATH):$(HOME)/retroPc/src/z80/z88dk/bin
export ZCCCFG ?= $(HOME)/retroPc/src/z80/z88dk/lib/config

all: clean catfdd_dot catfdd_tap

clean:
	rm -rf build

catfdd_tap: build
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		catfdd.c -o build/catfdd -create-app
	mv *.lis build/
	ls -lah build/catfdd_CODE.bin

catfdd_wav: catfdd_tap
	tape2wav build/catfdd.tap build/catfdd.wav

catfdd_play: catfdd_wav
	play build/catfdd.wav

catfdd_dot: build
	# -vn 
	zcc +zx -vn --list -clib=sdcc_iy -startup=30\
		-SO3 --max-allocs-per-node200000 --opt-code-size\
		catfdd.c -o build/catfdd -subtype=dot -create-app
	mv *.lis build/
	ls -lah build/catfdd_CODE.bin
	ls -lah build/CATFDD

build:
	mkdir -p build