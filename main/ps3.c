#include <string.h>
#include "zephyr/types.h"
#include "util.h"
#include "ps3.h"

enum {
    PS3_SELECT,
    PS3_L3,
    PS3_R3,
    PS3_START,
    PS3_D_UP,
    PS3_D_RIGHT,
    PS3_D_DOWN,
    PS3_D_LEFT,
    PS3_L2,
    PS3_R2,
    PS3_L1,
    PS3_R1,
    PS3_T,
    PS3_C,
    PS3_X,
    PS3_S,
    PS3_PS,
};

const uint8_t ps3_axes_idx[6] =
{
/*  AXIS_LX, AXIS_LY, AXIS_RX, AXIS_RY, TRIG_L, TRIG_R  */
    0,       1,       2,       3,       12,     13
};

const struct ctrl_meta ps3_btn_meta =
{
    .polarity = 0,
};

const struct ctrl_meta ps3_axes_meta[6] =
{
    {.neutral = 0x80, .abs_max = 0x80},
    {.neutral = 0x80, .abs_max = 0x80, .polarity = 1},
    {.neutral = 0x80, .abs_max = 0x80},
    {.neutral = 0x80, .abs_max = 0x80, .polarity = 1},
    {.neutral = 0x00, .abs_max = 0xFF},
    {.neutral = 0x00, .abs_max = 0xFF},
};

struct ps3_map {
    uint32_t buttons;
    uint8_t reserved;
    uint8_t axes[20];
} __packed;

const uint32_t ps3_mask[4] = {0xAA7F0FFF, 0x00000000, 0x00000000, 0x00000000};
const uint32_t ps3_desc[4] = {0x110000FF, 0x00000000, 0x00000000, 0x00000000};

const uint32_t ps3_btns_mask[32] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    BIT(PS3_D_LEFT), BIT(PS3_D_RIGHT), BIT(PS3_D_DOWN), BIT(PS3_D_UP),
    0, 0, 0, 0,
    BIT(PS3_S), BIT(PS3_C), BIT(PS3_X), BIT(PS3_T),
    BIT(PS3_START), BIT(PS3_SELECT), BIT(PS3_PS), 0,
    0, BIT(PS3_L1), 0, BIT(PS3_L3),
    0, BIT(PS3_R2), 0, BIT(PS3_R3),
};

void ps3_to_generic(struct bt_data *bt_data, struct generic_ctrl *ctrl_data) {
    struct ps3_map *map = (struct ps3_map *)bt_data->input;

    memset((void *)ctrl_data, 0, sizeof(*ctrl_data));

    ctrl_data->mask = (uint32_t *)ps3_mask;
    ctrl_data->desc = (uint32_t *)ps3_desc;

    for (uint32_t i = 0; i < ARRAY_SIZE(generic_btns_mask); i++) {
        if (map->buttons & ps3_btns_mask[i]) {
            ctrl_data->btns[0].value |= generic_btns_mask[i];
        }
    }

    if (!atomic_test_bit(&bt_data->flags, BT_INIT)) {
        for (uint32_t i = 0; i < ARRAY_SIZE(map->axes); i++) {
            bt_data->axes_cal[i] = -(map->axes[ps3_axes_idx[i]] - ps3_axes_meta[i].neutral);
        }
        atomic_set_bit(&bt_data->flags, BT_INIT);
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(map->axes); i++) {
        ctrl_data->axes[i].meta = &ps3_axes_meta[i];
        ctrl_data->axes[i].value = map->axes[ps3_axes_idx[i]] - ps3_axes_meta[i].neutral + bt_data->axes_cal[i];
    }

}