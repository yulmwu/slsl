> [!NOTE]
> 
> The project has not started yet. this is work for a prototype.

## Introduction and Setup

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

## Example

```shell
ping -c 1 1.1.1.1
ping -c 1 google.com
```

```shell
program loaded and attached

len=84 proto=0x0800 1.1.1.1 -> 192.168.5.15

len=67 proto=0x0800 127.0.0.1 -> 127.0.0.53
len=67 proto=0x0800 127.0.0.1 -> 127.0.0.53
len=56 proto=0x0800 192.168.5.2 -> 192.168.5.15
len=56 proto=0x0800 192.168.5.2 -> 192.168.5.15
len=67 proto=0x0800 127.0.0.53 -> 127.0.0.1
len=212 proto=0x0800 192.168.5.2 -> 192.168.5.15
len=212 proto=0x0800 192.168.5.2 -> 192.168.5.15
len=163 proto=0x0800 127.0.0.53 -> 127.0.0.1
len=84 proto=0x0800 142.251.118.113 -> 192.168.5.15
len=85 proto=0x0800 127.0.0.1 -> 127.0.0.53
len=74 proto=0x0800 192.168.5.2 -> 192.168.5.15
len=85 proto=0x0800 127.0.0.53 -> 127.0.0.1
```

## License

The kernel running eBPF is licensed under the GPL. but the user-space program can be licensed under MIT or other licenses. 
