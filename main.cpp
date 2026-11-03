// cpu-simulator -- 第四关：执行 addi 和 add
#include <cstdint>
#include <cstdio>
#include <cstring>

// ===== 机器状态 =====
static int32_t  reg[32];
static uint8_t  mem[4096];
static uint32_t pc = 0;

// 从内存读一个 32 位指令字（小端）
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
// 例：0x064 -> 100        0xFFF -> -1
// 关键：看 bit11。它是 1 就说明这个数在补码里是负的，高 20 位要补满 1
static int32_t sext12(uint32_t v) {
    // ---------------- 空 1 ----------------
    return (int32_t)(v << 20) >> 20;
}

// ---- 写寄存器 ----
// x0 是硬连线到 0 的寄存器，任何写入都必须丢掉
static void write_reg(uint32_t idx, int32_t val) {
    // ---------------- 空 2 ----------------
    if(idx == 0) return;
    reg[idx] = val;
}

// ---- 执行一条指令 ----
// 返回 false = 遇到不支持的指令，停机
static bool execute(const Decoded& d, uint32_t inst) {
    switch (d.opcode) {

    case 0x13: {                        // I 型：addi 之类
        int32_t imm = sext12((inst >> 20) & 0xFFF);
        int32_t a   = reg[d.rs1];
        if (d.funct3 == 0x0) {          // 0x13 + funct3=000  =>  addi
            // ---------------- 空 3 ----------------
            int32_t res = a + imm;
            write_reg(d.rd, res);
            printf("  addi  x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        break;
    }

    case 0x33: {                        // R 型：add 之类
        int32_t a = reg[d.rs1];
        int32_t b = reg[d.rs2];
        if (d.funct3 == 0x0 && d.funct7 == 0x00) {   // => add
            // ---------------- 空 4 ----------------
            int32_t res = a + b;
            write_reg(d.rd, res);
            printf("  add   x%-2u <= %d\n", d.rd, reg[d.rd]);
            return true;
        }
        break;
    }

    }

    printf("  [不支持] opcode=0x%X funct3=%u funct7=%u\n", d.opcode, d.funct3, d.funct7);
    return false;
}

// 本关测试程序：只用 addi 和 add
static const uint32_t kProgram[] = {
    0x06400093,   // addi x1, x0, 100
    0x00800113,   // addi x2, x0, 8
    0x002081B3,   // add  x3, x1, x2
    0xFFF00213,   // addi x4, x0, -1      <-- 符号扩展考场
    0x001202B3,   // add  x5, x4, x1
};

int main() {
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    pc = 0;

    for (size_t i = 0; i < sizeof(kProgram) / sizeof(kProgram[0]); i++) {
        store_word((uint32_t)(i * 4), kProgram[i]);
    }

    printf("=== cpu-simulator v0.3 ===\n");

    while (true) {
        uint32_t inst = fetch(pc);
        if (inst == 0) break;                    // 全 0 当停机

        printf("pc=0x%08X inst=0x%08X\n", pc, inst);
        Decoded d = decode(inst);
        if (!execute(d, inst)) break;

        pc += 4;                                 // 执行成功才前进
    }

    printf("\n-- 寄存器 --\n");
    for (int i = 0; i < 6; i++) {
        printf("  x%-2d = %d\n", i, reg[i]);
    }
    printf("halt: pc=0x%08X\n", pc);
    return 0;
}
