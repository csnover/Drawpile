/*
 * Copyright (C) 2022 - 2023 askmeaboutloom
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * This code is based on Drawpile, using it under the GNU General Public
 * License, version 3. See 3rdparty/licenses/drawpile/COPYING for details.
 */
#ifndef DPENGINE_FLOOD_FILL_H
#define DPENGINE_FLOOD_FILL_H
#include "pixels.h"
#include <dpcommon/common.h>

typedef struct DP_CanvasState DP_CanvasState;
typedef struct DP_Image DP_Image;

typedef enum DP_FloodFillResult {
    DP_FLOOD_FILL_SUCCESS,
    DP_FLOOD_FILL_OUT_OF_BOUNDS,
    DP_FLOOD_FILL_INVALID_LAYER,
    DP_FLOOD_FILL_SIZE_LIMIT_EXCEEDED,
    DP_FLOOD_FILL_NOTHING_TO_FILL,
} DP_FloodFillResult;

DP_FloodFillResult DP_flood_fill(DP_CanvasState *cs, int x, int y,
                                 DP_UPixelFloat fill_color, double tolerance,
                                 int layer_id, bool sample_merged,
                                 int size_limit, int expand, int feather_radius,
                                 DP_Image **out_img, int *out_x, int *out_y);


#endif
