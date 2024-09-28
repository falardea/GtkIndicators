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

IndicatorLayout *indicator_layout_set(IndicatorLayout *src, gboolean h_ex, gboolean v_ex, gint t, gint b, gint s, gint e)
{
   g_return_val_if_fail(src != NULL, NULL);
   src->h_expand = h_ex;
   src->v_expand = v_ex;
   src->top = t;
   src->bottom = b;
   src->start = s;
   src->end = e;
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

