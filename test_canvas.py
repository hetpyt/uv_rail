#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tkinter as tk
from tkinter.filedialog import asksaveasfile, askopenfile

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
        
        self.MOD_BUTTON1 = (1 << 8)
        self._last_set = None
        
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
        if col >= self.cols or row >= self.rows:
            return
        self.data[col][row] = val
        self.draw_cell(col, row, (self.active_color if val else self.inactive_color))
        
    def draw_cell(self, col, row, color):
        x = col * self.cell_interval + self.delim_thikness
        y = row * self.cell_interval + self.delim_thikness
        self.create_rectangle(x, y, x + self.cell_size, y + self.cell_size, outline = self.delim_color, fill = color)
    
    def invert_cell(self, col, row):
        self.set_cell(col, row, (0 if self.data[col][row] else 1))
            
    def update(self):
        for c in range(self.cols):
            for r in range(self.rows):
                self.set_cell(c, r, self.data[c][r])
    
    def store_data(self):
        bytes = (self.rows // 8) + (1 if self.rows % 8 else 0)
        result = 'uint8_t data[' + str(self.cols) + '][' + str(bytes) + '] = {\n'
        f = asksaveasfile(mode = 'w')
        if f:
            for c in range(self.cols):
                result += '{'
                sbyte = ''
                for r in range(self.rows):
                    sbyte += str(self.data[c][r])
                    if ((r + 1) % 8) == 0 or r == (self.rows - 1):
                        # last bit in byte
                        result += '0b' + ('0' * (8 - len(sbyte))) + sbyte + ','
                        sbyte = ''
                    
                result = result.rstrip(',') + '},\n'
            result = result.rstrip(',\n') + '};'
            f.write(result)
            f.close()
    
    def load_data(self):
        f = askopenfile()
        if f:
            sdata = f.read()
            print(len(sdata))
            f.close()
            
            col_begin = False
            row_begin = False
            byte_begin = False
            dims_begin = False
            level = 0
            tdata = []
            ccol = -1
            crow = -1
            sbyte = ''
            sdim = ''
            dims = []
            for char in sdata:
                if char == '[':
                    dims_begin = True
                    
                elif char == ']':
                    dims_begin = False
                    dims.append(int(sdim))
                    if len(dims) != 2:
                        raise Exception('Only 2 dimension arrays supported'.format(char))
                    tdata = [[0 for r in range(dims[0])] for c in range(dims[1])]
                    print(dims)
                    
                elif char == '{':
                    level += 1
                    if level == 1:
                        ccol += 1
                    elif level == 2:
                        crow += 1
                    else:
                        raise Exception('Unexpected char "{}". Only 2 dimension arrays supported'.format(char))
                        
                elif char == '}':
                    level -= 1
                    if level == 0:
                        # all done
                        pass
                    elif level == 1:
                        # col done
                        if byte_begin:
                            tdata[ccol][crow] = sbyte
                        crow = -1
                    else:
                        raise Exception('Unexpected char "{}".'.format(char))
                        
                    byte_begin = False
                    
                elif char == ',':
                    if level == 2:
                        # end of byte in row - store
                        if byte_begin:
                            tdata[ccol][crow] = sbyte
                            byte_begin = False

                    elif dims_begin:
                        dims.append(int(sdim))
                        
                elif char == 'b':
                    # new byte begin with next char
                    byte_begin = True
                    sbyte = ''
                        
                elif char.isdigit():
                    if byte_begin:
                        sbyte += char
                    elif dims_begin:
                        sdim += char
                        
            print(tdata)
    
    def on_left_button(self, event):
        #print(event)
        col, row = self.coord_to_cell(event.x, event.y)
        self.invert_cell(col, row)
        self._last_set = self.data[col][row]
    
    def on_mouse_move(self, event):
        #print(event)
        if event.state & self.MOD_BUTTON1 == self.MOD_BUTTON1:
            col, row = self.coord_to_cell(event.x, event.y)
            self.set_cell(col, row, self._last_set)
            

    def on_button_release(self, event):
        #print(event)
        pass
    
    def on_key_pressed(self, event):
        print(event)
        if event.keycode == 67: # c
            self.reset()
            self.update()
            
        elif event.keycode == 85: # u
            self.update()
            
        elif event.keycode == 83: # s
            self.store_data()
            
        elif event.keycode == 76: # l
            self.load_data()
            
            
if __name__ == '__main__':
    root = tk.Tk()
    canvas = MainCanvas(root, 16, 50, 20, 5)
    root.mainloop()