/* 
 * Arm PrimeCell PL050 Keyboard / Mouse Interface
 *
 * Copyright (c) 2006 CodeSourcery.
 * Written by Paul Brook
 *
 * This code is licenced under the GPL.
 */

#include "vl.h"

typedef struct {
    void *dev;
    uint32_t base;
    uint32_t cr;
    uint32_t clk;
    uint32_t last;
    void *pic;
    int pending;
    int irq;
    int is_mouse;
} pl050_state;

static const unsigned char pl050_id[] =
{ 0x50, 0x10, 0x04, 0x00, 0x0d, 0xf0, 0x05, 0xb1 };

static void pl050_update(void *opaque, int level)
{
    pl050_state *s = (pl050_state *)opaque;
    int raise;

    s->pending = level;
    raise = (s->pending && (s->cr & 0x10) != 0)
            || (s->cr & 0x08) != 0;
    pic_set_irq_new(s->pic, s->irq, raise);
}

static uint32_t pl050_read(void *opaque, target_phys_addr_t offset)
{
    pl050_state *s = (pl050_state *)opaque;
    offset -= s->base;
    if (offset >= 0xfe0 && offset < 0x1000)
        return pl050_id[(offset - 0xfe0) >> 2];

    switch (offset >> 2) {
    case 0: /* KMICR */
        return s->cr;
    case 1: /* KMISTAT */
        /* KMIC and KMID bits not implemented.  */
        if (s->pending) {
            return 0x10;
        } else {
            return 0;
        }
    case 2: /* KMIDATA */
        if (s->pending)
            s->last = ps2_read_data(s->dev);
        return s->last;
    case 3: /* KMICLKDIV */
        return s->clk;
    case 4: /* KMIIR */
        return s->pending | 2;
    default:
        cpu_abort (cpu_single_env, "pl050_read: Bad offset %x\n", offset);
        return 0;
    }
}

static void pl050_write(void *opaque, target_phys_addr_t offset,
                          uint32_t value)
{
    pl050_state *s = (pl050_state *)opaque;
    offset -= s->base;
    switch (offset >> 2) {
    case 0: /* KMICR */
        s->cr = value;
        pl050_update(s, s->pending);
        /* ??? Need to implement the enable/disable bit.  */
        break;
    case 2: /* KMIDATA */
        /* ??? This should toggle the TX interrupt line.  */
        /* ??? This means kbd/mouse can block each other.  */
        if (s->is_mouse) {
            ps2_write_mouse(s->dev, value);
        } else {
            ps2_write_keyboard(s->dev, value);
        }
        break;
    case 3: /* KMICLKDIV */
        s->clk = value;
        return;
    default:
        cpu_abort (cpu_single_env, "pl050_write: Bad offset %x\n", offset);
    }
}
static CPUReadMemoryFunc *pl050_readfn[] = {
   pl050_read,
   pl050_read,
   pl050_read
};

static CPUWriteMemoryFunc *pl050_writefn[] = {
   pl050_write,
   pl050_write,
   pl050_write
};

void pl050_init(uint32_t base, void *pic, int irq, int is_mouse)
{
    int iomemtype;
    pl050_state *s;

    s = (pl050_state *)qemu_mallocz(sizeof(pl050_state));
    iomemtype = cpu_register_io_memory(0, pl050_readfn,
                                       pl050_writefn, s);
    cpu_register_physical_memory(base, 0x00000fff, iomemtype);
    s->base = base;
    s->pic = pic;
    s->irq = irq;
    s->is_mouse = is_mouse;
    if (is_mouse)
        s->dev = ps2_mouse_init(pl050_update, s);
    else
        s->dev = ps2_kbd_init(pl050_update, s);
    /* ??? Save/restore.  */
}

