#include "graph_element.h"
#include "layout.h"
#include "preferences.h"
#include "staleness.h"

#define BOLUS_TICK_HEIGHT 7
#define MAX_PREDICTION_AGE_TO_SHOW_SECONDS (20*60)
#define NO_BG 32767

static GPoint center_of_point(int16_t x, int16_t y) {
  if (get_prefs()->point_shape == POINT_SHAPE_CIRCLE) {
    return GPoint(x + get_prefs()->point_width / 2, y + get_prefs()->point_width / 2);
  } else {
    return GPoint(x + get_prefs()->point_width / 2, y + get_prefs()->point_rect_height / 2);
  }
}

static void plot_point(int16_t x, int16_t y, GColor c, GContext *ctx) {
  graphics_context_set_fill_color(ctx, c);
  if (get_prefs()->point_shape == POINT_SHAPE_RECTANGLE) {
    graphics_fill_rect(ctx, GRect(x, y, get_prefs()->point_width, get_prefs()->point_rect_height), 0, GCornerNone);
  } else if (get_prefs()->point_shape == POINT_SHAPE_CIRCLE) {
    graphics_fill_circle(ctx, center_of_point(x, y), get_prefs()->point_width / 2);
  }
}

static void plot_tick(int16_t x, int16_t bottom_y, GContext *ctx, uint8_t point_width) {
  uint8_t width;
  if (point_width >= 5 && point_width % 2 == 1) {
    width = 3;
  } else {
    width = 2;
  }
  graphics_fill_rect(ctx, GRect(x + point_width / 2 - width / 2, bottom_y - BOLUS_TICK_HEIGHT, width, BOLUS_TICK_HEIGHT), 0, GCornerNone);
}

static int16_t bg_to_y(int16_t height, int16_t bg, Preferences *prefs) {
  // Graph lower bound, graph upper bound
  uint8_t graph_min = prefs->bottom_of_graph;
  uint16_t graph_max = prefs->top_of_graph;
  return (float)height - (float)(bg - graph_min) / (float)(graph_max - graph_min) * (float)height + 0.5f;
}

static int16_t index_to_x(int16_t i, uint16_t graph_width, uint16_t padding) {
  return graph_width - (get_prefs()->point_width + get_prefs()->point_margin) * (1 + i + padding) + get_prefs()->point_margin - get_prefs()->point_right_margin;
}

static int16_t bg_to_y_for_point(uint8_t height, int16_t bg, Preferences *prefs) {
  uint8_t diameter = prefs->point_shape == POINT_SHAPE_CIRCLE ? prefs->point_width : prefs->point_rect_height;
  uint8_t max = height - diameter;

  int16_t y = bg_to_y(height, bg, prefs) - diameter / 2;
  if (y < 0) {
    return 0;
  } else if (y > max) {
    return max;
  } else {
    return y;
  }
}

#ifdef PBL_COLOR
static GColor color_for_bg(int16_t bg, Preferences *prefs) {
  if (bg > prefs->top_of_range) {
    return prefs->colors[COLOR_KEY_POINT_HIGH];
  } else if (bg < prefs->bottom_of_range) {
    return prefs->colors[COLOR_KEY_POINT_LOW];
  } else {
    return prefs->colors[COLOR_KEY_POINT_DEFAULT];
  }
}

static GColor color_for_predicted_bg(int16_t bg, Preferences *prefs) {
  if (bg > prefs->top_of_range) {
    return get_prefs()->colors[COLOR_KEY_PREDICT_HIGH];
  } else if (bg < prefs->bottom_of_range) {
    return get_prefs()->colors[COLOR_KEY_PREDICT_LOW];
  } else {
    return get_prefs()->colors[COLOR_KEY_PREDICT_DEFAULT];
  }
}
#endif

static void fill_rect_gray(GContext *ctx, GRect bounds, GColor previous_color) {
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, previous_color);
}

static uint8_t sgv_graph_height(int16_t available_height) {
  return get_prefs()->basal_graph ? available_height - get_prefs()->basal_height : available_height;
}

