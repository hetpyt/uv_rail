#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tkinter as tk

class MainCanvas(tk.Canvas):
    def __init__(self, parent, rows, cols, cell_s, delim_t):
        self.delim_color = "gray"
        self.active_color = "purple"
        self.inactive_color = "white"
        self.rows = rows
        self.cols = cols
        self.cell_size = cell_s
        self.delim_thikness = delim_t
        self.cell_interval = cell_s + delim_t
        self.canvas_width = cols * cell_s + (cols + 1) * delim_t
        self.canvas_height = rows * cell_s + (rows + 1) * delim_t

        #self.data = [[0 for r in range(rows)] for c in range(cols)]
        self.reset()
        
        super().__init__(parent, width = self.canvas_width, height = self.canvas_height)
        self.pack()

        # draw grid
        # bakcground
        self.create_rectangle(0, 0, self.canvas_width, self.canvas_height, fill = self.inactive_color)
        # hor "lines"
        for i in range(0, self.rows + 1):
            self.create_rectangle(0, i * self.cell_interval, self.canvas_width, i * self.cell_interval + self.delim_thikness, outline = self.delim_color, fill = self.delim_color)
        # vert "lines"
        for i in range(0, cols + 1):
            self.create_rectangle(i * self.cell_interval, 0, i * self.cell_interval + self.delim_thikness, self.canvas_height, outline = self.delim_color, fill = self.delim_color)

        self.bind("<Button-1>", self.on_left_button)
        self.bind("<Key>", self.on_key_pressed)
        self.bind("<Motion>", self.on_mouse_move)
        self.bind("<ButtonRelease>", self.on_button_release)
        self.focus_set()
    
    def reset(self):
        self.data = [[0 for r in range(self.rows)] for c in range(self.cols)]
    
    def coord_to_cell(self, x, y):
        col = (x - self.delim_thikness) // self.cell_interval
        row = (y - self.delim_thikness) // self.cell_interval
        return col, row
        
    def set_cell(self, col, row, val):
        self.data[col][row] = val
        self.draw_cell(col, row, (self.active_color if val else self.inactive_color))
        
    def draw_cell(self, col, row, color):
        x = col * self.cell_interval + self.delim_thikness
        y = row * self.cell_interval + self.delim_thikness
        self.create_rectangle(x, y, x + self.cell_size, y + self.cell_size, outline = color, fill = color)
    
    def invert_cell(self, col, row):
        self.set_cell(col, row, not self.data[col][row])
            
    def update(self):
        for c in range(self.cols):
            for r in range(self.rows):
                self.set_cell(c, r, self.data[c][r])
                
    def on_left_button(self, event):
        col, row = self.coord_to_cell(event.x, event.y)
        self.invert_cell(col, row)
    
    def on_mouse_move(self, event):
        #print(event)
        pass
    
    def on_button_release(self, event):
        print(event)
    
    def on_key_pressed(self, event):
        print(event)
        if event.keycode == 67: # c
            self.reset()
            self.update()
            
        elif event.keycode == 85: # u
            self.update()
    
if __name__ == '__main__':
    root = tk.Tk()
    canvas = MainCanvas(root, 16, 50, 20, 5)
    root.mainloop()