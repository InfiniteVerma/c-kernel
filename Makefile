PROJECTS = libc kernel
SYSROOT = $(PWD)/sysroot

NEW_PATH := $(shell ./my_path.sh)
QEMU_SCRIPT := ./qemu.sh
export PATH := $(NEW_PATH):$(PATH)
export PREFIX := /usr
export HOST := $(shell ./default-host.sh)

export AR := $(HOST)-ar
export AS := $(HOST)-as
export CC := $(HOST)-gcc

export EXEC_PREFIX := $(PREFIX)
export BOOTDIR := /boot
export LIBDIR := $(EXEC_PREFIX)/lib
export INCLUDEDIR = $(PREFIX)/include

#export CFLAGS := -O2 -g
export CFLAGS := -g
export CPPFLAGS := 
export CC := $(CC) --sysroot=$(SYSROOT)

# Conditionally export CC based on the HOST variable
ifeq ($(shell echo $(HOST) | grep -Eq -- '-elf$$'),)
	export CC := $(CC) -isystem=$(INCLUDEDIR)
endif

all: headers build iso

debug: CFLAGS += -DDEBUG
debug: all
	$(QEMU_SCRIPT)

test: CFLAGS += -DTEST
test: all
	$(QEMU_SCRIPT)

iso:
	./iso.sh

headers:
	@mkdir -p $(SYSROOT)
	echo $(SYSROOT)
	@for proj in $(PROJECTS); do \
		echo "Installing headers - $$proj..."; \
		DESTDIR=$(SYSROOT) $(MAKE) -C $$proj install-headers; \
	done

build: headers
	@for proj in $(PROJECTS); do \
		DESTDIR=$(SYSROOT) $(MAKE) -C $$proj install; \
	done

clean:
	echo $(PATH)
	@for proj in $(PROJECTS); do \
		echo "Cleaning in $$proj..."; \
		$(MAKE) -C $$proj clean; \
	done
	@rm -rf sysroot
	@rm -rf isodir
	@rm -rf myos.iso
