/**
 * Created by french on 7/28/24.
 */
#ifndef INDICATOR_LAYOUT_H__
#define INDICATOR_LAYOUT_H__
#include <gtk/gtk.h>
G_BEGIN_DECLS

#define INDICATOR_TYPE_LAYOUT ( indicator_layout_get_type() )

typedef struct IndicatorLayout
{
   gboolean h_expand; // Expand Horizontally
   gboolean v_expand; // Expand Vertically
   gint     top; // Top Margin
   gint     bottom; // Bottom Margin
   gint     start; // Start (left) Margin
   gint     end; // End (right) Margin
} IndicatorLayout;

GType indicator_layout_get_type(void) G_GNUC_CONST;

// IndicatorLayout *indicator_layout_new(void);

IndicatorLayout *indicator_layout_set(IndicatorLayout *src,
                                      gboolean h_expand, gboolean v_expand, gint top, gint bottom, gint start, gint end);

IndicatorLayout *indicator_layout_copy(IndicatorLayout *src);

void indicator_layout_free(IndicatorLayout *src);

G_END_DECLS
#endif  // INDICATOR_LAYOUT_H__
