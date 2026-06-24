#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* 判断 n 是否为素数（试除法，只检查到 sqrt(n)） */
bool is_prime(long n) {
    if (n < 2)  return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    long limit = (long)sqrt(n);
    for (long i = 3; i <= limit; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

/* 统计 [start, end] 区间内的素数个数 */
long count_primes(long start, long end) {
    long cnt = 0;
    for (long i = start; i <= end; i++) {
        if (is_prime(i)) cnt++;
    }
    return cnt;
}

/* 获取当前时间（秒） */
double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* 获取 CPU 核心数 */
long get_cpu_cores(void) {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

int main(int argc, char *argv[]) {
    /* 默认测试三个数据规模 */
    long N_vals[] = {10000000L, 50000000L, 100000000L};
    const char *N_str[] = {"10^7", "5x10^7", "10^8"};
    int ntests = 3;

    /* 如果命令行指定了 N，则只测该规模 */
    if (argc > 1) {
        N_vals[0] = atol(argv[1]);
        N_str[0]  = argv[1];
        ntests     = 1;
    }

    printf("============================================================\n");
    printf("  Prime Counting  -  Single-Threaded\n");
    printf("  CPU cores : %ld\n", get_cpu_cores());
    printf("============================================================\n\n");

    for (int t = 0; t < ntests; t++) {
        long N = N_vals[t];

        printf("  Data Size: N = %ld (%s)\n", N, N_str[t]);
        printf("  Counting primes in [2, %ld] ...\n", N);

        double t0 = now_sec();
        long total = count_primes(2, N);
        double t1 = now_sec();

        printf("  Prime Count : %ld\n", total);
        printf("  Runtime     : %.6f  seconds\n", t1 - t0);
        printf("------------------------------------------------------------\n");
    }

    printf("\nDone.\n");
    return 0;
}
