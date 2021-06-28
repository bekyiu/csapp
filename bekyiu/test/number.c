//
// Created by bekyiu on 2021/6/27.
//


#include <stdint.h>

// 获取二进制最低位的1的2的幂
// 例: 0b1010 -> 0b10
uint32_t LowBit(uint32_t x) {
    return x & ((~x) + 1);
}

// 判断一个数的16进制是否全部都是数字
// 例: 0xab -> 0x11, 0x1a -> 0x01
uint32_t Letter(uint32_t x) {
    uint32_t x1 = x & 0x22222222;
    uint32_t x2 = x & 0x44444444;
    uint32_t x3 = x & 0x88888888;
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

// 返回最高位1的位置
uint32_t HighestOneIdx(uint32_t u) {
    // 32 个0
    if (u == 0) {
        return -1;
    }

    int n = 0;
    for (int i = 0; i < 32; ++i) {
        if ((u >> i) == 1) {
            n = i;
            break;
        }
    }
    return n + 1;
}

// 将一个uint32的值转换为对应的float表示
uint32_t Uint2float(uint32_t u) {
    uint32_t n = HighestOneIdx(u);
    // 浮点数尾数只能容纳23位
    if (n - 1 <= 23) {
        uint32_t s = u >> 31;
        // u & mask 取到尾数的bit
        uint32_t f = u & (0xffffffff >> (32 - (n - 1)));
        //
        uint32_t e = n - 1 + 127;
        return (s << 31) | (e << 23) | f;
    } else {
        // todo 需要近似尾数
    }
    return 0;
}

// 近似位数位
uint32_t NearFraction(uint32_t u, uint32_t n) {

}
