VERSION="v0.0.4"

TARGETS  = catfdd loadfdd cpfdd imgfdd

SHARED_C_FILES = plusd.c fdd_fs.c

ZCC_ARGS = +zx -vn --list -m -s
LDFLAGS = -clib=sdcc_iy -startup=30
CFALGS = -DVERSION=$(VERSION) -SO3 --max-allocs-per-node200000 --opt-code-size
#DEV_CFLAGS=-DUNMODIFIED_PLUSD

#-----------------------------------------------------------

export PATH := $(PATH):$(HOME)/retroPc/src/z80/z88dk/bin
export ZCCCFG ?= $(HOME)/retroPc/src/z80/z88dk/lib/config

Q ?= @
ECHO = echo
LS = ls
MKDIR = mkdir
MV = mv
ZCC = zcc

all: clean

# $(0) target
# $(1) subtarget
# $(2) subtype
# $(3) extra C_FLAGS
define build_app
all: $(1)_$(2)
$(1)_$(2):
	$(Q)$(ECHO) "building " $(1) as $(2)
	$(Q)$(MKDIR) -p build/$(1)_$(2)
	$(Q)$(ZCC) $(ZCC_ARGS) \
		$(LDFLAGS) \
		$(CFALGS) $(4) $(DEV_CFLAGS)\
		$(1).c $(SHARED_C_FILES) \
		-o build/$(1)_$(2)/$(1) \
		$(3) -create-app
	$(Q)$(MV) *.lis *.sym build/$(1)_$(2)
	$(Q)$(LS) -lah build/$(1)_$(2)/$(1)_CODE.bin
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

clean:
	$(RM) -rf build

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
