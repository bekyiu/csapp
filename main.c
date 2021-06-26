#include <stdio.h>
#include <stdbool.h>

// 获取二进制最低位的1的2的幂
// 例: 0b1010 -> 0b10
u_int32_t LowBit(u_int32_t x) {
    return x & ((~x) + 1);
}

// 判断一个数的16进制是否全部都是数字
// 例: 0xab -> 0x11, 0x1a -> 0x01
u_int32_t Letter(u_int32_t x) {
    u_int32_t x1 = x & 0x22222222;
    u_int32_t x2 = x & 0x44444444;
    u_int32_t x3 = x & 0x88888888;
    return (x3 >> 3) & ((x2 >> 2) | (x1 >> 1));
}

// float: (-1)^s * 1.f * 2^e
//   s           e               f
// [32]    [30] ... [23]   [22] ... [0]
//   1           8               23
// s: 符号位 0 1两种取值
// f: 位数部分, 实际是多少就填多少
// e: 指数部分 本来是[0x00, 0xff], 但是这样就无法表示负的指数了,
// 所以实际的指数要减去一个常量(127), 让[0x00, 0xff]中一半的数来表示负数, 一半的表示非负数


int main() {
    printf("0x%x\n", LowBit(0xa));
    printf("0x%8x is letter: 0x%8x\n", 0xabcdefab, Letter(0xabcdefab));
    printf("0x%8x is letter: 0x%8x\n", 0xa0b0c0d0, Letter(0xa0b0c0d0));
    return 0;
}
