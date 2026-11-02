// cpu-simulator -- 第三关：译码
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

// 往内存写一个 32 位字（小端）-- 就是 fetch 的反操作
static void store_word(uint32_t addr, uint32_t val) {
    mem[addr + 0] = (uint8_t)(val);
    mem[addr + 1] = (uint8_t)(val >> 8);
    mem[addr + 2] = (uint8_t)(val >> 16);
    mem[addr + 3] = (uint8_t)(val >> 24);
}

// 一条指令拆出来的六个字段
struct Decoded {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
};

// 把 32 位指令拆成字段
// 规律：右移「这个字段的起始位」，再 &「(1<<宽度)-1」把上下的 bit 抹掉
static Decoded decode(uint32_t inst) {
    Decoded d;
    d.opcode = inst & 0x7F;             // [6:0]   7 位，起始位 0
    d.rd     = (inst >> 7) & 0x1F;      // [11:7]  5 位，起始位 7
    d.funct3 = (inst >> 12) & 0x7;                  // [14:12] 3 位，起始位 12
    d.rs1    = (inst >> 15) & 0x1F;     // [19:15] 5 位，起始位 15
    d.rs2    = (inst >> 20) & 0x1F;                  // [24:20] 5 位，起始位 20
    d.funct7 = (inst >> 25) & 0x7F;                  // [31:25] 7 位，起始位 25
    return d;
}

// 验收程序：9 条指令的机器码
static const uint32_t kProgram[] = {
    0x06400093,   // addi x1, x0, 100
    0x00800113,   // addi x2, x0, 8
    0x00112023,   // sw   x1, 0(x2)
    0x00012183,   // lw   x3, 0(x2)
    0x01600213,   // addi x4, x0, 22
    0x004182B3,   // add  x5, x3, x4
    0x0042F333,   // and  x6, x5, x4
    0x0042E3B3,   // or   x7, x5, x4
    0x40428433,   // sub  x8, x5, x4
};

int main() {
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    pc = 0;

    // 把程序装到内存里，从地址 0 开始
    for (size_t i = 0; i < sizeof(kProgram) / sizeof(kProgram[0]); i++) {
        store_word((uint32_t)(i * 4), kProgram[i]);
    }

    printf("=== cpu-simulator v0.2 ===\n");
    printf("%-10s %-7s %-3s %-3s %-4s %-4s %-4s\n",
           "inst", "opcode", "rd", "f3", "rs1", "rs2", "f7");

    while (true) {
        uint32_t inst = fetch(pc);
        if (inst == 0) break;            // 全 0 当停机

        Decoded d = decode(inst);
        printf("0x%08X 0x%-5X %-3u %-3u %-4u %-4u %-4u\n",
               inst, d.opcode, d.rd, d.funct3, d.rs1, d.rs2, d.funct7);

        pc += 4;
    }

    printf("halt: pc=0x%08X\n", pc);
    return 0;
}
