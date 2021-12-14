### Visual Statistics

This tool is used to plot the CMG KPI-KCI counter files. You can download it from [here](http://sdu.int.nokia-sbell.com:4099/VisualStatisticsSetup.exe).

<img src="images/main-window.gif" style="width:50%;padding-right:2px;"/><img src="images/plot-window.gif" style="width:50%;padding-left:2px;"/> 

### Tutorials

**Convert file format**

The XML format KPI-KCI files must be converted to CSV format for performance reason. Click the button <img src="images/tool-button-csv.png"/> in tool bar to convert the selected KPI-KCI files. After the conversion, this tool will ask you whether to open the converted file immediately, or you can open it whenever you want by clicking the <img src="images/tool-button-open.png"/>button.

**Filter counter names**

Once the converted CSV format file is opened, all of the counter names will be displayed in the right pane of the main window. The left pane of the main window displays the modules of the counters. You can select one or more modules to filter the displayed counter names in the right pane.

Additionally, you can type regular expression in the combobox on top right of the main window to filter the displayed counter names more accurately. The "#" character can be used in filter expression as a pipe operator, which has similar function as "|" in Linux shell. For example, the expression `aa#bb` uses `aa` to filter the counter names firstly and then use `bb` to filter against the result of `aa`. The negation symbol is also supported, e.g. `aa#!bb`.

**Plot counters**

Select one or more counter names and click the button <img src="images/tool-button-plot.png"/> in tool bar to plot the selected counters in a single window. If you select more than one counter names and click the button <img src="images/tool-button-plotm.png"/> then each counter will be plotted in a separate window.

**Plot window manipulation**

Use mouse wheel to zoom in/out the plot. While zooming in/out the plot, press `ctrl` key to do the zooming only in *x* axis or `shift` key in *y* axis. The tool button <img src="images/tool-button-restore.png"/> can be used to restore the zoomed plot.

Select the counter names in the legend box to hide the unselected counters. Press `ctrl`/`shift` key to select more than one counter names. Press `alt` key when selecting counter names to rescale the *y* axis for the visible graphs.

Two types of comment can be added in plot window. Right click at blank area and click the `Add Comment` of the context menu to add a comment text. When a counter value is displayed in a text box with a tracer, the `Add Comment` can add a special type of comment which has an arrow point to the position of the displayed value.

The legend box and comment items are draggable and can be dragged at any place in the axis rectangle.

### Lua support

Click the button <img src="images/tool-button-script.png"/> in tool bar of plot window to open the script window. Below is the description of the exposed functions.

```lua
function plot.graph_count()
```

Returns the total number of graphs in a plot window.

```lua
function plot.graph_name(graph)
```

Returns the name of a graph. The first parameter 'graph' is the index of the graph in legend box, the index starts from 0.

```lua
function plot.get_lastkey()
```

Returns the last key of the plot, i.e. the last coordinate of the *x* axis. The first key of a plot is 0, the second key is 1, and so on.

```lua
function plot.get_value(graph, key, default)
```

Returns the value of a graph at 'key'. The first parameter 'graph' is the index of the graph in legend box, the index starts from 0.  If the value of 'key' does not exist, the third optional parameter 'default' will be returned. If the 'default' is not given, a nil will be returned.

```lua
function plot.add_graph(name, data, r, g, b)
```

Adds a new graph in plot window with 'name'. The parameter 'data' is an array, the elements' index is represented as the key of the graph. The optional parameters *r*, *g*, *b* will be used as the color of the graph.

```lua
function plot.update(rescaleY)
```

Refreshes the plot area. It is necessary to call this function after adding one or more new graphs. If the optional parameter 'rescaleY' is set to `true` then the *y* axis will be rescaled if necessary.

```lua
function print(text)
```

Prints out the 'text' to the log pane of the script window.

**Examples**

---

Add a new graph whose values are the sum of the first two graphs.

```lua
local data = {}
for key=0, plot.get_lastkey() do
    data[key] = plot.get_value(0, key, 0) + plot.get_value(1, key, 0)
end
plot.add_graph("graph_sum", data)
plot.update(true)
```

Add a new graph with blue color and its values are the first graph's value divided by 1000.

```lua
local data = {}
for key=0, plot.get_lastkey() do
    data[key] = plot.get_value(0, key, 0)/1000
end
plot.add_graph("divide_1000", data, 0, 0, 255)
plot.update(true)
```

