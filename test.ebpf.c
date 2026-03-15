// Currently, netif_receive_skb is used. This is scheduled to change in the future.

#include "vmlinux.h"
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#define ETH_P_IP 0x0800

struct event {
    __u64 ts_ns;
    __u32 len;
    __u16 proto;
    __u8 is_ip;
    __u8 _pad[1];
    __u8 saddr[4];
    __u8 daddr[4];
};

struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
    __uint(max_entries, 128);
} events SEC(".maps");

struct trace_event_raw_netif_receive_skb {
    __u16 common_type;
    __u8 common_flags;
    __u8 common_preempt_count;
    __s32 common_pid;
    __u64 skbaddr;
    __u32 len;
    __u32 __data_loc_name;
};

SEC("tracepoint/net/netif_receive_skb")
int handle_recv(struct trace_event_raw_netif_receive_skb *ctx)
{
    struct event e = {};
    void *data = NULL;
    struct sk_buff *skb = (struct sk_buff *)ctx->skbaddr;

    e.ts_ns = bpf_ktime_get_ns();
    e.len = ctx->len;
    e.proto = BPF_CORE_READ(skb, protocol);

    if (e.proto == bpf_htons(ETH_P_IP)) {
        struct iphdr iph;

        data = (void *)BPF_CORE_READ(skb, data);
        if (data && bpf_probe_read_kernel(&iph, sizeof(iph), data) == 0) {
            __builtin_memcpy(e.saddr, &iph.saddr, sizeof(e.saddr));
            __builtin_memcpy(e.daddr, &iph.daddr, sizeof(e.daddr));
            e.is_ip = 1;
        }
    }

    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