static void graph_update_proc(Layer *layer, GContext *ctx) {
  int16_t i, x, y;
  GSize layer_size = layer_get_bounds(layer).size;
  uint16_t graph_width = layer_size.w;
  uint16_t graph_height = sgv_graph_height(layer_size.h);
  Preferences *prefs = get_prefs();

  GColor color = ((GraphData*)layer_get_data(layer))->color;
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);

  // Target range bounds
  // Draw bounds symmetrically, on the inside of the range
  fill_rect_gray(ctx, GRect(0, bg_to_y(graph_height, prefs->top_of_range, prefs) - 1, graph_width, 4), color);
  fill_rect_gray(ctx, GRect(0, bg_to_y(graph_height, prefs->bottom_of_range, prefs) - 2, graph_width, 4), color);

  // Horizontal gridlines
  uint8_t h_gridline_frequency = prefs->h_gridlines;
  if (h_gridline_frequency > 0) {
    uint8_t graph_min = prefs->bottom_of_graph;
    uint16_t graph_max = prefs->top_of_graph;
    for(int16_t g = 0; g < graph_max; g += h_gridline_frequency) {
      if (g <= graph_min || g == prefs->top_of_range || g == prefs->bottom_of_range) {
        continue;
      }
      y = bg_to_y(graph_height, g, prefs);
      for(x = 2; x < graph_width; x += 8) {
        graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 1, y));
      }
    }
  }

  DataMessage *data = last_data_message();
  if (data == NULL) {
    return;
  }

  uint16_t sgv_padding = sgv_graph_padding();

  // Prediction preprocessing
  uint8_t prediction_skip = 0;
  uint8_t prediction_padding = 0;
  if (data->prediction_length > 0) {
    // Show prediction points starting 2.5 minutes after the most recent SGV
    time_t future_boundary = data->received_at - data->recency + 5 * 60 * sgv_padding + 150;
    time_t prediction_start_time = data->received_at - data->prediction_recency;
    if (prediction_start_time < future_boundary) {
      prediction_skip = (future_boundary - prediction_start_time + 299) / 300;
    } else {
      prediction_padding = (prediction_start_time - future_boundary) / 300;
    }
  }

  uint16_t padding = data->prediction_length - prediction_skip + prediction_padding + sgv_padding;
  int16_t prediction_line_x = index_to_x(-1, graph_width, padding - sgv_padding) - prefs->point_margin - 1;

  // Line and point preprocessing
  static GPoint to_plot[GRAPH_MAX_SGV_COUNT];
  int16_t bg;
  for(i = 0; i < data->sgv_count; i++) {
    // XXX: JS divides by 2 to fit into 1 byte
    bg = data->sgvs[i] * 2;
    if(bg == 0) {
      continue;
    }
    x = index_to_x(i, graph_width, padding);
    y = bg_to_y_for_point(graph_height, bg, prefs);
    to_plot[i] = GPoint(x, y);
    // stop plotting if the SGV is off-screen
    if ((prefs->point_margin >= 0 && x < 0) || x <= -prefs->point_width) {
      break;
    }
  }
  uint8_t plot_count = i;

  // Basals
  if (prefs->basal_graph) {
    graphics_draw_line(ctx, GPoint(0, graph_height), GPoint(graph_width, graph_height));
    for(i = 0; i < data->sgv_count; i++) {
      uint8_t basal = data->graph_extra[i].basal;
      x = index_to_x(i, graph_width, padding);
      y = layer_size.h - basal;
      uint8_t width = prefs->point_width + prefs->point_margin;
      if (prefs->point_margin < 0 && i == 0) {
        // if points overlap and this is the rightmost point, extend its basal to the right edge
        width -= prefs->point_margin;
      }
      if (i == data->sgv_count - 1 && x >= 0) {
        // if this is the last point to draw, extend its basal data to the left edge
        width += x;
        x = 0;
      }
      graphics_draw_line(ctx, GPoint(x, y), GPoint(x + width - 1, y));
      if (basal > 1) {
        fill_rect_gray(ctx, GRect(x, y + 1, width, basal - 1), color);
      }
    }
    if (sgv_padding > 0) {
      x = index_to_x(padding - 1, graph_width, 0);
      graphics_fill_rect(ctx, GRect(x, graph_height, prediction_line_x - x + 1, prefs->basal_height), 0, GCornerNone);
    }
  }

  // Vertical line dividing history from prediction
  if (data->prediction_length > 0) {
    graphics_context_set_stroke_color(ctx, color);
#ifdef PBL_COLOR
    for (y = 1; y < layer_size.h; y += 2) {
      graphics_draw_pixel(ctx, GPoint(prediction_line_x, y));
    }
#else
    // BW displays can't distinguish future points by color, so make the dividing line more visible
    for (y = 1; y < layer_size.h; y += 4) {
      graphics_draw_line(ctx, GPoint(prediction_line_x, y), GPoint(prediction_line_x, y + 2));
    }
#endif
  }

  // Line
  if (prefs->plot_line) {
    graphics_context_set_stroke_width(ctx, prefs->plot_line_width);
    int16_t last_bg = NO_BG;
    GPoint last_center;
    for(i = 0; i < plot_count; i++) {
      bg = data->sgvs[i] * 2;
      if (bg == 0) {
        continue;
      }
      GPoint center = center_of_point(to_plot[i].x, to_plot[i].y);
      if (last_bg != NO_BG) {
        if (prefs->plot_line_is_custom_color) {
          graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(prefs->colors[COLOR_KEY_PLOT_LINE], color));
        } else {
          graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(color_for_bg(last_bg, prefs), color));
        }
        graphics_draw_line(ctx, center, last_center);
      }
      last_bg = bg;
      last_center = center;
    }
    graphics_context_set_stroke_width(ctx, 1);
  }

  // Points
  for(i = 0; i < plot_count; i++) {
    bg = data->sgvs[i] * 2;
    if (bg != 0) {
      plot_point(to_plot[i].x, to_plot[i].y, COLOR_FALLBACK(color_for_bg(bg, prefs), color), ctx);
    }
  }

  // Prediction
  if (data->prediction_length > 0 && data->received_at - data->prediction_recency >= time(NULL) - MAX_PREDICTION_AGE_TO_SHOW_SECONDS) {
    uint8_t* series[4] = {data->prediction_1, data->prediction_2, data->prediction_3, data->prediction_4};
    for(uint8_t si = 0; si < 4; si++) {
      for(i = prediction_skip; i < data->prediction_length; i++) {
        bg = series[si][i] * 2;
        if (bg == 0) {
          continue;
        }
        x = index_to_x(-i + prediction_skip - prediction_padding - 1, graph_width, padding - sgv_padding);
        y = bg_to_y_for_point(graph_height, bg, prefs);
        plot_point(x, y, COLOR_FALLBACK(color_for_predicted_bg(bg, prefs), GColorBlack), ctx);
      }
    }
  }

  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);

  // Boluses
  for(i = 0; i < data->sgv_count; i++) {
    if (data->graph_extra[i].bolus) {
      x = index_to_x(i, graph_width, padding);
      plot_tick(x, graph_height, ctx, prefs->point_width);
    }
  }
}

