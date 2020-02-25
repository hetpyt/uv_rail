#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tkinter as tk
class MainWindow(tk.Tk):
    def __init__(self, rows, cols, cell_s, delim_t):
        super(tk.Tk, self).__init__()
        self.delim_color = "gray"
        self.active_color = "purple"
        self.inactive_color = "white"
        self.rows = rows
        self.cols = cols
        self.cell_size = cell_s
        self.delim_thikness = delim_t
        self.canvas_width = cols * cell_s + (cols + 1) * delim_t
        self.canvas_height = rows * cell_s + (rows + 1) * delim_t

        #root = tk.Tk()
        self.canvas = self.Canvas(self, width = 600, height = 400)
        self.canvas.pack()

        # draw grid
        # bakcground
        self.canvas.create_rectangle(0, 0, self.canvas_width, self.canvas_height, fill = self.inactive_color)
        # hor "lines"
        for i in range(0, self.rows + 1):
            self.canvas.create_rectangle(0, i * (self.cell_size + self.delim_thikness), self.canvas_width, i * (self.cell_size + self.delim_thikness) + self.delim_thikness, outline = self.delim_color, fill = self.delim_color)
        # vert "lines"
        for i in range(0, cols + 1):
            self.canvas.create_rectangle(i * (self.cell_size + self.delim_thikness), 0, i * (self.cell_size + self.delim_thikness) + self.delim_thikness, self.canvas_height, outline = self.delim_color, fill = self.delim_color)

    # def draw_square(x, y, size):
        # canvas.create_rectangle(x - size/2, y - size/2, x + size/2, y + size/2, fill = 'purple')

        
    # def print_col(x_pos, y_pos):
        # size = 10
        # for i in range(0, 16):
            # draw_square(x_pos, y_pos + i * 15, size)

root = MainWindow(16, 50, 20, 5)
root.mainloop()