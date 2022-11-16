#include <pebble.h>
#include "main.h"

#define ROUND PBL_IF_ROUND_ELSE(true, false)
#define ANTIALIASING true
#define ANIMATION_DURATION 500
#define ANIMATION_DELAY 600

typedef struct {
  uint8_t hours;
  uint8_t minutes;
} Time;

Layer *s_canvas_layer;
Window *s_main_window;
GBitmap *image;

// A struct for our specific settings (see main.h)
ClaySettings settings;

uint8_t *bitmap_data;
int bytes_per_row;
static GPoint s_center;
static Time s_last_time, s_anim_time;
static uint8_t s_radius = 0, s_anim_hours_60 = 0;
static uint8_t s_radius_final;
static uint8_t s_hour_margin = 8, s_min_margin = 5;
static bool s_animating = false;
int Background1 = 0xFFFFFF;
int Background2 = 0xAAAAAA;
int HourHandCol = 0xFF5500;
int MinHandCol = 0x555555;

// Set the correct screen height and width (checking for Pebble Time Round)
int HEIGHT = PBL_IF_RECT_ELSE(180,180);
int WIDTH = PBL_IF_RECT_ELSE(180,180);

// Set the height and width of the image
int IMAGE_HEIGHT = 180;
int IMAGE_WIDTH = 180;

static void col_schemes() {
  if(settings.col_number == 9 ) {
    Background1 = 0xAA55FF;
    Background2 = 0xAAAAFF;
    HourHandCol = 0x00FFAA;
    MinHandCol = 0x005500;
  } else if (settings.col_number == 8 ) {
    Background1 = 0x5555AA;
    Background2 = 0xAAAAFF;
    HourHandCol = 0xFFFFFF;
    MinHandCol = 0x000055;
  } else if (settings.col_number == 7 ) {
    Background1 = 0xAA0000;
    Background2 = 0xFF5555;
    HourHandCol = 0xFFFFAA;
    MinHandCol = 0x000000;
  } else if (settings.col_number == 6 ) {
    Background1 = 0xFFFFFF;
    Background2 = 0xAAFFAA;
    HourHandCol = 0xFF0055;
    MinHandCol = 0x005555;
  } else if (settings.col_number == 5 ) {
    Background1 = 0xFF5555;
    Background2 = 0xFFAAAA;
    HourHandCol = 0x55FF00;
    MinHandCol = 0x550000;
  } else if (settings.col_number == 4 ) {
    Background1 = 0xFFFFFF;
    Background2 = 0xAAAAFF;
    HourHandCol = 0x000055;
    MinHandCol = 0xFF0055;
  } else if (settings.col_number == 3 ) {
    Background1 = 0xFFFFFF;
    Background2 = 0xFFAAAA;
    HourHandCol = 0xFF0055;
    MinHandCol = 0x005555;
  } else if (settings.col_number == 2 ) {
    Background1 = 0x000000;
    Background2 = 0x555555;
    HourHandCol = 0xFFAA00;
    MinHandCol = 0xFFFFFF;
  } else if (settings.col_number == 1 ) {
    Background1 = 0xFFFFFF;
    Background2 = 0xAAAAAA;
    HourHandCol = 0xFF5500;
    MinHandCol = 0x555555;
  }
}

// Initialize the default settings
static void prv_default_settings() {
    settings.col_number = 1;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_proc();
}


// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *ctx) {
    Tuple *col_t = dict_find(iter, MESSAGE_KEY_ColorNumber);
    if (col_t) {
       settings.col_number = atoi(col_t->value->cstring);
}

  // Save the new settings to persistent storage

  prv_save_settings();

}

/*************************** AnimationImplementation **************************/

static void prv_animation_started(Animation *anim, void *ctx) {
  s_animating = true;
}

static void prv_animation_stopped(Animation *anim, bool stopped, void *ctx) {
  s_animating = false;
}

