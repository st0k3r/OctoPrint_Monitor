#pragma once

#include <gui/view.h>
#include "parse.h"

View* status_view_alloc(void);
void  status_view_free(View* view);
void  status_view_update(View* view, const PrinterStatus* status);
