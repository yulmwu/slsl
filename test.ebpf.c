#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

SEC("tracepoint/syscalls/sys_enter_execve")
int hello(void *ctx)
{
    bpf_printk("hello from ebpf\n");
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
