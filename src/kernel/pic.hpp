#pragma once
#include "common.hpp"

#define PIC1_CMD 0x20
#define PIC1_DAT 0x21
#define PIC2_CMD 0xA0
#define PIC2_DAT 0xA1

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

#define PIC_READ_IRR 0xA
#define PIC_READ_ISR 0xB

void pic_eoi(u8 irq);
void pic_set_mask(u8 irq);
void pic_clear_mask(u8 irq);
void pic_remap(u8 offset);

void pic_init();