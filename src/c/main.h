#pragma once
#include <pebble.h>

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
    int col_number;
    bool enable_vibrate_bluetooth;
    int vibe_type;
    bool toggle_date;
} ClaySettings;

static void prv_default_settings();
static void prv_load_settings();
static void prv_save_settings();
static void update_colors();
static void prv_update_display();
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);
