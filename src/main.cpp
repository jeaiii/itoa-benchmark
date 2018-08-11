#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
#include "resultfilename.h"
#include "timer.h"
#include "test.h"

/*
Changes Copyright (c) 2018 James Edward Anhalt III - https://github.com/jeaiii
*/

const unsigned c_scale = 4096*2;

const int kTrial = 128;

template <typename T>
struct Traits {
};

template <>
struct Traits<uint32_t> {
    enum { kBufferSize = 11 };
    enum { kMaxDigit = 10 };
    static uint32_t Negate(uint32_t x) { return x; };
};

template <>
struct Traits<int32_t> {
    enum { kBufferSize = 12 };
    enum { kMaxDigit = 10 };
    static int32_t Negate(int32_t x) { return -x; };
};

template <>
struct Traits<uint64_t> {
    enum { kBufferSize = 21 };
    enum { kMaxDigit = 20 };
    static uint64_t Negate(uint64_t x) { return x; };
};

template <>
struct Traits<int64_t> {
    enum { kBufferSize = 21 };
    enum { kMaxDigit = 19 };
    static int64_t Negate(int64_t x) { return -x; };
};

template <typename T>
static void VerifyValue(T value, void(*f)(T, char*), void(*g)(T, char*), const char* fname, const char* gname) {
    char buffer1[Traits<T>::kBufferSize];
    char buffer2[Traits<T>::kBufferSize];

    f(value, buffer1);
    g(value, buffer2);

    if (strcmp(buffer1, buffer2) != 0) {
        printf("\nError: %s -> %s, %s -> %s\n", fname, buffer1, gname, buffer2);
        throw std::exception();
    }
    //puts(buffer1);
}

template <typename T>
static void Verify(void(*f)(T, char*), void(*g)(T, char*), const char* fname, const char* gname) {
    printf("Verifying %s = %s ... ", fname, gname);

    // Boundary cases
    VerifyValue<T>(0, f, g, fname, gname);
    VerifyValue<T>(std::numeric_limits<T>::min(), f, g, fname, gname);
    VerifyValue<T>(std::numeric_limits<T>::max(), f, g, fname, gname);

    // 2^n - 1, 2^n, 10^n - 1, 10^n until overflow
    for (uint32_t power = 2; power <= 10; power += 8) {
        T i = 1, last;
        do {
            VerifyValue<T>(i - 1, f, g, fname, gname);
            VerifyValue<T>(i, f, g, fname, gname);
            if (std::numeric_limits<T>::min() < 0) {
                VerifyValue<T>(Traits<T>::Negate(i), f, g, fname, gname);
                VerifyValue<T>(Traits<T>::Negate(i + 1), f, g, fname, gname);
            }
            last = i;
            i *= power;
        } while (last < i);
    }

    printf("OK\n");
}

void VerifyAll() {
    const TestList& tests = TestManager::Instance().GetTests();

    // Find naive for verification
    const Test* naive = 0;
    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
    if (strcmp((*itr)->fname, "naive") == 0) {
        naive = *itr;
        break;
    }

    assert(naive != 0);

    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr) {
        if (strcmp((*itr)->fname, "null") != 0) {   // skip null
            try {
                Verify(naive->u32toa, (*itr)->u32toa, "naive_u32toa", (*itr)->fname);
                Verify(naive->i32toa, (*itr)->i32toa, "naive_i32toa", (*itr)->fname);
                Verify(naive->u64toa, (*itr)->u64toa, "naive_u64toa", (*itr)->fname);
                Verify(naive->i64toa, (*itr)->i64toa, "naive_i64toa", (*itr)->fname);
            }
            catch (...) {
            }
        }
    }
}
#include <intrin.h>

uint64_t TimeEnter() {
    int info[4];

    _ReadWriteBarrier();
    __cpuidex(info, 0, 0);
    uint64_t time = __rdtsc();
    _ReadWriteBarrier();
    return time;
}

uint64_t TimeLeave() {
    int info[4];
    unsigned int aux;
    _ReadWriteBarrier();
    uint64_t time = __rdtscp(&aux);
    __cpuidex(info, 0, 0);
    _ReadWriteBarrier();
    return time;
}


template<class T, size_t N>
uint64_t BenchData(void(*f)(T, char*), const T(&data)[N])
{
    char buffer[Traits<T>::kBufferSize];
    uint64_t duration = std::numeric_limits<uint64_t>::max();

   // Sleep(15);

    int trial = kTrial;
    do {

        uint64_t time = TimeEnter();
        for (size_t i = 0; i < N; i += 8) {
            f(data[i + 0], buffer);
            f(data[i + 1], buffer);
            f(data[i + 2], buffer);
            f(data[i + 3], buffer);
            f(data[i + 4], buffer);
            f(data[i + 5], buffer);
            f(data[i + 6], buffer);
            f(data[i + 7], buffer);
        }

        time = (TimeLeave() - time) / N;

        if (time < duration)
            duration = time, trial = kTrial;

    } while (--trial > 0);

    return duration;
}

