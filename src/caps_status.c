/*
 * zmk-feature-caps: Caps Lock / Caps Word status widget
 * Icons for very small displays; text for larger ones.
 * Explicit init function (no ZMK display widget macros).
 */

#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>

/* HID indicators (Caps Lock LED from host) are optional in some builds. */
#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid.h> /* ZMK_HID_LED_CAPS_LOCK or HID_KBD_LED_CAPS_LOCK */
#endif

/* ---- Compatibility shim for different ZMK versions of the CAPS LED bit ---- */
#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
#ifndef ZMK_HID_LED_CAPS_LOCK
#define ZMK_HID_LED_CAPS_LOCK HID_KBD_LED_CAPS_LOCK
#endif
#endif
/* -------------------------------------------------------------------------- */

/* Caps Word is optional: compile only if ZMK defines it AND the indicator option is enabled. */
#if defined(CONFIG_ZMK_CAPS_WORD)
#include <zmk/events/caps_word_state_changed.h>
#endif

/* -------------------------------------------------------------------------- */
/* State                                                                      */
/* -------------------------------------------------------------------------- */

static lv_obj_t *widget_obj;      /* root object created by init fn */
static lv_obj_t *caps_canvas;     /* used in icon mode (small displays) */
static lv_obj_t *caps_label;      /* used in text mode (larger displays) */

static bool use_icons = true;
static bool caps_lock_on = false;

#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
static bool caps_word_on = false;
#endif

/* 12×12 icon bitmaps: 1 = pixel on (white), 0 = off (black) */
static const uint16_t caps_lock_bitmap[12] = {
    0b000111111000,
    0b001000000100,
    0b010001110010,
    0b010001010010,
    0b010001110010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b001000000100,
    0b000111111000,
    0b000000000000
};

/* Outline/“off” bitmap so the widget is visible even when inactive */
static const uint16_t caps_outline_bitmap[12] = {
    0b000111111000,
    0b001000000100,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b010000000010,
    0b001000000100,
    0b000111111000,
    0b000000000000
};

#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
static const uint16_t caps_word_bitmap[12] = {
    0b111111111111,
    0b101100111110,
    0b101010101010,
    0b101100111110,
    0b101010101010,
    0b101010111110,
    0b100000001110,
    0b100111110010,
    0b100000001110,
    0b100111110010,
    0b100000001110,
    0b111111111111
};
#endif

/* -------------------------------------------------------------------------- */
/* Icon rendering helpers (TRUE_COLOR canvas, 12x12)                          */
/* -------------------------------------------------------------------------- */

static lv_color_t icon_buf[12 * 12]; /* TRUE_COLOR buffer */

static void icon_canvas_init(lv_obj_t *parent) {
    caps_canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(caps_canvas, icon_buf, 12, 12, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(caps_canvas);
}

static void icon_canvas_fill(lv_color_t color) {
    lv_draw_rect_dsc_t rect;
    lv_draw_rect_dsc_init(&rect);
    rect.bg_color = color;
    rect.bg_opa = LV_OPA_COVER;
    lv_canvas_draw_rect(caps_canvas, 0, 0, 12, 12, &rect);
}

static void icon_canvas_draw_bitmap(const uint16_t rows[12]) {
    icon_canvas_fill(lv_color_black());
    for (int y = 0; y < 12; y++) {
        uint16_t row = rows[y];
        for (int x = 0; x < 12; x++) {
            if ((row >> (11 - x)) & 0x1) {
                lv_canvas_set_px_color(caps_canvas, x, y, lv_color_white());
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Text mode helpers                                                           */
/* -------------------------------------------------------------------------- */

static void label_init(lv_obj_t *parent) {
    caps_label = lv_label_create(parent);
    lv_obj_center(caps_label);
}

static void label_set_text_cstr(const char *txt) {
    lv_label_set_text(caps_label, txt ? txt : "");
}

/* -------------------------------------------------------------------------- */
/* Display update                                                              */
/* -------------------------------------------------------------------------- */

static void update_display(void) {
    if (!widget_obj) {
        return;
    }

    if (use_icons) {
        if (!caps_canvas) {
            icon_canvas_init(widget_obj);
        }
        /* Choose which icon to show; draw outline when inactive so it's visible */
        if (caps_lock_on) {
            icon_canvas_draw_bitmap(caps_lock_bitmap);
#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
        } else if (caps_word_on) {
            icon_canvas_draw_bitmap(caps_word_bitmap);
#endif
        } else {
            icon_canvas_draw_bitmap(caps_outline_bitmap);
        }
    } else {
        if (!caps_label) {
            label_init(widget_obj);
        }
        if (caps_lock_on) {
            label_set_text_cstr("CAPS LOCK");
#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
        } else if (caps_word_on) {
            label_set_text_cstr("CAPS WORD");
#endif
        } else {
            label_set_text_cstr("caps");
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Public init (called from nice_view_plus/status.c)                           */
/* -------------------------------------------------------------------------- */

lv_obj_t *zmk_widget_caps_status_init(lv_obj_t *parent) {
    /* Decide icon vs text based on display height (<=32px => icon mode) */
    lv_disp_t *disp = lv_disp_get_default();
    use_icons = (lv_disp_get_ver_res(disp) <= 32);

    widget_obj = lv_obj_create(parent);

    /* Transparent root so it overlays the canvas cleanly; bring to front */
    lv_obj_set_style_bg_opa(widget_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(widget_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(widget_obj, 0, 0);
    lv_obj_move_foreground(widget_obj);

    /* Draw once now (children created lazily) */
    update_display();
    return widget_obj;
}

/* -------------------------------------------------------------------------- */
/* Event listeners                                                             */
/* -------------------------------------------------------------------------- */

#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
static int hid_listener(const zmk_event_t *eh) {
    const struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    if (ev) {
        caps_lock_on = (ev->indicators & ZMK_HID_LED_CAPS_LOCK);
        update_display();
    }
    return 0;
}
ZMK_LISTENER(caps_hid_status, hid_listener)
ZMK_SUBSCRIPTION(caps_hid_status, zmk_hid_indicators_changed)
#endif /* CONFIG_ZMK_HID_INDICATORS */

#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
static int caps_word_listener(const zmk_event_t *eh) {
    const struct zmk_caps_word_state_changed *ev = as_zmk_caps_word_state_changed(eh);
    if (ev) {
        caps_word_on = ev->state;
        update_display();
    }
    return 0;
}
ZMK_LISTENER(caps_word_status, caps_word_listener)
ZMK_SUBSCRIPTION(caps_word_status, zmk_caps_word_state_changed)
#endif
