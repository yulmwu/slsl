#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <linux/types.h>

struct event {
    __u64 ts_ns;
    __u32 len;
    __u16 proto;
    __u8 is_ip;
    __u8 _pad[1];
    __u8 saddr[4];
    __u8 daddr[4];
};

static volatile sig_atomic_t exiting;

static void handle_sig(int sig)
{
    (void)sig;
    exiting = 1;
}

static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
    struct event *e = data;
    char src[INET_ADDRSTRLEN] = "-";
    char dst[INET_ADDRSTRLEN] = "-";
    struct in_addr saddr = {0};
    struct in_addr daddr = {0};

    (void)ctx;
    (void)cpu;
    (void)data_sz;

    if (e->is_ip) {
        __builtin_memcpy(&saddr, e->saddr, sizeof(saddr));
        __builtin_memcpy(&daddr, e->daddr, sizeof(daddr));
        inet_ntop(AF_INET, &saddr, src, sizeof(src));
        inet_ntop(AF_INET, &daddr, dst, sizeof(dst));
    }

    printf("len=%u proto=0x%04x %s -> %s\n", e->len, ntohs(e->proto), src, dst);
}

static void handle_lost(void *ctx, int cpu, __u64 lost_cnt)
{
    (void)ctx;
    (void)cpu;
    fprintf(stderr, "lost %llu events\n", (unsigned long long)lost_cnt);
}

int main()
{
    struct bpf_object *obj = NULL;
    struct bpf_program *prog = NULL;
    struct bpf_link *link = NULL;
    struct bpf_map *map = NULL;
    struct perf_buffer *pb = NULL;
    int map_fd;
    struct rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};

    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    if (setrlimit(RLIMIT_MEMLOCK, &rlim)) {
        fprintf(stderr, "failed to set rlimit\n");
        return 1;
    }

    obj = bpf_object__open_file(".build/test.ebpf.o", NULL);
    if (!obj) {
        fprintf(stderr, "failed to open bpf object\n");
        return 1;
    }

    if (bpf_object__load(obj)) {
        fprintf(stderr, "failed to load bpf object\n");
        return 1;
    }

    prog = bpf_object__find_program_by_name(obj, "handle_recv");
    if (!prog) {
        fprintf(stderr, "failed to find program\n");
        return 1;
    }

    link = bpf_program__attach_tracepoint(prog, "net", "netif_receive_skb");
    if (!link) {
        fprintf(stderr, "failed to attach kprobe\n");
        return 1;
    }

    map = bpf_object__find_map_by_name(obj, "events");
    if (!map) {
        fprintf(stderr, "failed to find events map\n");
        return 1;
    }
    map_fd = bpf_map__fd(map);

    pb = perf_buffer__new(map_fd, 8, handle_event, handle_lost, NULL, NULL);
    if (!pb) {
        fprintf(stderr, "failed to create perf buffer\n");
        return 1;
    }

    printf("program loaded and attached\n");

    while (!exiting) {
        int err = perf_buffer__poll(pb, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "perf buffer poll failed: %d\n", err);
            break;
        }
    }

    perf_buffer__free(pb);
    bpf_link__destroy(link);
    bpf_object__close(obj);
    return 0;
}
