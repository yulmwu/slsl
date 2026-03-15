CLANG := clang
BPF_CLANG := clang

ARCH := $(shell uname -m)
INCLUDES := -I/usr/include/$(ARCH)-linux-gnu

BPF_CFLAGS := -O2 -g -target bpf $(INCLUDES)
USER_CFLAGS := -O2 -g
LIBS := -lbpf -lelf

TARGET := hello

all: $(TARGET)

$(TARGET).bpf.o: $(TARGET).bpf.c
	$(BPF_CLANG) $(BPF_CFLAGS) -c $< -o $@

$(TARGET): $(TARGET).c $(TARGET).bpf.o
	$(CLANG) $(USER_CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
