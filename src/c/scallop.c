#include <pebble.h>
#include "gbitmap_color_palette_manipulator.h"
#include <pebble-effect-layer/pebble-effect-layer.h>

static Window *s_window;
static Layer *s_canvas_layer;
static GBitmap *s_bitmap;

GPoint screencenter;

#define MINUTE_HAND_LENGTH 48
#define HOUR_HAND_LENGTH 32

#define MINUTE_HAND_LENGTH_EMERY 68
#define HOUR_HAND_LENGTH_EMERY 45

#define SETTINGS_KEY 1
typedef struct ClaySettings {
  GColor bg_color;
  GColor fg_color;
  GColor minute_color;
  GColor hour_color;
  bool invert;
} ClaySettings;
static ClaySettings settings;

#ifndef PBL_COLOR
EffectLayer* effect_layer;
#endif

float angle(const int value, const int max){
  if(value == 0 || value == max)
    return 0;
  return TRIG_MAX_ANGLE * value / max;
}

float angle_hour(const tm * const time, const bool with_delta){
  const int hour = time->tm_hour % 12;
  if(with_delta){
    return angle(hour * 50 + time->tm_min * 50 / 60, 600);
  }
  return angle(hour, 12);
}

GPoint gpoint_on_circle(const GPoint center, const int angle, const int radius){
  const int diameter = radius * 2;
  const GRect grect_for_polar = GRect(center.x - radius + 1, center.y - radius + 1, diameter, diameter);
  return gpoint_from_polar(grect_for_polar, GOvalScaleModeFitCircle, angle);
}

static void update_minute_hand(Layer *layer, GContext * ctx, uint8_t minutes){
  #ifdef PBL_PLATFORM_EMERY
  const GPoint hand_end = gpoint_on_circle(screencenter, angle(minutes, 60), MINUTE_HAND_LENGTH_EMERY);
  #else
  const GPoint hand_end = gpoint_on_circle(screencenter, angle(minutes, 60), MINUTE_HAND_LENGTH);
  #endif
  graphics_context_set_stroke_width(ctx, 6);
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, settings.minute_color);
  #else
  graphics_context_set_stroke_color(ctx, GColorWhite);
  #endif
  graphics_draw_line(ctx, screencenter, hand_end);
}

static void update_hour_hand(Layer *layer, GContext * ctx, const tm * const time){
  #ifdef PBL_PLATFORM_EMERY
  const GPoint hand_end = gpoint_on_circle(screencenter, angle_hour(time, true), HOUR_HAND_LENGTH_EMERY);
  #else
  const GPoint hand_end = gpoint_on_circle(screencenter, angle_hour(time, true), HOUR_HAND_LENGTH);
  #endif
  graphics_context_set_stroke_width(ctx, 6);
  #ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, settings.hour_color);
  #else
  graphics_context_set_stroke_color(ctx, GColorWhite);
  #endif
  graphics_draw_line(ctx, screencenter, hand_end);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  #ifdef PBL_PLATFORM_EMERY
  graphics_draw_rotated_bitmap(ctx, s_bitmap, GPoint(96, 96), DEG_TO_TRIGANGLE(6*tick_time->tm_sec), screencenter);
  #else
  graphics_draw_rotated_bitmap(ctx, s_bitmap, GPoint(68, 68), DEG_TO_TRIGANGLE(6*tick_time->tm_sec), screencenter);
  #endif
  update_minute_hand(layer, ctx, tick_time->tm_min);
  update_hour_hand(layer, ctx, tick_time);
}

static void get_bitmap(){
  #ifdef PBL_PLATFORM_EMERY
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SCALLOP_BIG);
  #else
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SCALLOP);
  #endif
}

static void prv_window_load(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  get_bitmap();
  s_canvas_layer = layer_create(bounds);
  screencenter = GPoint(bounds.size.w/2, bounds.size.h/2);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_canvas_layer);
  
  #ifndef PBL_COLOR
  effect_layer = effect_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_add_child(window_get_root_layer(s_window), effect_layer_get_layer(effect_layer));
  #endif
  
  layer_mark_dirty(s_canvas_layer);
}

static void prv_window_unload(Window *window) {
  #ifndef PBL_COLOR
  effect_layer_destroy(effect_layer);
  #endif
  layer_destroy(s_canvas_layer);
  gbitmap_destroy(s_bitmap);
}

static void update_colors(){
  #ifdef PBL_COLOR
  window_set_background_color(s_window, settings.bg_color);
  get_bitmap();
  replace_gbitmap_color(GColorWhite, GColorFromHEX(0xbd35f4), s_bitmap, NULL); //random number which is probably not in the color selector
  replace_gbitmap_color(GColorBlack, settings.fg_color, s_bitmap, NULL);
  replace_gbitmap_color(GColorFromHEX(0xbd35f4), settings.bg_color, s_bitmap, NULL);
  #else
  if (settings.invert){
    effect_layer_add_effect(effect_layer, effect_invert, NULL);
  }
  else{
    effect_layer_remove_effect(effect_layer);
  }
  #endif
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if(bg_color_t) {
    settings.bg_color = GColorFromHEX(bg_color_t->value->int32);
  }
  Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if(fg_color_t) {
    settings.fg_color = GColorFromHEX(fg_color_t->value->int32);
  }
  Tuple *minute_color_t = dict_find(iter, MESSAGE_KEY_MinuteHand);
  if(minute_color_t) {
    settings.minute_color = GColorFromHEX(minute_color_t->value->int32);
  }
  Tuple *hour_color_t = dict_find(iter, MESSAGE_KEY_HourHand);
  if(hour_color_t) {
    settings.hour_color = GColorFromHEX(hour_color_t->value->int32);
  }
  Tuple *invert_t = dict_find(iter, MESSAGE_KEY_Invert);
  if(invert_t) {
    settings.invert = invert_t->value->int32 == 1;
  }
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  update_colors();
}

static void prv_init(void) {
  // for color settings
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = false;
  window_stack_push(s_window, animated);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // defaults
  if (!persist_exists(SETTINGS_KEY)){
    settings.bg_color = GColorBlack;
    settings.fg_color = GColorWhite;
    settings.minute_color = GColorBlack;
    settings.hour_color = GColorBlack;
    settings.invert = true;
  }
  else{
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
  update_colors();
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
