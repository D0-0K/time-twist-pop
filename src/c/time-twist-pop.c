#include <pebble.h>
#include "main.h"

// Time Twist Pop - By Dook
// Made during the Rebble Hackathon #001
//
//
// Colour Replacing Code from Learning C with Pebble Exercise 5-2
// Special Thanks to Rebble, Will0, Lavender, Nikki & Kennedn as well Doovles & Roo. 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ROUND PBL_IF_ROUND_ELSE(true, false)

static Window *s_window;
static Layer *s_background_layer;
static Layer *s_hands_layer;

static GBitmap *s_image;

uint8_t *bitmap_data;
int bytes_per_row;

static ClaySettings settings;

int Background1 = 0xFFFFFF;
int Background2 = 0xAAAAAA;
int BackgroundG2 = 0xAA5500;
int BackgroundG3 = 0xAA5555;
int HourHandCol = 0xFF5500;
int MinHandCol = 0x555555;
int IMAGE_HEIGHT = 180;
int IMAGE_WIDTH = 180;

static void prv_default_settings() {
    settings.col_number = 1;
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *color_t = dict_find(iter, MESSAGE_KEY_ColorNumber);
    if (color_t) {
       settings.col_number = atoi(color_t->value->cstring);
  }
  prv_save_settings();
}

// Read settings from persistent storage
static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Get the color of the pixel at (x,y)
GColor get_pixel_color(int x, int y){
	return (GColor){ .argb = bitmap_data[y*bytes_per_row + x] };
}

// Set the color of the pixel at (x,y) to "color"
void set_pixel_color(int x, int y, GColor color){
	bitmap_data[y*bytes_per_row + x] = color.argb;
}

// Go through pixels using a nested loop and replace old_color with new_color
void replace_colors(int pixel_width, int pixel_height, GColor old_color, GColor new_color){
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

static void draw_hand(GContext *ctx, GPoint start, int angle, int inset, int length, int thickness, GColor color) {
 // Calculate where the end point of the hand goes
  GPoint hand_end = {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)length / TRIG_MAX_RATIO) + start.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)length / TRIG_MAX_RATIO) + start.y,
  };

  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, thickness);

  // Draw stroke
   graphics_draw_line(ctx, start, hand_end);
}

static void color_set(int bg, int fg, int hour, int min, int gaybg, int gayfg) {
  Background1 = bg;
  Background2 = fg;
  BackgroundG2 = gaybg;
  BackgroundG3 = gayfg;
  HourHandCol = hour;
  MinHandCol = min;
}

static void image_create(char round, char rect) {
  if (ROUND) {
    s_image = gbitmap_create_with_resource(round);
  } else {
    s_image = gbitmap_create_with_resource(rect);
  } 
}

static void load_image() {
  gbitmap_destroy(s_image);
  if (settings.col_number < 13) {
    image_create(RESOURCE_ID_ROUND,RESOURCE_ID_RECT);
  } else if (settings.col_number == 13) {
    image_create(RESOURCE_ID_AROROUND,RESOURCE_ID_ARORECT);
  } else if (settings.col_number == 14) {
    image_create(RESOURCE_ID_GFROUND,RESOURCE_ID_GFRECT);
  } else if (settings.col_number == 15) {
    image_create(RESOURCE_ID_LESROUND,RESOURCE_ID_LESRECT);
  } else if (settings.col_number == 16) {
    image_create(RESOURCE_ID_GAYROUND,RESOURCE_ID_GAYRECT);
  } else if (settings.col_number > 16) {
    image_create(RESOURCE_ID_PRIDEROUND,RESOURCE_ID_PRIDERECT);
  }
}

static void col_replace() {
  if (settings.col_number < 16) {
    replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorArmyGreen, GColorFromHEX(Background1));
    replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorWindsorTan, GColorFromHEX(Background2));
  } else if (settings.col_number > 16) {
    replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorArmyGreen, GColorFromHEX(Background2));
    replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorWindsorTan, GColorFromHEX(BackgroundG2));
    replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorRoseVale, GColorFromHEX(BackgroundG3));
  }
}

