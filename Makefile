CLANG := clang
BPF_CLANG := clang

ARCH := $(shell uname -m)
KARCH := $(shell uname -m | sed -e 's/x86_64/x86/' -e 's/aarch64/arm64/')
BUILD_DIR := .build
VMLINUX_DIR := headers
VMLINUX_H := $(VMLINUX_DIR)/vmlinux.h
INCLUDES := -I. -I$(VMLINUX_DIR) -I/usr/include/$(ARCH)-linux-gnu

BPF_CFLAGS := -O2 -g -target bpf $(INCLUDES) -D__TARGET_ARCH_$(KARCH)
USER_CFLAGS := -O2 -g
LIBS := -lbpf -lelf

TARGET := test
BPF_OBJ := $(BUILD_DIR)/$(TARGET).ebpf.o
BIN := $(BUILD_DIR)/$(TARGET)

all: $(BIN)

vmlinux: $(VMLINUX_H)

$(VMLINUX_H):
	mkdir -p $(VMLINUX_DIR)
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $(VMLINUX_H)

$(BPF_OBJ): $(TARGET).ebpf.c
	mkdir -p $(BUILD_DIR)
	$(BPF_CLANG) $(BPF_CFLAGS) -c $< -o $@

$(BIN): $(TARGET).c $(BPF_OBJ)
	mkdir -p $(BUILD_DIR)
	$(CLANG) $(USER_CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean vmlinux
