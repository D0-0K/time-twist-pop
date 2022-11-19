#include <pebble.h>
#include "main.h"

#define ROUND PBL_IF_ROUND_ELSE(true, false)

static Window *s_window;
static Layer *s_background_layer;

static GBitmap *s_image;

uint8_t *bitmap_data;
int bytes_per_row;

// An instance of the struct
static ClaySettings settings;

int Background1 = 0xFFFFFF;
int Background2 = 0xAAAAAA;
// Set the height and width of the image
int IMAGE_HEIGHT = 180;
int IMAGE_WIDTH = 180;

static void prv_default_settings() {
    settings.col_number = 1;
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "received settings");
    Tuple *color_t = dict_find(iter, MESSAGE_KEY_ColorNumber);
    if (color_t) {
       settings.col_number = atoi(color_t->value->cstring);
}
  // Save the new settings to persistent storage
  prv_save_settings();
}

// Read settings from persistent storage
static void prv_load_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "loading settings");
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}
static void update_colors() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "update colors");
  layer_mark_dirty(s_background_layer);
  if (settings.col_number == 1) {
    Background1 = 0xFFFFFF;
    Background2 = 0xAAAAAA;
  } else if (settings.col_number == 2) {
    Background1 = 0x000000;
    Background2 = 0x555555;
  }
}

// Update the display elements
static void prv_update_display() {
  update_colors();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Update Display");
  layer_mark_dirty(s_background_layer);
}
// Save the settings to persistent storage
static void prv_save_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "save settings");
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  prv_update_display();
}

// Get the color of the pixel at (x,y)
GColor get_pixel_color(int x, int y){
	return (GColor){ .argb = bitmap_data[y*bytes_per_row + x] };
}

// Set the color of the pixel at (x,y) to "color"
void set_pixel_color(int x, int y, GColor color){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "set pixel color");
	bitmap_data[y*bytes_per_row + x] = color.argb;
}

// Go through pixels using a nested loop and replace old_color with new_color
void replace_colors(int pixel_width, int pixel_height, GColor old_color, GColor new_color){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Replacing Colours");
	int max_y = pixel_height; //Use 28 for just the hat
	int max_x = pixel_width;
	
	for(int y = 0; y < max_y; y++){
		for(int x = 0; x < max_x; x++){
			GColor pixel_color = get_pixel_color(x,y);
			if(gcolor_equal(pixel_color, old_color)){
			  set_pixel_color(x, y, new_color);
			}
		}
	}
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Background update proc");
 // Get the bounds of the image
  GRect bitmap_bounds = gbitmap_get_bounds(s_image);

// Set the compositing mode (GCompOpSet is required for transparency)
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

 // Draw the image
  graphics_draw_bitmap_in_rect(ctx, s_image, bitmap_bounds);
  
  replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorWhite, GColorFromHEX(Background1));
  replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorLightGray, GColorFromHEX(Background2));
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "background layer created");
  s_background_layer = layer_create(bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);

  
  if (ROUND) {
    s_image = gbitmap_create_with_resource(RESOURCE_ID_ROUND);
  } else {
    s_image = gbitmap_create_with_resource(RESOURCE_ID_RECT);
  }
  
  bitmap_data = gbitmap_get_data(s_image);
  bytes_per_row = gbitmap_get_bytes_per_row(s_image);
  
  update_colors();
 
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_background_layer);
  gbitmap_destroy(s_image);
}

static void prv_init(void) {
  prv_load_settings();
  s_window = window_create();

  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
