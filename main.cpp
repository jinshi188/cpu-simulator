// cpu-simulator -- 第二关：立起机器状态 + 取指
#include <cstdint>
#include <cstdio>
#include <cstring>

// ===== 机器状态：三样，全部用普通变量/数组模拟 =====
static int32_t  reg[32];    // 32 个 32 位寄存器，有符号
static uint8_t  mem[4096];  // 4 KB 字节内存，无符号
static uint32_t pc = 0;     // 程序计数器：下一条指令的地址

// 从内存 addr 处读出一个 32 位指令字
// RISC-V 是小端序：低字节在低地址
static uint32_t fetch(uint32_t addr) {
    // ---------------- 空 1 ----------------
    // 目标：把 mem[addr] .. mem[addr+3] 这 4 个字节拼成一个 uint32_t
    // 要求：只能用移位和按位或，不许用指针强转
    return (uint32_t)mem[addr]
        | ((uint32_t)mem[addr + 1] << 8)
        | ((uint32_t)mem[addr + 2] << 16)
        | ((uint32_t)mem[addr + 3] << 24);
}

int main() {
    // ---- 清零机器状态 ----
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    pc = 0;

    // ---- 塞测试数据：故意挑 0xDEADBEEF 这个有辨识度的值 ----
    // 按小端序摆：低字节 0xEF 放在最低地址
    mem[0] = 0xEF;
    mem[1] = 0xBE;
    mem[2] = 0xAD;
    mem[3] = 0xDE;

    printf("=== cpu-simulator v0.1 ===\n");
    printf("reg 占用 %zu 字节, mem 占用 %zu 字节\n", sizeof(reg), sizeof(mem));

    // ---- 主循环：这一关只取指，不译码 ----
    int steps = 0;
    while (steps < 1) {
        // ---------------- 空 2 ----------------
        // 目标：调用 fetch，把 pc 处的那条指令读出来
        uint32_t inst = fetch(pc);

        printf("pc=0x%08X  inst=0x%08X\n", pc, inst);

        // ---------------- 空 3 ----------------
        // 目标：一条指令占 4 字节，执行完 pc 要往后走
        pc += 4;

        steps++;
    }

    printf("halt: pc=0x%08X\n", pc);
    return 0;
}
