WASM_ARCH ?= wasm32
ARCH = $(WASM_ARCH)
CONFIG ?= Release
#EMSCRIPTEN := $(HOME)/emsdk/$(EMSCRIPTEN_VERSION)-$(ARCH)/emscripten

EMSCRIPTEN := $(HOME)/experiment/emsdk/upstream/emscripten


HOST_CC ?= clang
HOST_CFLAGS ?= -W
HOST_CXX ?= clang++
HOST_CXXFLAGS ?= -W

CC = $(EMSCRIPTEN)/emcc
CXX = $(EMSCRIPTEN)/em++
AR = $(EMSCRIPTEN)/emar
RANLIB = $(EMSCRIPTEN)/emranlib
LD = $(EMSCRIPTEN)/emcc

RM ?= rm
CP ?= cp
MKDIR ?= mkdir
MV ?= mv

#
# Top Make file, which we want to trigger a rebuild on if it changes
#
TOP_MAKE := $(word 1,$(MAKEFILE_LIST))


#
# The default target
#
# If no targets are specified on the command-line, the first target listed in
# the makefile becomes the default target.  By convention this is usually called
# the 'all' target.  Here we leave it blank to be first, but define it later
#
all:
.PHONY: all


#
# The install target is used to install built libraries to thier final destination.
#
install:
.PHONY: install


OUTBASE ?= .
CONFIG_DIR := $(ARCH)/$(CONFIG)
OUTDIR := $(OUTBASE)/$(CONFIG_DIR)
STAMPDIR ?= $(OUTDIR)
LIBDIR ?= $(EMSCRIPTEN)/lib


#
# Target to remove temporary files
#
.PHONY: clean
clean:
	$(RM) -rf $(OUTDIR)


#
# Rules for output directories.
#
# Output will be places in a directory name based on Toolchain and configuration
# be default this will be "newlib/Debug".  We use a python wrapped MKDIR to
# proivde a cross platform solution. The use of '|' checks for existance instead
# of timestamp, since the directory can update when files change.
#
%dir.stamp :
	$(call LOG,MKDIR,$@,$(MKDIR) -p $(dir $@))
	@echo Directory Stamp > $@


#
# Common Compile Options
#

ifeq ($(CONFIG),Release)
EMSCRIPTEN_CFLAGS ?= -O3 -I${EMSCRIPTEN}/system/local/include
EMSCRIPTEN_CXXFLAGS ?= -O3 -I${EMSCRIPTEN}/system/local/include
EMSCRIPTEN_LDFLAGS ?= -O1 -L$(EMSCRIPTEN)/system/local/lib
else
EMSCRIPTEN_CFLAGS ?= -O0 -g
EMSCRIPTEN_CXXFLAGS ?= -O0 -g
EMSCRIPTEN_LDFLAGS ?= -O0 -g
endif

EMSCRIPTEN_LDFLAGS += -s ERROR_ON_UNDEFINED_SYMBOLS=0

#
# Default Paths
#
INC_PATHS := $(EXTRA_INC_PATHS)
LIB_PATHS := $(NACL_SDK_ROOT)/lib $(EXTRA_LIB_PATHS)


# Define a LOG macro that allow a command to be run in quiet mode where
# the command echoed is not the same as the actual command executed.
# The primary use case for this is to avoid echoing the full compiler
# and linker command in the default case.  Defining V=1 will restore
# the verbose behavior
#
# $1 = The name of the tool being run
# $2 = The target file being built
# $3 = The full command to run
#
ifdef V
define LOG
$(3)
endef
else
ifeq ($(OSNAME),win)
define LOG
@echo   $(1) $(2) && $(3)
endef
else
define LOG
@echo "  $(1) $(2)" && $(3)
endef
endif
endif


#
# Convert a source path to a object file path.
# If source path is absolute then just use the basename of for the object
# file name (absolute sources paths with the same basename are not allowed).
# For relative paths use the full path to the source in the object file path
# name.
#
# $1 = Source Name
# $2 = Arch suffix
#
define SRC_TO_OBJ
$(if $(filter /%,$(1)), $(OUTDIR)/$(basename $(notdir $(1)))$(2).o, $(OUTDIR)/$(basename $(subst ..,__,$(1)))$(2).o)
endef


