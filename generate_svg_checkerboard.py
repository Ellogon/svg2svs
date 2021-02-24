#!/usr/bin/env python3
# Copyright 2021 Ellogon BV.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Generate a fancy checkerboard.

This script provides a simple way to output a customizable checkerboard
useful for debugging purposes in image processing or 3D graphics.
"""

import argparse
import math
import itertools
from typing import Tuple

import cairo


_BLACK = (0.0, 0.0, 0.0, 1.0)
_WHITE = (1.0, 1.0, 1.0, 1.0)


def draw_square_grid(ctx: cairo.Context, x: float, y: float,
                     width: float, height: float, divisions: int,
                     linewidth: float = 0.01,
                     rgba: Tuple[float, float, float, float] = _BLACK):
    """Draw a grid on a cairo context."""
    # Generate 2d grid
    # Horizontal lines
    line_step = 1 / divisions
    ctx.set_line_width(linewidth)
    ctx.set_source_rgba(*rgba)

    for i in range(divisions + 1):
        y_pos = i * line_step
        ctx.move_to(0, y_pos)
        ctx.line_to(1, y_pos)
        ctx.stroke()

    # Vertical lines
    for i in range(divisions + 1):
        x_pos = i * line_step
        ctx.move_to(x_pos, 0)
        ctx.line_to(x_pos, 1)
        ctx.stroke()


def draw_text_centered(ctx: cairo.Context, x: float, y: float, text: str, font_size: float = 0.01):
    """Draw centered text in the specified position."""
    ctx.select_font_face(
        'Sans', cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)

    ctx.set_source_rgba(*_BLACK)
    ctx.set_font_size(font_size)
    x_bearing, y_bearing, width, height, x_advance, y_advance = ctx.text_extents(text)
    x = x - (width / 2 + x_bearing)
    y = y - (height / 2 + y_bearing)

    ctx.move_to(x, y)
    ctx.show_text(text)


def positive_integer(value):
    """Validate a >=0 integer argument."""
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("'%s' is an invalid number of subdivisions." % value)
    return ivalue


def main():
    """Generate svg checkerboard."""
    parser = argparse.ArgumentParser(description='Generate a fancy checkerboard.')
    parser.add_argument('--divisions', default=10, type=positive_integer, help='Main-grid divisions.')
    parser.add_argument('--subdivisions', default=10, type=positive_integer, help='Sub-grid divisions.')
    parser.add_argument('output_svg_filename', type=str, help='Destination output')
    args = parser.parse_args()

    divisions = args.divisions
    subdivisions = args.subdivisions

    SIDE_LENGTH = 16000
    DIVISIONS_LINE_WIDTH = 16
    subdivisions_line_width = DIVISIONS_LINE_WIDTH // 4
    size = (SIDE_LENGTH, SIDE_LENGTH)

    with cairo.SVGSurface(args.output_svg_filename, *size) as surface:
        surface.set_document_unit(cairo.SVGUnit.USER)
        ctx = cairo.Context(surface)
        # Normalizing the canvas
        ctx.scale(*size)

        # Generate gradient background
        pat = cairo.LinearGradient(0.0, 0.0, 1.0, 1.0)

        pat.add_color_stop_rgba(0.0, 0.059, 0.294, 0.643, 1.0)
        pat.add_color_stop_rgba(0.25, 0.0, 0.502, 0.831, 1.0)
        pat.add_color_stop_rgba(0.5, 0.0, 0.678, 0.769, 1.0)
        pat.add_color_stop_rgba(0.75, 0.0, 0.827, 0.498, 1.0)
        pat.add_color_stop_rgba(1.0, 0.659, 0.922, 0.071, 1.0)

        ctx.rectangle(0, 0, 1, 1)
        ctx.set_source(pat)
        ctx.fill()

        # Sub grid
        draw_square_grid(
            ctx, 0, 0, 1, 1, divisions * subdivisions,
            subdivisions_line_width / SIDE_LENGTH, (0.3, 0.3, 0.3, 1.0))

        # Main grid
        draw_square_grid(
            ctx, 0, 0, 1, 1, divisions, DIVISIONS_LINE_WIDTH / SIDE_LENGTH)

        # Draw text
        box_size = 1 / divisions
        indexes = itertools.product(range(divisions), range(divisions))
        for i, j in indexes:
            x_center = i * box_size + box_size / 2
            y_center = j * box_size + box_size / 2
            draw_text_centered(ctx, x_center, y_center, f'{i},{j}')


if __name__ == '__main__':
    main()
