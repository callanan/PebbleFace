#include <pebble.h>

static GFont station_font_30;
Window *window;
TextLayer *preText_layer, *midText_layer, *postText_layer, *dateText_layer;
const char *hour_label[] = {"twelve","one","two","three","four","five","six","seven","eight","nine","ten","eleven","twelve"};
const char *min_label[] = {"o'clock",
                           "five past",
                           "ten past",
                           "qtr past",
                           "20 past",
                           "25 past",
                           "half",
                           "25 to",
                           "twenty to",
                           "quarter to",
                           "ten to",
                           "five to"};

const char *before_label[] = {"it's almost","it's nearly"};
const char *after_label[] = {"just after","just gone"};
const char *exact_label[] = {"it's","it is now"};
enum proximity { exact=-1, before=0, after=1 };
int lastHour = 0;
int lastMin = 0;

const char* preString(enum proximity prox){
  switch(prox){
    case after:
      return after_label[rand() % 2];
    case before:
      return before_label[rand() % 2];
    default:
      return exact_label[rand() % 2];
  }
}

const char* midString(enum proximity prox, int min){
  switch(prox){
    case before:
      return min_label[(min+1) % 12];
    default:
      return min_label[min % 12];
  }
}

const char* postString(bool pastHour, int hour){
  switch(pastHour){
    case true:
      return hour_label[hour % 12];
    default:
      return hour_label[(hour + 1) % 12];
  }
}

void handle_timechanges(struct tm *tick_time, TimeUnits units_change){
  static char date_buffer[20];
  
  int hour = (tick_time->tm_hour % 12);
  int min = tick_time->tm_min / 5;
  if(lastHour == tick_time->tm_hour && lastMin == tick_time->tm_min){
    // Only update the clock if it has changed
    return;
  }
  lastHour = tick_time->tm_hour;
  lastMin = tick_time->tm_min;
  
  enum proximity prox = tick_time->tm_min % 5 == 0 ? exact : tick_time->tm_min % 5 >= 3 ? before : after;
  bool pastHour = tick_time->tm_min >= 33 ? false : true;
  
  text_layer_set_text(preText_layer, preString(prox));
  
  if((min == 11 && prox == before) || (min == 0 && (prox == exact || prox == after))){
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", "Here");
    text_layer_set_text(midText_layer, postString(pastHour, hour));
    text_layer_set_text(postText_layer, midString(prox, min));
  }
  else {
    text_layer_set_text(midText_layer, midString(prox, min));
    text_layer_set_text(postText_layer, postString(pastHour, hour));
  }
  
  strftime(date_buffer, sizeof(date_buffer),"%a %B %d", tick_time);
  text_layer_set_text(dateText_layer, date_buffer);
}

void handle_init(void) {
	// Create a window and text layer
	window = window_create();
	preText_layer = text_layer_create(GRect(0, 0, 144, 40));
  midText_layer = text_layer_create(GRect(0, 40, 144, 40));
  postText_layer = text_layer_create(GRect(0, 80, 144, 40));
  dateText_layer = text_layer_create(GRect(0, 130, 144, 40));
  
	// Set the text, font, and text alignment
  station_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_STATION_30));
	text_layer_set_font(preText_layer, station_font_30);
	text_layer_set_text_alignment(preText_layer, GTextAlignmentLeft);
  
  text_layer_set_font(midText_layer, station_font_30);
	text_layer_set_text_alignment(midText_layer, GTextAlignmentLeft);
  
  text_layer_set_font(postText_layer, station_font_30);
	text_layer_set_text_alignment(postText_layer, GTextAlignmentLeft);

  text_layer_set_font(dateText_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(dateText_layer, GTextAlignmentCenter);

	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(preText_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(midText_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(postText_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(dateText_layer));
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_timechanges);

	// Push the window
	window_stack_push(window, true);
	
	// App Logging!
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	// Destroy the text layer
	text_layer_destroy(preText_layer);
  text_layer_destroy(midText_layer);
  text_layer_destroy(postText_layer);
  text_layer_destroy(dateText_layer);
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
