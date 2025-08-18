#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/caps_word_state_changed.h>
#include <zmk/display.h>
#include <zmk/display/widgets/icon.h>
#include <zmk/display/widgets/label.h>

#ifdef CONFIG_ZMK_CAPS_WORD
#include <zmk/events/caps_word_state_changed.h>
#else
// If someone forces this file to compile without Caps Word, fail loudly at compile-time
#error "caps_status.c requires CONFIG_ZMK_CAPS_WORD (ZMK Caps Word feature)."
#endif

static struct zmk_widget_icon caps_icon;
static struct zmk_widget_label caps_label;
static lv_obj_t *widget_obj;

static bool caps_lock_on = false;
static bool caps_word_on = false;
static bool use_icons = true;

// 12×12 icons
static const uint8_t caps_lock_bitmap[] = {
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

static const uint8_t caps_word_bitmap[] = {
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

static const uint8_t empty_bitmap[] = {
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b000000000000
};

static void update_display() {
    if (use_icons) {
        if (caps_lock_on) {
            zmk_widget_icon_set_bitmap(&caps_icon, caps_lock_bitmap, 12, 12);
        } else if (caps_word_on) {
            zmk_widget_icon_set_bitmap(&caps_icon, caps_word_bitmap, 12, 12);
        } else {
            zmk_widget_icon_set_bitmap(&caps_icon, empty_bitmap, 12, 12);
        }
    } else {
        if (caps_lock_on) {
            zmk_widget_label_set_text(&caps_label, "CAPS LOCK");
        } else if (caps_word_on) {
            zmk_widget_label_set_text(&caps_label, "CAPS WORD");
        } else {
            zmk_widget_label_set_text(&caps_label, "");
        }
    }
}

static void detect_mode() {
    lv_disp_t *disp = lv_disp_get_default();
    int height = lv_disp_get_ver_res(disp);

    if (height <= 32) {
        use_icons = true;
        widget_obj = zmk_widget_icon_init(&caps_icon);
    } else {
        use_icons = false;
        widget_obj = zmk_widget_label_init(&caps_label);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(caps_status, widget_obj, update_display)
ZMK_DISPLAY_WIDGET_INIT_FN(caps_status, detect_mode)

static int hid_listener(const struct zmk_event_header *eh) {
    const struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    if (ev) {
        caps_lock_on = (ev->indicators & HID_CAPS_LOCK);
        update_display();
    }
    return 0;
}

static int caps_word_listener(const struct zmk_event_header *eh) {
    const struct zmk_caps_word_state_changed *ev = as_zmk_caps_word_state_changed(eh);
    if (ev) {
        caps_word_on = ev->state;
        update_display();
    }
    return 0;
}

/* Renamed listener symbol to avoid any collision with the widget symbol */
ZMK_LISTENER(caps_hid_status, hid_listener)
ZMK_SUBSCRIPTION(caps_hid_status, zmk_hid_indicators_changed)

ZMK_LISTENER(caps_word_status, caps_word_listener)
ZMK_SUBSCRIPTION(caps_word_status, zmk_caps_word_state_changed)
