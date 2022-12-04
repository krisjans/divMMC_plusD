VERSION="v0.0.4"

TARGETS  = catfdd loadfdd cpfdd imgfdd

SHARED_C_FILES = $(wildcard libfdd_fs/*.c)

SHARED_H_FILES = $(wildcard *.h) $(wildcard libfdd_fs/*.h)

ZCC_ARGS = +zx -vn
LDFLAGS = -clib=sdcc_iy -startup=30
CFALGS = -DVERSION=$(VERSION) -SO3 --max-allocs-per-node200000 --opt-code-size
#DEV_CFLAGS=-DUNMODIFIED_PLUSD

#-----------------------------------------------------------

all:
	$(Q)$(ECHO) All done;

export PATH := $(PATH):$(HOME)/retroPc/src/z80/z88dk/bin
export ZCCCFG ?= $(HOME)/retroPc/src/z80/z88dk/lib/config

Q ?= @
ECHO = echo
LS = ls
MKDIR = mkdir
CP = cp
MV = mv
RM = rm
ZCC = zcc

# $(1) cfile to build
# $(2) object file
# $(3) build subdirectory
# $(4) extra C_FLAGS
define compile_c

$(2): $(1) $(SHARED_H_FILES) Makefile
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(MKDIR) -p $(dir $(2))
	$(Q)$(ZCC) $(ZCC_ARGS) \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-c $(1) \
		-o $(2)

endef

define compile_lib

$(2): $(1) $(SHARED_H_FILES) Makefile
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(MKDIR) -p $(dir $(2))
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm\
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-c $(1) \
		-o $(2)
	$(Q)$(MV) $(1).lis $(1).sym $(dir $(2))

endef

# $(1) target
# $(2) subtarget
# $(3) subtype
# $(4) extra C_FLAGS
# $(5) final binary file
define build_app

all: build/$(5)

build/$(5): build/plusd.lib build/$(1)_$(2)/$(5)
	$(Q)$(CP) build/$(1)_$(2)/$(5) build/$(5)
	$(Q)$(LS) -lah build/$(5)

build/$(1)_$(2)/$(5): build/plusd.lib $(foreach cf, $(1).c, build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf)))
	$(Q)$(ECHO) "building " $(1) as $(2) $(5)
	$(Q)$(MKDIR) -p build/$(1)_$(2)
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-lbuild/plusd.lib \
		$(foreach cf, $(1).c, build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf))) \
		-o build/$(1)_$(2)/$(1) \
		$(3) -create-app

$(foreach cf, $(1).c, $(eval $(call compile_c,$(cf),build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf)),build/$(1)_$(2),$(4))))

endef

build/libfdd_fs/plusd.lib: $(foreach cfile, $(SHARED_C_FILES), build/$(patsubst %.c,%.o,$(cfile)))
	$(Q)$(ECHO) "building " $@ as library
	$(Q)$(MKDIR) -p build/libfdd_fs
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm \
		$(LDFLAGS) \
		$(CFALGS) $(DEV_CFLAGS)\
		$(foreach cfile, $(SHARED_C_FILES), build/$(patsubst %.c,%.o,$(cfile))) \
		-o build/libfdd_fs/plusd \
		-x -create-app

$(foreach cf, $(SHARED_C_FILES), $(eval $(call compile_lib,$(cf),build/$(patsubst %.c,%.o,$(cf)),build/libfdd_fs,$(4))))

build/plusd.lib: build/libfdd_fs/plusd.lib
	$(Q)$(CP) $< $@

define build_all
	$(eval $(call build_app,$(1),tap,"","-DNO_ARGC_ARGV",$(1).tap))
	$(eval $(call build_app,$(1),dot,"-subtype=dot","",$(shell echo $(1) | tr '[:lower:]' '[:upper:]')))

$(1)_dot: build/$(shell echo $(1) | tr '[:lower:]' '[:upper:]')

$(1)_tap: build/$(1).tap

$(1)_wav: build/$(1)_tap/$(1).wav

build/$(1)_tap/$(1).wav: build/$(1).tap
	tape2wav build/$(1).tap build/$(1)_tap/$(1).wav

$(1)_play: build/$(1)_tap/$(1).wav
	play build/$(1)_tap/$(1).wav
endef

$(foreach target, $(TARGETS), \
	$(eval $(call build_all,$(target)))\
)

clean:
	$(RM) -rf build

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
