VERSION="v0.0.4"

TARGETS  = catfdd loadfdd cpfdd imgfdd

SHARED_C_FILES = plusd.c fdd_fs_dumpFileList.c fdd_fs_copyFile.c\
				 fdd_fs_writePlus3dosFileHeader.c fdd_fs_loadFileData.c\
				 fdd_fs_loadFile.c fdd_fs_dumpFileInfo.c

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

$(2): $(1)
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(MKDIR) -p $(3)
	$(Q)$(ZCC) $(ZCC_ARGS) \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-c $(1) \
		-o $(2)

endef

define compile_lib

$(2): $(1)
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(MKDIR) -p $(3)
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm\
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-c $(1) \
		-o $(2)
	$(Q)$(MV) $(1).lis $(1).sym build/plusd_lib

endef

# $(1) target
# $(2) subtarget
# $(3) subtype
# $(4) extra C_FLAGS
define build_app

all: build/$(1).$(2)

build/$(1).$(2): build/plusd.lib build/$(1)_$(2)/$(1)
	$(Q)$(CP) build/$(1)_$(2)/$(1) build/$(1).$(2)

build/$(1)_$(2)/$(1): build/plusd.lib $(foreach cf, $(1).c, build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf)))
	$(Q)$(ECHO) "building " $(1) as $(2)
	$(Q)$(MKDIR) -p build/$(1)_$(2)
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-lbuild/plusd.lib \
		$(foreach cf, $(1).c, build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf))) \
		-o build/$(1)_$(2)/$(1) \
		$(3) -create-app
	$(Q)$(LS) -lah build/$(1)_$(2)/$(1)_CODE.bin

$(foreach cf, $(1).c, $(eval $(call compile_c,$(cf),build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf)),build/$(1)_$(2),$(4))))

endef

build/plusd_lib/plusd.lib: $(foreach cfile, $(SHARED_C_FILES), build/plusd_lib/$(patsubst %.c,%.o,$(cfile)))
	$(Q)$(ECHO) "building " $@ as library
	$(Q)$(MKDIR) -p build/plusd_lib
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm \
		$(LDFLAGS) \
		$(CFALGS) $(DEV_CFLAGS)\
		$(foreach cfile, $(SHARED_C_FILES), build/plusd_lib/$(patsubst %.c,%.o,$(cfile))) \
		-o build/plusd_lib/plusd \
		-x -create-app

$(foreach cf, $(SHARED_C_FILES), $(eval $(call compile_lib,$(cf),build/plusd_lib/$(patsubst %.c,%.o,$(cf)),build/plusd_lib,$(4))))

build/plusd.lib: build/plusd_lib/plusd.lib
	$(Q)$(CP) $< $@

define build_all
	$(eval $(call build_app,$(1),tap,"","-DNO_ARGC_ARGV"))
	$(eval $(call build_app,$(1),dot,"-subtype=dot",""))

$(1)_dot: build/$(1).dot

$(1)_tap: build/$(1).tap

$(1)_wav: build/$(1)_tap/$(1).wav

build/$(1)_tap/$(1).wav: build/$(1)_tap/$(1)
	tape2wav build/$(1)_tap/$(1).tap build/$(1)_tap/$(1).wav

$(1)_play: build/$(1)_tap/$(1).wav
	play build/$(1)_tap/$(1).wav
endef

$(foreach target, $(TARGETS), \
	$(eval $(call build_all,$(target)))\
)

build:
	$(Q)$(MKDIR) -p build

clean:
	$(RM) -rf build

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