template <typename T>
void BenchSequential(void(*f)(T, char*), const char* type, const char* fname, FILE* fp) {
    printf("Benchmarking sequential %-20s ... ", fname);

    T data[c_scale];

    uint64_t durations[Traits<T>::kMaxDigit + 1];

    T start = 0;
    for (int digit = 0; ++digit <= Traits<T>::kMaxDigit;) {
        T end = (digit == Traits<T>::kMaxDigit) ? std::numeric_limits<T>::max() : (start + !start) * 10 - 1;
        T v = start;
        for (size_t i = 0; i < c_scale; ++i) {
            data[i] = v;
            v = v == end ? start : v + 1;
        }

        durations[digit] = BenchData(f, data);

        start = end + 1;
    }

    uint64_t min_d = durations[1];
    uint64_t max_d = durations[1];
    for (int digit = 1; digit <= Traits<T>::kMaxDigit; digit++) {
        fprintf(fp, "%lld,", durations[digit]);
        min_d = std::min(min_d, durations[digit]);
        max_d = std::max(max_d, durations[digit]);
    }

    printf("[%lld cc, %lld cc]\n", min_d, max_d);
}

template <class T, size_t N>
class RandomData {
public:
    static T(&GetData())[N] {
        static RandomData singleton;
        return singleton.m_data;
    }

    
private:
    RandomData()
    {
        T x = 0;
        T sign = 1;

        for (size_t i = 0; i < N; ++i) {
            if (x == 0)
                x = std::numeric_limits<T>::max();

            m_data[i] = x * sign;
            if (i % 64 == 0) 
                sign = Traits<T>::Negate(sign);
            x /= 10;
        }
        std::random_shuffle(m_data, m_data + N);
    }

    T m_data[N];
};

template <typename T>
void BenchRandom(void(*f)(T, char*), const char* type, const char* fname, FILE* fp) {
    printf("Benchmarking     random %-20s ... ", fname);

    uint64_t duration = BenchData(f, RandomData<T, c_scale>::GetData());

    fprintf(fp, "%lld", duration);
    printf("%lld cc\n", duration);
}

template <typename T>
void Bench(void(*f)(T, char*), const char* type, const char* fname, FILE* fp) {

    fprintf(fp, "{ name: '%s', data:[", fname);

    BenchSequential(f, type, fname, fp);
    BenchRandom(f, type, fname, fp);

    fprintf(fp, "]},\n");
}


void BenchAll() {
    // Try to write to /result path, where template.php exists
    FILE *fp;
    if ((fp = fopen("../../result/template.php", "r")) != NULL) {
        fclose(fp);
        fp = fopen("../../result/" RESULT_FILENAME, "w");
    }
    else if ((fp = fopen("../result/template.php", "r")) != NULL) {
        fclose(fp);
        fp = fopen("../result/" RESULT_FILENAME, "w");
    }
    else
        fp = fopen(RESULT_FILENAME, "w");

    const TestList& tests = TestManager::Instance().GetTests();

    fprintf(fp, "var tests=[\n");

    puts("u32toa");
    fprintf(fp, "{ name: 'u32toa', data:[\n");
    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
        Bench((*itr)->u32toa, "u32toa", (*itr)->fname, fp);
    fprintf(fp, "]},\n");

    puts("");
    puts("i32toa");
    fprintf(fp, "{ name: 'i32toa', data:[\n");
    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
        Bench((*itr)->i32toa, "i32toa", (*itr)->fname, fp);
    fprintf(fp, "]},\n");

    puts("");
    puts("u64toa");
    fprintf(fp, "{ name: 'u64toa', data:[\n");
    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
        Bench((*itr)->u64toa, "u64toa", (*itr)->fname, fp);
    fprintf(fp, "]},\n");

    puts("");
    puts("i64toa");
    fprintf(fp, "{ name: 'i64toa', data:[\n");
    for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
        Bench((*itr)->i64toa, "i64toa", (*itr)->fname, fp);
    fprintf(fp, "]},\n");

    fprintf(fp, "];\n");
    fclose(fp);
}

int main() {
    // sort tests
    TestList& tests = TestManager::Instance().GetTests();

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadAffinityMask(GetCurrentThread(), 1);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    VerifyAll();
    BenchAll();
}