#
# Convert a source path to a dependency file path.
# We use the .deps extension for dependencies.  These files are generated by
# fix_deps.py based on the .d files which gcc generates.  We don't reference
# the .d files directly so that we can avoid the the case where the compile
# failed but still generated a .d file (in that case the .d file would not
# be processed by fix_deps.py)
#
# $1 = Source Name
# $2 = Arch suffix
#
define SRC_TO_DEP
$(patsubst %.o,%.deps,$(call SRC_TO_OBJ,$(1),$(2)))
endef

#
# Copy Macro
#
# $1 = Source file name
#
define COPY_RULE
$(OUTDIR)/$(1): $(1) $(OUTDIR)/$(dir $(1))dir.stamp
	$(call LOG,CP   ,$$@,$(CP) $(1) $$@)
endef

#
# Compile Macro
#
# $1 = Source name
# $2 = Compile flags
# $3 = Include directories
#
define C_COMPILER_RULE
-include $(call SRC_TO_DEP,$(1))
$(call SRC_TO_OBJ,$(1)): $(1) $(TOP_MAKE) | $(dir $(call SRC_TO_OBJ,$(1)))dir.stamp
	$(call LOG,CC   ,$$@,$(CC) -o $$@ -c $$< $(EMSCRIPTEN_CFLAGS) $(CFLAGS) $(2))
endef

define CXX_COMPILER_RULE
-include $(call SRC_TO_DEP,$(1))
$(call SRC_TO_OBJ,$(1)): $(1) $(TOP_MAKE) | $(dir $(call SRC_TO_OBJ,$(1)))dir.stamp
	$(call LOG,CXX  ,$$@,$(CXX) -o $$@ -c $$< $(EMSCRIPTEN_CFLAGS) $(CXXFLAGS) $(2))
endef


# $1 = Source Name
# $2 = POSIX Compile Flags
# $3 = Include Directories
define COMPILE_RULE
ifeq ($(suffix $(1)),.c)
$(call C_COMPILER_RULE,$(1),$(2) $(foreach inc,$(INC_PATHS),-I$(inc)) $(3))
else
$(call CXX_COMPILER_RULE,$(1),$(2) $(foreach inc,$(INC_PATHS),-I$(inc)) $(3))
endif
endef


#
# Specific Link Macro
#
# $1 = Target Name
# $2 = List of inputs
# $3 = List of libs
# $4 = List of lib dirs
# $5 = Other Linker Args
# $6 = List of pre-js files
# $7 = List of files to copy
# $8 = HTML template
#
define LINKER_RULE
all: $(1).html
$(1).html: $(2) $(foreach file,$(7),$(OUTDIR)/$(file))
	$(call LOG,LD   ,$$@,$(CC) -o $$@ -o $$@ $(2) $(EMSCRIPTEN_LDFLAGS) $(LDFLAGS) $(foreach path,$(5),-L$(path)) $(foreach lib,$(3),-l$(lib)) $(4) $(foreach js,$(6),--pre-js $(js)) --shell-file $(8))
endef

#
# Generalized Target Macro
#
# $1 = Target Name
# $2 = List of Sources
# $3 = List of LIBS
# $4 = POSIX Linker Switches
# $5 - List of pre-js files
# $6 - HTML template
#
define TARGET_RULE
$(foreach src,$(2),$(eval $(call COMPILE_RULE,$(src),$(CXXFLAGS),$(EXTRA_INC_PATHS))))
$(foreach asset,$(6),$(eval $(call COPY_RULE,$(asset))))
$(call LINKER_RULE,$(OUTDIR)/$(1),$(foreach src,$(2),$(call SRC_TO_OBJ,$(src))),$(3),$(4),$(LIB_PATHS),$(5),,$(6))
endef