static void update_colors() {
  layer_mark_dirty(s_background_layer);
  load_image();
  if (settings.col_number == 1) {
    color_set(0xFFFFFF,0xAAAAAA,0xFF5500,0x555555,0,0);
  } else if (settings.col_number == 2) {
    color_set(0x000000,0x555555,0xFFAA00,0xFFFFFF,0,0);
  } else if (settings.col_number == 3) {
    color_set(0xFFFFFF,0xFFAAAA,0xFF0055,0x005555,0,0);
  } else if (settings.col_number == 4) {
    color_set(0xFFFFFF,0xAAAAFF,0x000055,0xFF0055,0,0);
  } else if (settings.col_number == 5) {
    color_set(0xFF5555,0xFFAAAA,0x55FF00,0x550000,0,0);
  } else if (settings.col_number == 6) {
    color_set(0xFFFFFF,0xAAFFAA,0xFF0055,0x005555,0,0);
  } else if (settings.col_number == 7) {
    color_set(0xAA0000,0xFF5555,0xFFFFAA,0x000000,0,0);
  } else if (settings.col_number == 8) {
    color_set(0x5555AA,0xAAAAFF,0xFFFF55,0xFF5500,0,0);
  } else if (settings.col_number == 9) {
    color_set(0xAA55FF,0xAAAAFF,0x00FFAA,0x005500,0,0);
  } else if (settings.col_number == 10) {
    color_set(0x0000AA,0x5555FF,0xFFFFFF,0x000055,0,0);
  } else if (settings.col_number == 13) {
    color_set(0xFFFFFF,0,0x000000,0xFF5500,0,0);
  } else if (settings.col_number == 14) {
    color_set(0xFFFFFF,0,0x000000,0xFFAA55,0,0);
  } else if (settings.col_number == 15) {
    color_set(0xFFFFFF,0,0x00AA55,0x000000,0,0);
  } else if (settings.col_number == 16) {
    color_set(0xFF0000,0,0xFFFFFF,0x000000,0,0);
  } else if (settings.col_number == 17) {
    color_set(0xFFFFFF,0x55AAFF,0xFF5555,0x000055,0xFFFFFF,0xFFAAAA);
  } else if (settings.col_number == 18) {
    color_set(0xAA0055,0xAA0055,0xFFAAAA,0xFFFFFF,0xAA55AA,0x0000AA);
  } else if (settings.col_number == 19) {
    color_set(0xFFFFFF,0xFFFF55,0x000000,0xFF5555,0xFFFFFF,0xAAAAFF);
  } else if (settings.col_number == 20) {
    color_set(0xFFFFFF,0x550055,0x000000,0xFF5500,0xFFFFFF,0xAAAAAA);
  } else if (settings.col_number == 21) {
    color_set(0xFFFFFF,0xAAFFAA,0x000000,0xFFAA00,0xFFFFFF,0xAAAAAA);
  } else if (settings.col_number == 22) {
    color_set(0xFFFF00,0xFF00AA,0x555555,0x550000,0xFFFF00,0x00AAFF);
  } else if (settings.col_number == 23) {
    color_set(0x00AAFF,0xFF00AA,0xAA0055,0x555555,0x00FF00,0x00AAFF);
  } else if (settings.col_number == 24) {
    color_set(0xFFFF00,0xAA00FF,0xFF0000,0x550055,0xFFFFFF,0x00AA00);
  } else if (settings.col_number == 25) {
    color_set(0xFFFFFF,0xAAAAAA,0x555555,0x0000AA,0xFFFFFF,0x55AAFF);
  } else if (settings.col_number == 26) {
    color_set(0xFFFFFF,0xAAAAAA,0x555555,0xAA0055,0xFFFFFF,0xFFAAFF);
  }
  col_replace();
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 // Update minute/hour hands if a minute has passed
  if ((units_changed & MINUTE_UNIT) != 0) {
    layer_mark_dirty(s_hands_layer);
  }
}

static void initialize_time() {
  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_update_display() {
  update_colors();
  layer_mark_dirty(s_background_layer);
  layer_mark_dirty(s_hands_layer);
  initialize_time();
}
// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  prv_update_display();
}

static void background_update_proc(Layer *layer, GContext *ctx) {
 // Get the bounds of the image
  GRect bitmap_bounds = gbitmap_get_bounds(s_image);
 // Set the compositing mode (GCompOpSet is required for transparency)
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
 // Draw the image
  graphics_draw_bitmap_in_rect(ctx, s_image, bitmap_bounds);
}

static void hands_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint s_center = grect_center_point(&bounds);
  
  const int16_t max_hour_length = PBL_IF_ROUND_ELSE(((bounds.size.w - 10) / 2), (bounds.size.w - 10) / 2);
  const int16_t max_min_length = PBL_IF_ROUND_ELSE(((bounds.size.w - 22) / 2), (bounds.size.w - 15) / 2);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
 // calculate minute hand
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
 // draw minute hand
  draw_hand(ctx, s_center, minute_angle, 10, max_min_length, 8, GColorFromHEX(MinHandCol));
  
 // calculate hour hand
  int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
 // draw hour hand
  draw_hand(ctx, s_center, hour_angle, 10, max_hour_length * 0.7, 16, GColorFromHEX(HourHandCol));
  
 // Center Dot
  graphics_context_set_fill_color(ctx, GColorFromHEX(Background1));
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 2, bounds.size.h / 2 - 2, 6, 6), 3, GCornersAll);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_layer = layer_create(bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_layer_update_proc);
  layer_add_child(window_layer, s_hands_layer);
  
  initialize_time();
  
  load_image();
  
  bitmap_data = gbitmap_get_data(s_image);
  bytes_per_row = gbitmap_get_bytes_per_row(s_image);
  
  update_colors();
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_hands_layer);
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
  app_event_loop();
  prv_deinit();
}
