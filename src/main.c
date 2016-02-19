#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  text_layer_set_text(s_time_layer, s_buffer);
  
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[32];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d", new_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BM_BACKGROUND);
  
  //Create layers, place them
  s_bitmap_layer = bitmap_layer_create(bounds);
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, bounds.size.h));
  s_battery_layer = text_layer_create(GRect(0, 0, bounds.size.w, PBL_IF_ROUND_ELSE(58, 52)));

  
  //Background Layer setup
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  
  // Battery Layer setup
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);  
  
  
  //Time Layer Setup
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  //Add layers to root
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  battery_handler(battery_state_service_peek());
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_destroy(s_bitmap);
}

static void init(){
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {.load = main_window_load, .unload = main_window_unload});  
  window_stack_push(s_main_window, true);
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
}

static void deinit(){
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}