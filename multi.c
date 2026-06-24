#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* ---------- 素数判定 ---------- */
static inline bool is_prime(long n) {
    if (n < 2)  return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    long limit = (long)sqrt(n);
    for (long i = 3; i <= limit; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

/* ---------- 单区间统计 ---------- */
static long count_primes(long start, long end) {
    long cnt = 0;
    for (long i = start; i <= end; i++) {
        if (is_prime(i)) cnt++;
    }
    return cnt;
}

/* ---------- 工作线程参数 ---------- */
typedef struct {
    long start;
    long end;
    long count;
} ThreadArg;

/* ---------- 线程入口 ---------- */
static void *worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    a->count = count_primes(a->start, a->end);
    return NULL;
}

/* ---------- 计时 ---------- */
static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* ---------- 获取 CPU 核心数 ---------- */
static long get_cpu_cores(void) {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

/* ---------- 多线程版素数统计 ---------- */
static long count_primes_mt(long N, int nthreads, double *elapsed) {
    pthread_t *tids  = malloc((size_t)nthreads * sizeof(pthread_t));
    ThreadArg *args  = malloc((size_t)nthreads * sizeof(ThreadArg));

    /* 将 [2, N] 尽量均分给各线程 */
    long range = N - 1;
    long chunk = range / nthreads;
    long rem   = range % nthreads;

    long cur = 2;
    for (int i = 0; i < nthreads; i++) {
        args[i].start = cur;
        long extra = (i < rem) ? 1 : 0;
        args[i].end = cur + chunk + extra - 1;
        if (i == nthreads - 1) args[i].end = N;
        args[i].count = 0;
        cur = args[i].end + 1;
    }

    /* 计时开始 */
    double t0 = now_sec();
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&tids[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < nthreads; i++) {
        pthread_join(tids[i], NULL);
    }
    double t1 = now_sec();
    *elapsed = t1 - t0;

    /* 汇总 */
    long total = 0;
    for (int i = 0; i < nthreads; i++) {
        total += args[i].count;
    }

    free(tids);
    free(args);
    return total;
}

/* ---------- 主函数 ---------- */
int main(int argc, char *argv[]) {
    /* 测试规模 */
    long N_vals[] = {10000000L, 50000000L, 100000000L};
    const char *N_str[] = {"10^7", "5x10^7", "10^8"};
    int ntests = 3;

    /* 线程数列表 */
    int thr_counts[] = {1, 2, 4, 8, 16};
    int nthr_tests   = 5;

    /* CPU 核心数 */
    long ncores = get_cpu_cores();

    /* 命令行可指定 N */
    if (argc > 1) {
        N_vals[0] = atol(argv[1]);
        N_str[0]  = argv[1];
        ntests    = 1;
    }

    printf("============================================================\n");
    printf("  Prime Counting  -  Multi-Threaded  (pthread)\n");
    printf("============================================================\n");
    printf("  CPU cores detected : %ld\n", ncores);
    printf("============================================================\n\n");

    /* 逐个数据规模测试 */
    for (int t = 0; t < ntests; t++) {
        long N = N_vals[t];

        /* ----- 单线程基线（纯循环，无 pthread 开销）----- */
        double base_time;
        long   base_cnt;
        {
            printf("[Baseline] Single-thread (no pthread), N = %ld (%s) ...\n",
                   N, N_str[t]);
            double t0 = now_sec();
            base_cnt  = count_primes(2, N);
            double t1 = now_sec();
            base_time = t1 - t0;
            printf("  -> primes = %ld,  time = %.6f s\n\n", base_cnt, base_time);
        }

        /* ----- 多线程测试 ----- */
        printf("  %-10s %-15s %-15s %-10s\n",
               "Threads", "Runtime(s)", "Primes", "Speedup");
        printf("  -------------------------------------------------\n");

        /* 先打印基线行（speedup = 1.0） */
        printf("  %-10s %-15.6f %-15ld %-10.2f\n",
               "[base]", base_time, base_cnt, 1.0);

        for (int i = 0; i < nthr_tests; i++) {
            int nthr = thr_counts[i];
            double mt_time;
            long mt_cnt = count_primes_mt(N, nthr, &mt_time);
            double speedup = base_time / mt_time;

            printf("  %-10d %-15.6f %-15ld %-10.2f\n",
                   nthr, mt_time, mt_cnt, speedup);
        }

        printf("\n============================================================\n\n");
    }

    printf("Done.\n");
    return 0;
}
