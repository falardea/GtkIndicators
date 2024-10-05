/**
 * Created by french on 7/28/24.
 */
#include <glib-object.h>
#include "indicator_layout.h"

G_DEFINE_BOXED_TYPE(IndicatorLayout, indicator_layout, indicator_layout_copy, indicator_layout_free);

// IndicatorLayout *indicator_layout_new(void)
// {
//
// }

IndicatorLayout *indicator_layout_set(IndicatorLayout *src, gboolean h_expand, gboolean v_expand, gint top, gint bottom, gint start, gint end)
{
   g_return_val_if_fail(src != NULL, NULL);
   src->h_expand = h_expand;
   src->v_expand = v_expand;
   src->top = top;
   src->bottom = bottom;
   src->start = start;
   src->end = end;
   return src;
}

IndicatorLayout *indicator_layout_copy(IndicatorLayout *src)
{
   IndicatorLayout *dest = g_new(IndicatorLayout, 1);
   *dest = *src;
   return dest;
}

void indicator_layout_free(IndicatorLayout *src)
{
   g_free(src);
}

