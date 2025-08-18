/*
 * zmk-feature-caps: Caps Lock / Caps Word status widget
 * Icons for very small displays; text for larger ones.
 */

#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>

/* Caps Word is optional: only compile if the base ZMK defines it AND the
 * module option to show a Caps Word indicator is enabled. */
#if defined(CONFIG_ZMK_CAPS_WORD)
#include <zmk/events/caps_word_state_changed.h>
#endif

/* -------------------------------------------------------------------------- */
/* State                                                                      */
/* -------------------------------------------------------------------------- */

static lv_obj_t *widget_obj;      /* root object returned by init fn */
static lv_obj_t *caps_canvas;     /* used for icon mode (small displays) */
static lv_obj_t *caps_label;      /* used for text mode (larger displays) */

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

static lv_color_t icon_buf[12 * 12]; /* TRUE_COLOR; ZMK already uses TRUE_COLOR canvases */

static void icon_canvas_init(lv_obj_t *parent) {
    caps_canvas = lv_canvas_create(parent);
    /* A 12x12 TRUE_COLOR canvas; ZMK/nice_view_plus also uses TRUE_COLOR for its canvases. */
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
    /* Draw white pixels on black background per 12-bit row. */
    icon_canvas_fill(lv_color_black());
    for (int y = 0; y < 12; y++) {
        uint16_t row = rows[y];
        for (int x = 0; x < 12; x++) {
            bool on = (row >> (11 - x)) & 0x1;
            if (on) {
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
        /* Choose which icon to show, or clear if none active */
        if (caps_lock_on) {
            icon_canvas_draw_bitmap(caps_lock_bitmap);
#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
        } else if (caps_word_on) {
            icon_canvas_draw_bitmap(caps_word_bitmap);
#endif
        } else {
            icon_canvas_fill(lv_color_black());
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
            label_set_text_cstr("");
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Widget init                                                                 */
/* -------------------------------------------------------------------------- */

static void detect_mode(void) {
    /* Decide icon vs text based on display vertical resolution (same logic you used) */
    lv_disp_t *disp = lv_disp_get_default();
    int h = lv_disp_get_ver_res(disp);
    use_icons = (h <= 32);

    /* Create the widget root container (returned by the generated init fn) */
    widget_obj = lv_obj_create(lv_obj_get_parent(lv_scr_act())); /* parent set later by init fn */
    /* The generated init fn will re-parent widget_obj appropriately for ZMK’s display manager. */

    /* Child init is deferred to first update to ensure correct parent is set. */
    update_display();
}

/* Register the widget: this generates `lv_obj_t *zmk_widget_caps_status_init(lv_obj_t *parent)` */
ZMK_DISPLAY_WIDGET_LISTENER(caps_status, widget_obj, update_display)
ZMK_DISPLAY_WIDGET_INIT_FN(caps_status, detect_mode)

/* -------------------------------------------------------------------------- */
/* Event listeners                                                             */
/* -------------------------------------------------------------------------- */

static int hid_listener(const struct zmk_event_header *eh) {
    const struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    if (ev) {
        caps_lock_on = (ev->indicators & HID_CAPS_LOCK);
        update_display();
    }
    return 0;
}

/* Use a unique listener name to avoid clashing with the widget listener symbol */
ZMK_LISTENER(caps_hid_status, hid_listener)
ZMK_SUBSCRIPTION(caps_hid_status, zmk_hid_indicators_changed)

#if defined(CONFIG_ZMK_CAPS_WORD) && IS_ENABLED(CONFIG_ZMK_FEATURE_CAPS_WORD_INDICATOR)
static int caps_word_listener(const struct zmk_event_header *eh) {
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