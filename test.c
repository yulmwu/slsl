#include <stdio.h>
#include <unistd.h>
#include <bpf/libbpf.h>

int main()
{
    struct bpf_object *obj;
    struct bpf_program *prog;
    struct bpf_link *link;

    obj = bpf_object__open_file("hello.ebpf.o", NULL);
    if (!obj) {
        printf("failed to open bpf object\n");
        return 1;
    }

    if (bpf_object__load(obj)) {
        printf("failed to load bpf object\n");
        return 1;
    }

    prog = bpf_object__find_program_by_name(obj, "hello");
    if (!prog) {
        printf("failed to find program\n");
        return 1;
    }

    link = bpf_program__attach_tracepoint(prog, "syscalls", "sys_enter_execve");
    if (!link) {
        printf("failed to attach\n");
        return 1;
    }

    printf("program loaded and attached\n");

    while (1)
        sleep(1);
}
