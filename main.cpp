// cpu-simulator -- 第六关：访存 lw / sw
#include <cstdint>
#include <cstdio>
#include <cstring>

// ===== 机器状态 =====
static int32_t  reg[32];
static uint8_t  mem[4096];
static uint32_t pc = 0;

// 从内存读一个 32 位字（小端）
static uint32_t fetch(uint32_t addr) {
    return (uint32_t)mem[addr]
        | ((uint32_t)mem[addr + 1] << 8)
        | ((uint32_t)mem[addr + 2] << 16)
        | ((uint32_t)mem[addr + 3] << 24);
}

// 往内存写一个 32 位字（小端）
static void store_word(uint32_t addr, uint32_t val) {
    mem[addr + 0] = (uint8_t)(val);
    mem[addr + 1] = (uint8_t)(val >> 8);
    mem[addr + 2] = (uint8_t)(val >> 16);
    mem[addr + 3] = (uint8_t)(val >> 24);
}

// 译码结果
struct Decoded {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
};

// 把 32 位指令拆成字段
static Decoded decode(uint32_t inst) {
    Decoded d;
    d.opcode = inst & 0x7F;
    d.rd     = (inst >> 7) & 0x1F;
    d.funct3 = (inst >> 12) & 0x7;
    d.rs1    = (inst >> 15) & 0x1F;
    d.rs2    = (inst >> 20) & 0x1F;
    d.funct7 = (inst >> 25) & 0x7F;
    return d;
}

// ---- 把 12 位立即数当成有符号数，扩展到 32 位 ----
static int32_t sext12(uint32_t v) {
    return (int32_t)(v << 20) >> 20;
}

// ---- 写寄存器：x0 恒为 0 ----
static void write_reg(uint32_t idx, int32_t val) {
    if (idx == 0) return;
    reg[idx] = val;
}

// ---- 执行一条指令 ----
static bool execute(const Decoded& d, uint32_t inst) {
    switch (d.opcode) {

    case 0x13: {                        // I 型：addi
        int32_t imm = sext12((inst >> 20) & 0xFFF);
        int32_t a   = reg[d.rs1];
        if (d.funct3 == 0x0) {
            write_reg(d.rd, a + imm);
            printf("  addi  x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        break;
    }

    case 0x33: {                        // R 型
        int32_t a = reg[d.rs1];
        int32_t b = reg[d.rs2];
        if (d.funct3 == 0x0 && d.funct7 == 0x00) {   // => add
            write_reg(d.rd, a + b);
            printf("  add   x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        if (d.funct3 == 0x0 && d.funct7 == 0x20) {   // => sub
            write_reg(d.rd, a - b);
            printf("  sub   x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        if (d.funct3 == 0x7 && d.funct7 == 0x00) {   // => and
            write_reg(d.rd, a & b);
            printf("  and   x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        if (d.funct3 == 0x6 && d.funct7 == 0x00) {   // => or
            write_reg(d.rd, a | b);
            printf("  or    x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        break;
    }

    case 0x03: {                        // I 型：lw（从内存读 4 字节进寄存器）
        uint32_t addr = reg[d.rs1] + sext12((inst >> 20) & 0xFFF);
        if (d.funct3 == 0x2) {
            // ---------------- 空 1 ----------------
            // 目标：把 mem[addr] 处的 4 字节读出来，写进 d.rd 号寄存器
            // 提示：读内存用 fetch，写寄存器用 write_reg
            write_reg(d.rd, (int32_t)fetch(addr));
            printf("  lw    x%-2u <= %d  (来自 mem[0x%X])\n", d.rd, reg[d.rd], addr);
            return true;
        }
        break;
    }

    case 0x23: {                        // S 型：sw（把寄存器写进内存 4 字节）
        // ---------------- 空 2 ----------------
        // S 型立即数被拆成了两段：
        //   高 7 位 imm[11:5] 在 [31:25]
        //   低 5 位 imm[4:0]  在 [11:7]（就是 rd 那个位置，S 型没有 rd）
        // 目标：把两段拼回一个 12 位数，再做符号扩展
        int32_t imm = sext12(((inst >> 25) << 5) | ((inst >> 7) & 0x1F));
        uint32_t addr = reg[d.rs1] + imm;
        // ---------------- 空 3 ----------------
        // 目标：把 d.rs2 号寄存器的值写进 mem[addr]
        // 提示：写内存用 store_word；注意数据来源是 rs2，不是 rd！
        store_word(addr, (uint32_t)reg[d.rs2]);
        printf("  sw    mem[0x%X] <= %d\n", addr, reg[d.rs2]);
        return true;
    }

    }

    printf("  [不支持] opcode=0x%X funct3=%u funct7=%u\n", d.opcode, d.funct3, d.funct7);
    return false;
}

// 第一版验收程序：9 条指令，把 7 条子集指令全用上
// 编码公式：funct7<<25 | rs2<<20 | rs1<<15 | funct3<<12 | rd<<7 | opcode
// 例：add x5,x3,x4 -> 4<<20 | 3<<15 | 5<<7 | 0x33 = 0x004182B3
static const uint32_t kProgram[] = {
    0x06400093,   // addi x1, x0, 100      -> x1 = 100
    0x00800113,   // addi x2, x0, 8        -> x2 = 8
    0x00112023,   // sw   x1, 0(x2)        -> mem[8] = 100
    0x00012183,   // lw   x3, 0(x2)        -> x3 = 100
    0x01600213,   // addi x4, x0, 22       -> x4 = 22
    0x004182B3,   // add  x5, x3, x4       -> x5 = 122
    0x0042F333,   // and  x6, x5, x4       -> x6 = 18
    0x0042E3B3,   // or   x7, x5, x4       -> x7 = 126
    0x40428433,   // sub  x8, x5, x4       -> x8 = 100
};

int main() {
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    pc = 0;

    for (size_t i = 0; i < sizeof(kProgram) / sizeof(kProgram[0]); i++) {
        store_word((uint32_t)(i * 4), kProgram[i]);
    }

    printf("=== cpu-simulator v1.0 ===\n");

    while (true) {
        uint32_t inst = fetch(pc);
        if (inst == 0) break;                    // 全 0 当停机

        printf("pc=0x%08X inst=0x%08X  ", pc, inst);   // 不换行，与下一行接成一条轨迹
        Decoded d = decode(inst);
        if (!execute(d, inst)) break;

        pc += 4;                                 // 执行成功才前进
    }

    printf("\n-- 寄存器 --\n");
    for (int i = 0; i < 9; i++) {
        printf("  x%-2d = %d\n", i, reg[i]);
    }
    printf("\nmem[0x8] 处的 4 字节 = %u\n", fetch(8));
    printf("halt: pc=0x%08X\n", pc);
    return 0;
}