static void prv_animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if (handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = prv_animation_started,
      .stopped = prv_animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void prv_tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  // Redraw
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static int prv_hours_to_minutes(int hours_out_of_12) {
  return hours_out_of_12 * 60 / 12;
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect full_bounds = layer_get_bounds(layer);
  GRect bounds = layer_get_unobstructed_bounds(layer);
  s_center = grect_center_point(&bounds);

  // Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if (s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius + s_min_margin) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius + s_min_margin) / TRIG_MAX_RATIO) + s_center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (2 * s_hour_margin)) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * s_hour_margin)) / TRIG_MAX_RATIO) + s_center.y,
  };

  // Draw hands with positive length only
  graphics_context_set_antialiased(ctx, ANTIALIASING);
  graphics_context_set_stroke_width(ctx, 8);
  graphics_context_set_stroke_color(ctx, GColorFromHEX(MinHandCol));
  if (s_radius > s_min_margin) {
    graphics_draw_line(ctx, s_center, minute_hand);
  }
  graphics_context_set_stroke_width(ctx, 16);
  graphics_context_set_stroke_color(ctx, GColorFromHEX(HourHandCol));
  if (s_radius > 2 * s_hour_margin) {
    graphics_draw_line(ctx, s_center, hour_hand);
  }

  graphics_context_set_fill_color(ctx, GColorFromHEX(Background1));
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 2, bounds.size.h / 2 - 2, 6, 6), 3, GCornersAll);
}

static int prv_anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(dist_normalized * max / ANIMATION_NORMALIZED_MAX);
}

static void prv_radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = prv_anim_percentage(dist_normalized, s_radius_final);

  layer_mark_dirty(s_canvas_layer);
}

static void prv_hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = prv_anim_percentage(dist_normalized, prv_hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = prv_anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void prv_start_animation() {
  // Prepare animations
  static AnimationImplementation s_radius_impl = {
    .update = prv_radius_update
  };
  prv_animate(ANIMATION_DURATION, ANIMATION_DELAY, &s_radius_impl, false);

  static AnimationImplementation s_hands_impl = {
    .update = prv_hands_update
  };
  prv_animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &s_hands_impl, true);
}

static void prv_create_canvas() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_unobstructed_bounds(window_layer);

  s_radius_final = (bounds.size.w - 30) / 2;

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, prv_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

/*********************************** App **************************************/

// Event fires once, before the obstruction appears or disappears
static void prv_unobstructed_will_change(GRect final_unobstructed_screen_area, void *ctx) {
  if(s_animating) {
    return;
  }
  // Reset the clock animation
  s_radius = 0;
  s_anim_hours_60 = 0;
}

// Event fires once, after obstruction appears or disappears
static void prv_unobstructed_did_change(void *ctx) {
  if(s_animating) {
    return;
  }
  // Play the clock animation
  prv_start_animation();
}

static void prv_window_load(Window *window) {
  prv_create_canvas();

  prv_start_animation();

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);

  // Subscribe to the unobstructed area events
  UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_will_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);

}


//------------ Background

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

void draw(Layer *layer, GContext *ctx){
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	graphics_draw_bitmap_in_rect(ctx, image, GRect((WIDTH-IMAGE_WIDTH), HEIGHT-IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT));
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

//---------------------------------------------------------------------------------------------

void prv_init(void){
  prv_load_settings();
  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
  col_schemes();

  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  prv_tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });


	s_canvas_layer = layer_create(layer_get_bounds(window_get_root_layer(s_main_window)));
	layer_set_update_proc(s_canvas_layer, draw);
	layer_add_child(window_get_root_layer(s_main_window), s_canvas_layer);
	
       // Change the background to fit the screen ratio
       if (ROUND) {
	image = gbitmap_create_with_resource(RESOURCE_ID_ROUND);
	} else {
        image = gbitmap_create_with_resource(RESOURCE_ID_RECT);
        s_hour_margin = 0;
        s_min_margin = 6;
        }
         
	bitmap_data = gbitmap_get_data(image);
	bytes_per_row = gbitmap_get_bytes_per_row(image);
	
	replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorWhite, GColorFromHEX(Background1));
	replace_colors(IMAGE_WIDTH, IMAGE_HEIGHT, GColorLightGray, GColorFromHEX(Background2));
	
	window_stack_push(s_main_window, true);
	
}

static void prv_deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
