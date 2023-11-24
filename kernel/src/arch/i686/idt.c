#include <kernel/interrupts.h>
#include <kernel/segmentation.h>

#define IDT_ENTRIES 256

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

typedef struct
{
  uint16_t offset_low;
  uint16_t selector;
  uint8_t reserved;
  uint8_t flags;
  uint16_t offset_high;
} __attribute__((packed)) idt_gate_t;

typedef struct
{
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idt_register_t;

idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

void isr_install(void);

void set_idt_gate(int n, uint32_t handler)
{
  idt[n].offset_low = low_16(handler);
  idt[n].selector = KERNEL_CODE_SELECTOR;
  idt[n].reserved = 0x00;
  idt[n].flags = 0x8E;
  idt[n].offset_high = high_16(handler);
}

uint32_t interrupts_init(void)
{
  isr_install();

  idt_reg.base = (uint32_t)&idt;
  idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

  __asm__ __volatile__("lidtl (%0)" : : "r"(&idt_reg));

  return EOK;
}

void enable_interrupts(void)
{
  __asm__("sti");
}

void disable_interrupts(void)
{
  __asm__("cli");
}
