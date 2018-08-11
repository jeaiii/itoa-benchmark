/*
MIT License

Copyright (c) 2017 James Edward Anhalt III - https://github.com/jeaiii/itoa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>


struct pair { char t, o; };
#define P(T) T, '0',  T, '1', T, '2', T, '3', T, '4', T, '5', T, '6', T, '7', T, '8', T, '9'
static const pair s_pairs[] = { P('0'), P('1'), P('2'), P('3'), P('4'), P('5'), P('6'), P('7'), P('8'), P('9') };


static const pair s_heads[] = { 
 '0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0,
P('1'), P('2'), P('3'), P('4'), P('5'), P('6'), P('7'), P('8'), P('9') };

#define W(N, I) *(pair*)&b[N] = s_pairs[I]
#define A(N) t = (uint64_t(1) << (32 + N / 5 * N * 53 / 16)) / uint32_t(1e##N) + 1 + N/6 - N/8, t *= u, t >>= N / 5 * N * 53 / 16, t += N / 6 * 4, W(0, t >> 32)
#define S(N) b[N] = char(uint64_t(10) * uint32_t(t) >> 32) + '0'
#define D(N) t = uint64_t(100) * uint32_t(t), W(N, t >> 32)

#define C0 b[0] = char(u) + '0'
#define C1 W(0, u)
#define C2 A(1), S(2)
#define C3 A(2), D(2)
#define C4 A(3), D(2), S(4)
#define C5 A(4), D(2), D(4)
#define C6 A(5), D(2), D(4), S(6)
#define C7 A(6), D(2), D(4), D(6)
#define C8 A(7), D(2), D(4), D(6), S(8)
#define C9 A(8), D(2), D(4), D(6), D(8)

#define L09(F) u < 100        ? L01(F) : L29(F)
#define L29(F) u < 1000000    ? L25(F) : L69(F)
#define L25(F) u < 10000      ? L23(F) : L45(F)
#define L69(F) u < 100000000  ? L67(F) : L89(F)
#define L03(F) u < 100        ? L01(F) : L23(F)

#define L01(F) u < 10         ? F(0) : F(1)
#define L23(F) u < 1000       ? F(2) : F(3)
#define L45(F) u < 100000     ? F(4) : F(5)
#define L67(F) u < 10000000   ? F(6) : F(7)
#define L89(F) u < 1000000000 ? F(8) : F(9)

#define POS(N) (N < length ? C##N, N + 1 : N + 1)
#define NEG(N) (N + 1 < length ? *b++ = '-', C##N, N + 2 : N + 2)


#define K(N) ((uint64_t(1) << 57) / uint32_t(1e##N) + 1)

void u64toa_jeaiii(uint64_t n, char* b);

void u64toa_jeaiii64(uint64_t n, char *b) {
    uint64_t t;

    n < 10000
        ? n < 100
        ? 
            (
            *(pair*)&b[0] = s_heads[n], b-= n < 10,
            b[2] = 0)
        :
            (t = K(2)*n, n = t >> 57,
            *(pair*)&b[0] = s_heads[n], b -= n < 10,
            t -= n << 57 - 0, t *= 25, *(pair*)&b[2] = s_pairs[t >> 57 - 2],
            b[4] = 0)
        : 
        n < 100000000
            ? n < 1000000
            ?
            (t = K(4)*n, n = t >> 57,
            *(pair*)&b[0] = s_heads[n], b -= n < 10,
            t -= n << 57 - 0, t *= 25, *(pair*)&b[2] = s_pairs[n = t >> 57 - 2],
            t -= n << 57 - 2, t *= 25, *(pair*)&b[4] = s_pairs[n = t >> 57 - 4],
            b[6] = 0)                
        :
            (t = K(6)*n, n = t >> 57,
            *(pair*)&b[0] = s_heads[n], b -= n < 10,
            t -= n << 57 - 0, t *= 25, *(pair*)&b[2] = s_pairs[n = t >> 57 - 2],
            t -= n << 57 - 2, t *= 25, *(pair*)&b[4] = s_pairs[n = t >> 57 - 4],
            t -= n << 57 - 4, t *= 25, *(pair*)&b[6] = s_pairs[n = t >> 57 - 6],
            b[8] = 0)
        :
        n < 1000000000000ull
            ? n < 10000000000ull
        ?
            (t = K(8)*n, n = t >> 57,
            *(pair*)&b[0] = s_heads[n], b -= n < 10,
            t -= n << 57 - 0, t *= 25, *(pair*)&b[2] = s_pairs[n = t >> 57 - 2],
            t -= n << 57 - 2, t *= 25, *(pair*)&b[4] = s_pairs[n = t >> 57 - 4],
            t -= n << 57 - 4, t *= 25, *(pair*)&b[6] = s_pairs[n = t >> 57 - 6],
            t -= n << 57 - 6, t *= 25, *(pair*)&b[8] = s_pairs[n = t >> 57 - 8],
            b[10] = 0)
        :
            (t = K(10)*n, n = t >> 57,
            *(pair*)&b[0] = s_heads[n], b -= n < 10,
            t -= n << 57 - 0, t *= 25, *(pair*)&b[2] = s_pairs[n = t >> 57 - 2],
            t -= n << 57 - 2, t *= 25, *(pair*)&b[4] = s_pairs[n = t >> 57 - 4],
            t -= n << 57 - 4, t *= 25, *(pair*)&b[6] = s_pairs[n = t >> 57 - 6],
            t -= n << 57 - 6, t *= 25, *(pair*)&b[8] = s_pairs[n = t >> 57 - 8],
            t -= n << 57 - 8, t *= 25, *(pair*)&b[10] = s_pairs[n = t >> 57 - 10],
            b[12] = 0)

        : u64toa_jeaiii(n, b);
}

//t <<= 7 + 0, t >>= 7 + 0, t *= 25, *(pair*)&b[2] = s_pairs[t >> 57 - 2],
