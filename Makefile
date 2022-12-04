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
MV = mv
RM = rm
ZCC = zcc

# $(1) cfile to build
# $(2) object file
# $(3) build subdirectory
# $(4) extra C_FLAGS
define compile_c

$(2): $(3) $(1)
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(ZCC) $(ZCC_ARGS) \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		-c $(1) \
		-o $(2)

endef

define build_lib

$(2): $(3) $(1)
	$(Q)$(ECHO) zcc $1 -o $2
	$(Q)$(ZCC) $(ZCC_ARGS) -x \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		$(1) \
		-o $(2) \
		-create-app

endef

# $(1) target
# $(2) subtarget
# $(3) subtype
# $(4) extra C_FLAGS
define build_app

all: $(1)_$(2)

$(1)_$(2): build/$(1)_$(2) build/$(1)_$(2)/$(1).o $(foreach cfile, $(SHARED_C_FILES), build/$(1)_$(2)/$(patsubst %.c,%.lib,$(cfile)))
	$(Q)$(ECHO) "building " $(1) as $(2)
	$(Q)$(ZCC) $(ZCC_ARGS) --list -m -s --c-code-in-asm \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		$(foreach cfile, $(SHARED_C_FILES), -lbuild/$(1)_$(2)/$(patsubst %.c,%.lib,$(cfile))) \
		build/$(1)_$(2)/$(1).o \
		-o build/$(1)_$(2)/$(1).bin \
		$(3) -create-app
	$(Q)$(LS) -lah build/$(1)_$(2)/$(1)_CODE.bin

build/$(1)_$(2): build
	$(Q)$(MKDIR) -p $$@

$(foreach cf, $(1).c, $(eval $(call compile_c,$(cf),build/$(1)_$(2)/$(patsubst %.c,%.o,$(cf)),build/$(1)_$(2),$(4))))
$(foreach cf, $(SHARED_C_FILES), $(eval $(call build_lib,$(cf),build/$(1)_$(2)/$(patsubst %.c,%.lib,$(cf)),build/$(1)_$(2),$(4))))

endef

define build_all
	$(eval $(call build_app,$(1),tap,"","-DNO_ARGC_ARGV"))
	$(eval $(call build_app,$(1),dot,"-subtype=dot",""))

$(1)_wav: $(1)_tap
	tape2wav build/$(1)_tap/$(1).tap build/$(1)_tap/$(1).wav

$(1)_play: $(1)_wav
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
