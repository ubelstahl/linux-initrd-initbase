###############################################################################
#                                                                             #
#           linux-initrd-initbase, Base init program for initramfs            #
#               Open-Source, Public Domain (CC0 1.0 Universal)                #
#                                                                             #
#                               2026 ubelstahl.                               #
#                                                                             #
###############################################################################

	PROJECT_NAME   := linux-initrd-initbase
	PROJECT_DESC   := Base init program for initramfs
	PROJECT_AUTHOR := ubelstahl
	PROJECT_SOURCE := https://github.com/ubelstahl/linux-initrd-initbase

###############################################################################

	TARGET_DIRECTORY  := dist
	TARGET_EXECUTABLE := init

###############################################################################

	SOURCES := $(wildcard src/*.c)

	INCLUDES := \
		inc \
		src

	LIBRARIES :=

###############################################################################

	BUSYBOX_EXIT_MEANS_REBOOT := false

###############################################################################

	CC      := musl-gcc
	CFLAGS  := -Wall -Wextra -Os -fPIC -fpermissive
	LDFLAGS := -static -L/usr/lib/musl/lib/ -L/usr/lib/

###############################################################################

ANSI_BOLD := $(shell printf "\e[1;98m")
ANSI_GRAY  := $(shell printf "\e[0;90m")
ANSI_RESET := $(shell printf "\e[0m")

TXT_LD    := "$(ANSI_BOLD)LD       $(ANSI_RESET)"
TXT_CC    := "$(ANSI_BOLD)CC       $(ANSI_RESET)"
TXT_COPY  := "$(ANSI_BOLD)COPY     $(ANSI_RESET)"
TXT_MKDIR := "$(ANSI_BOLD)MKDIR    $(ANSI_RESET)"
TXT_RM    := "$(ANSI_BOLD)RM       $(ANSI_RESET)"
TXT_TOUCH := "$(ANSI_BOLD)TOUCH    $(ANSI_RESET)"
TXT_EMPTY := "         "
PAD_ZERO  := "    "

TMPDIR := $(TARGET_DIRECTORY)/.buildcache
IDXFIL := $(TMPDIR)/.index

BUILD_PATH := $(TARGET_DIRECTORY)/$(TARGET_EXECUTABLE)

OBJS    := $(SOURCES:%=$(TMPDIR)/%.o)
LDFLAGS += $(addprefix -l, $(LIBRARIES))
CFLAGS  += -MMD -MP \
	"-DPROJECT_NAME=\"$(PROJECT_NAME)\"" \
	"-DPROJECT_DESC=\"$(PROJECT_DESC)\"" \
	"-DPROJECT_VERSION=\"$(shell date '+%Y.%j.%H%M%S')\"" \
	"-DPROJECT_AUTHOR=\"$(PROJECT_AUTHOR)\"" \
	"-DPROJECT_SOURCE=\"$(PROJECT_SOURCE)\"" \
	$(addprefix -I, $(INCLUDES))

ifeq ($(BUSYBOX_EXIT_MEANS_REBOOT),true)
    CFLAGS += -DBUSYBOX_EXIT_MEANS_REBOOT=1
endif

TOTAL_OBJ := $(words $(OBJS) $(BUILD_PATH))

.PHONY: info help build clean $(BUILD_PATH)
all: build

help:
	@echo "$(PROJECT_NAME), $(PROJECT_DESC)"
	@echo -e "Open-Source & Public Domain. 2026 $(PROJECT_AUTHOR).\n"
	@echo -e "Usage: make [...commands] (default: build)\n"
	@echo "Commands:"
	@echo "    help      Prints this exact help page"
	@echo "    build     Builds the library"
	@echo "    clean     Deletes all build artifacts"

clean:
	@echo $(PAD_ZERO) $(TXT_RM) "$(TARGET_DIRECTORY)"
	@rm -rf "$(TARGET_DIRECTORY)"

__prep: $(TMPDIR)
	@echo "0" > "$(IDXFIL)"

build: __prep $(BUILD_PATH)
	@rm -f "$(IDXFIL)"

$(TARGET_DIRECTORY):
	@echo $(PAD_ZERO) $(TXT_MKDIR) "$(TARGET_DIRECTORY)"
	@mkdir -p "$(TARGET_DIRECTORY)"

$(TMPDIR): $(TARGET_DIRECTORY)
	@echo $(PAD_ZERO) $(TXT_MKDIR) "$(TMPDIR)"
	@mkdir -p "$(TMPDIR)"

$(TMPDIR)/%.c.o: %.c
	@mkdir -p "$(dir $@)"
	@echo $$(($$(cat "$(IDXFIL)" 2>/dev/null || \
	    echo -1) + 1)) > "$(IDXFIL)"
	@\
	    percent=$$(($$(cat "$(IDXFIL)") * 100 / $(TOTAL_OBJ))); \
	    printf "$(ANSI_GRAY)%3d%%$(ANSI_RESET) " "$$percent"; \
	    echo $(TXT_CC) "$<"; \
	    $(CC) $(CFLAGS) -c "$<" -o "$@" || exit 1

$(BUILD_PATH): $(OBJS)
	@echo $$(($$(cat "$(IDXFIL)" 2>/dev/null || \
	    echo -1) + 1)) > "$(IDXFIL)"
	@\
	    percent=$$(($$(cat "$(IDXFIL)") * 100 / $(TOTAL_OBJ))); \
	    printf "$(ANSI_GRAY)%3d%%$(ANSI_RESET) " "$$percent"; \
	    echo $(TXT_LD) "$@"
	@mkdir -p "$(dir $@)"
	@$(CC) $(LDFLAGS) $(OBJS) -o $@ || exit 1

ALL_DEPS := $(OBJS:.o=.d)
-include $(wildcard $(ALL_DEPS))