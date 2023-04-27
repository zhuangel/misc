#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/kvm.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <string.h>
#include<sys/wait.h>

#define VM_MODE_BACKGROUND 0
#define VM_MODE_RUNNING 1

#define BACK_VM_CNT 900
#define TEST_VM_CNT 20
#define MAX_VM_CNT (BACK_VM_CNT+TEST_VM_CNT)

#define IRQFD_PER_DEV 3
#define DEVICE_PER_VM 5
#define IRQFD_PER_VM (IRQFD_PER_DEV*DEVICE_PER_VM)

#define BACK_VM_SLEEP 60*1000*1000
#define TEST_VM_SLEEP 400*1000

struct teststat {
        int mode;
        unsigned long long max;
        unsigned long long min;
        unsigned long long tot;
        unsigned long long avg;
};

int kvmfd = -1;
int process_idx = 0;
struct teststat stats[MAX_VM_CNT];

static __inline__ unsigned long long rdtsc(void)
{
        unsigned hi, lo;

        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));

        return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void *kvm_test(void *arg) {
        int vmfd, ret, irq, idx, sleep, loop;
        int efd[IRQFD_PER_VM];
        struct kvm_irqfd irqfd;
        struct teststat *st = &stats[process_idx];
        unsigned long long start, end, cost;

        if (process_idx < BACK_VM_CNT) {
                st->mode = VM_MODE_BACKGROUND;
                loop = 1;
                sleep = BACK_VM_SLEEP;
        } else {
                st->mode = VM_MODE_RUNNING;
                st->min = 0xffffffff;
                loop = 100;
                sleep = TEST_VM_SLEEP;
        }


        for (idx=0; idx<loop; idx++) {
                vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
                if (vmfd <= 0) {
                        printf("create vm failed: %d, %d\n", vmfd, errno);
                        return NULL;
                }
                ioctl(vmfd, KVM_CREATE_IRQCHIP);
                for (irq=0;irq<IRQFD_PER_VM;irq++) {
                        efd[irq] = eventfd(irq+32, EFD_NONBLOCK);
                        memset(&irqfd,0,sizeof(struct kvm_irqfd));
                        irqfd.fd = efd[irq];
                        irqfd.gsi = irq + 32;
                        irqfd.flags = 0;
                        start = rdtsc();
                        ioctl(vmfd, KVM_IRQFD, &irqfd);
                        end = rdtsc();
                        cost = end - start;
                        if (st->min == -1 || cost < st->min) {
                                st->min = cost;
                        }
                        if (cost > st->max) {
                                st->max = cost;
                        }
                        st->tot += cost;
                }
                usleep(sleep);
                for (irq=0;irq<IRQFD_PER_VM;irq++) {
                        memset(&irqfd,0,sizeof(struct kvm_irqfd));
                        irqfd.fd = efd[irq];
                        irqfd.gsi = irq + 32;
                        irqfd.flags = KVM_IRQFD_FLAG_DEASSIGN;
                        ioctl(vmfd, KVM_IRQFD, &irqfd);
                        close(efd[irq]);
                }
                close(vmfd);
        }
        st->avg = st->tot / (loop*IRQFD_PER_VM);
        if (process_idx >= BACK_VM_CNT && process_idx < MAX_VM_CNT) {
                printf("idx %llu min %llu max %llu avg %llu\n",process_idx, st->min, st->max, st->avg);
        }

        return NULL;
}


int main() {
        int idx, ret, stat;
        int mode[MAX_VM_CNT];
        pthread_t test[MAX_VM_CNT];
        pid_t childs[MAX_VM_CNT];
        struct teststat *st;

        kvmfd = open("/dev/kvm", O_RDONLY);
        if (kvmfd < 0) {
                printf("open failed: %d, %d\n", kvmfd, errno);
                return 0;
        }

        for (idx = 0; idx<MAX_VM_CNT; idx++) {
                if (idx==BACK_VM_CNT) {
                        printf("sleep 10 seconds to wait backgroud VM create\n");
                        sleep(10);
                }
                process_idx = idx;
                childs[idx] = fork();
                if (childs[idx] == 0) {
                        kvm_test(NULL);
                        exit(0);
                }
                continue;
        }

        for (idx=0; idx<MAX_VM_CNT; idx++) {
                waitpid(childs[idx], &stat, 0);
        }

        return 0;
}
