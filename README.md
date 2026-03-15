```shell
sudo apt update
sudo apt install -y clang llvm libbpf-dev libelf-dev bpftool build-essential linux-tools-common linux-tools-generic

clang --version
bpftool version

# ls /sys/kernel/btf/vmlinux
# ls /usr/include/*/asm/types.h
# if not exist, sudo apt install linux-headers-$(uname -r)
```

```shell
make clean
make

sudo ./test
# sudo cat /sys/kernel/tracing/trace_pipe
```