static void recency_size_changed(GSize size, void *context) {
  connection_status_component_update_offset((ConnectionStatusComponent*)context, GSize(size.w, 0));
}

GraphElement* graph_element_create(Layer *parent) {
  GraphElement *el = malloc(sizeof(GraphElement));

  GRect bounds = element_get_bounds(parent);

  el->graph_layer = layer_create_with_data(
    GRect(0, 0, bounds.size.w, bounds.size.h),
    sizeof(GraphData)
  );
  ((GraphData*)layer_get_data(el->graph_layer))->color = element_fg(parent);
  layer_set_update_proc(el->graph_layer, graph_update_proc);
  layer_add_child(parent, el->graph_layer);

  uint8_t conn_status_loc = get_prefs()->conn_status_loc;
  uint8_t recency_loc = get_prefs()->recency_loc;

  el->conn_status = NULL;
  int16_t conn_status_y = -1;
  bool conn_status_align_bottom;
  if (conn_status_loc == CONN_STATUS_LOC_GRAPH_TOP_LEFT) {
    conn_status_align_bottom = false;
    conn_status_y = 1;
  } else if (conn_status_loc == CONN_STATUS_LOC_GRAPH_BOTTOM_LEFT) {
    conn_status_align_bottom = true;
    conn_status_y = sgv_graph_height(bounds.size.h) - connection_status_component_size();
  }
  if (conn_status_y != -1) {
    el->conn_status = connection_status_component_create(parent, 0, conn_status_y, conn_status_align_bottom);
  }

  el->recency = NULL;
  int16_t recency_y = -1;
  if (recency_loc == RECENCY_LOC_GRAPH_TOP_LEFT) {
    recency_y = 1;
  } else if (recency_loc == RECENCY_LOC_GRAPH_BOTTOM_LEFT) {
    recency_y = sgv_graph_height(bounds.size.h) - recency_component_height();
  }
  if (recency_y != -1) {
    if (
        (recency_loc == RECENCY_LOC_GRAPH_TOP_LEFT && conn_status_loc == CONN_STATUS_LOC_GRAPH_TOP_LEFT) ||
        (recency_loc == RECENCY_LOC_GRAPH_BOTTOM_LEFT && conn_status_loc == CONN_STATUS_LOC_GRAPH_BOTTOM_LEFT)
    ) {
      // XXX: If the recency component and connection status component share the
      // same corner, the connection status must subscribe to changes in the
      // width of the recency to decide the x position of the icon.
      el->recency = recency_component_create(parent, recency_y, false, recency_size_changed, el->conn_status);
    } else {
      el->recency = recency_component_create(parent, recency_y, false, NULL, NULL);
    }
  }

  return el;
}

void graph_element_destroy(GraphElement *el) {
  layer_destroy(el->graph_layer);
  if (el->conn_status != NULL) {
    connection_status_component_destroy(el->conn_status);
  }
  if (el->recency != NULL) {
    recency_component_destroy(el->recency);
  }
  free(el);
}

void graph_element_update(GraphElement *el, DataMessage *data) {
  graph_element_tick(el);
}

void graph_element_tick(GraphElement *el) {
  layer_mark_dirty(el->graph_layer);
  if (el->conn_status != NULL) {
    connection_status_component_tick(el->conn_status);
  }
  if (el->recency != NULL) {
    recency_component_tick(el->recency);
  }
}

void graph_element_show_request_state(GraphElement *el, RequestState state, AppMessageResult reason) {
  if (el->conn_status != NULL) {
    connection_status_component_show_request_state(el->conn_status, state, reason);
  }
}
