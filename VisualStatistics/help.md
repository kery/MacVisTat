### Visual Statistics

This tool is used to plot the CMG KPI-KCI counter files. You can download it from [here](http://sdu.int.nokia-sbell.com:4099/VisualStatisticsSetup.exe).

Supported platform: Windows.

### Tutorials

**Convert file format**

The XML format KPI-KCI files must be converted to CSV format for performance reason. Click the button <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB8AAAAeCAYAAADU8sWcAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAEXRFWHRTb2Z0d2FyZQBTbmlwYXN0ZV0Xzt0AAARXSURBVEiJ5ZfNT1xVGMZ/d75ggGGYlulAkaZQ7JekarRWI7gwJDaNqUYXfi2MRjcad41/gNG4MTHGxI+dH124MLpoQmo0Vq2SUGNNgaLYloLAFDpDGWYuMzD3nvO6uHc+6DCls7GL3uTmnpxJ5nee57zvc+41UqmUcIsuz60C395wX7UfpnNnSaxNImgQjaAAjYhaNyeiwX0K4syLBoRYsJfdzYdrgw8tHedSdgjTToLYiFhosRCxEck7Ywpj25230aIQrZwFiqLRF+Vy+DSPdbyzIbzC9uncWQdsJRGxysBWCVwY68LYdhah7SJYRMjk5xm79g0Ty4M3pzyxNolpJxEsEBstFgZCe+MeOpr24zV8ZK1FxhdPYuoFWoPdIMK8ed6x3gWLaLTWZNQ88ZUR9oSPbK7c2U+7CBaxidS1cdfWARZzk0ynh5kzz7GmMogoesL9PH3n++wI3V8BFuBGIVJZ7aIrrK73NmIAU+kzeA0/e7c8ysHYMxyMvcDO5geINvTw7L5PeaTzDRp8EQcs4txakCorqLBdUGVgq1hcIIjY7NsywN3RJyv+qMEfob/zdZoD7Xz79zFHdQFcBb5Bn2u3eq2yqnbay3lWN1LpPInspXVgkerWVyoXtb6qcSrZEA9gcHr2Y8aTJ8nZy1h6hftiz3Gw7Xmmlof54fJ7XF76bR1Ya2qxXZeBLbdlFsjZaR5sfxGl82TtJeLJUcx8kkToAmOJE5ya/oCFlQnAADxu0XFD2ytDRrRjNZajGIPltTjDV74g5N+Gx/CQVznyKouBcD45yNjVE5jWNQzDgxhg4Mh2iq4W21GlAEEhGCCwvDZHanWmGJ3iNtJKPokUfBWP2y2Cdq3XugblItpJK7fQnDnDARY8XJfjzkIKraV1Ceyus6ryDapdudWu0KJQygZRhPxRtjXsps7bhBZFk7+V1voe6jyNBIwmmnxRlK1RSrM1uAsP/mLR1aS8lNUKQ+DxnrcJBzpIZC8QN8dZyv3LQNebxDOjLJgTzKZHGOg+xud/vsyW+u0c3fsWn/3xipsVNe25lJ1OmuZAG20N+zk+9hKp1TiC0NPSj61yzKXP8U/yFxazM+idNtubeumKHGL0yiCr1sqmyjeOVxcsoqj3hcmrLGb+qhs0mqnUGU5NfciuSD8P73iVoK+FieTPdEUeoityiPNXvy8qrinbSyeTc0jEM6Pk7BR9na/R3dJHd0sfLXV34DUCXLz2K3XeEB4jwMXFIe5pP0p6LcFSdn5T1RsrRypOp+8m38VSq+wMH8JWeSxtsTXYRaM/ypnZr8isJkiaU/x46SOGpr4s7fMm78UVex4L9tLoi5LJzxePxXnzL66kx9dF5k+ZT4rtJAI5ZfL7zNclq11wKBCjI3Tg5pTvbj7MvvATNHijFcdieVZfPy6/y8H3bnuK3tbKFwkAo9pHw8TyIPGVEaD0h3LdeKNn+dUROlAVfEP4/3Hdvh8NtxT+H/P5hv1xy764AAAAAElFTkSuQmCC"/> in tool bar to convert the selected KPI-KCI files. After the conversion, this tool will ask you whether to open the converted file immediately, or you can open it manually later. 

**Filter counter names**

Once the converted CSV format file is opened, all of the counter names will be displayed in the right pane of the main window. The left pane of the main window displays the modules of the counters. You can select one or more modules to filter the displayed counter names in the right pane.

Additionally, you can type regular expression in the combobox on top right of the main window to filter the displayed counter names more accurately. The "#" character can be used in filter expression as a pipe operator, which has similar function as "|" in Linux shell. For example, the expression `aa#bb` uses `aa` to filter the counter names firstly and then use `bb` to filter against the result of `aa`. The negation symbol is also supported, e.g. `aa#!bb`.

**Plot counters**

Select one or more counter names and click the button <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB8AAAAeCAYAAADU8sWcAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAEXRFWHRTb2Z0d2FyZQBTbmlwYXN0ZV0Xzt0AAAGMSURBVEiJ7ZZRSwJBFIXPrFZWJlhGVBQSSQ891Ft/on5KP6kf01s+SQ+9RIoICmKoaITQ3nt6GLfUXdNkysAODMPC7Pn23J25jGm328Sc5M0LvNjw+OBDp+fjodb5EdDpXgqpxBAuDL8rNWcGHG+tIJdJIF95wcVhEvnKC64aN7hfPkdn8zIE/ztlD5Td3sBuen0mwzaBk4PVj/k2eY1yo4vctPCzbAZHO6mZ4KMq1TsoP3cBhNtJJDwe80BHrSfuGagyij0KJwgCIDzjBg4DGGPnr+FEP7GBOmu6xnpOSk4AqgpVhag6QQd+UVlC/1xJCOksufWKNhtOTkKVIDH2he+KRN9zit2uqvDFDhfyxZY9SiPJP8vuO0oelD3KLnTURAlf7HAhX6znxCZDAiJ2p7va7aIKEZ2cnCCUCtHga13ArScntlcCInaxq92uVIhM0V7ZX/xYbTqDP9VaEJUpksOeyUKxjkKx7gQeKCqLGbw6994E1darU2ig/fQaEkux8fDf1uJenf/hc9E7AngAODAl4qoAAAAASUVORK5CYII="/> in tool bar to plot the selected counters in a single window. If you select more than one counter names and click the button <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB8AAAAeCAYAAADU8sWcAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAEXRFWHRTb2Z0d2FyZQBTbmlwYXN0ZV0Xzt0AAAJKSURBVEiJ7ZfNThNRGIafmZa2lgK11h8MEqIQjb+s8AIMG6M34EZvwFvxCjTRC3DhxgWu2MEGUWNiorakJgSlKbVU/pzzfS5Op6F0oNOhCST4JieTmTk5z7zfe86ZGadarSpHJPeowCcbHt99Utvy+Lxc69ngNy4OMpiK73u/DT5XqHQFGD+TZCKfYr5U5+5ohvlSnYerL/iQmKSWe3Ag/PiU3dfY2QGGT/eHHqSqcPXSqeZxNvOUpdV1JrqDKzhwZyzP5fODoeF7VfhZY6m8bscLDVcwRoi7DqVynddz37uCZhIumWSMyobHhUwfb94vM7xd5LebZXrqJiPZVEv/lswVUFWMKqLdb3zj+RT3r2XJpePN45M/r5jcWQzs35a5NMBO12j4Vt5iZf0vlQ2Pt1+qVDY8XvY/ts47wVUVYwQRxXUBlOnbo9wazUV4FKtPpRHeffwBAZVsc66qeKLEsDEMpRPIIV49Q+kEigZOvT3OwYhixHYW0WYMUSWqiATH2ApHMSJ4RojhYkQaLTrcH6Ojc9S6NaoggqoiyiGd2yg7Zu47NyKAax+k0aLKiC17uMyNsL3jkUr22QgaLao8v+wB9Ba4g3W/WPjFQDrZdO71wHnQVtsCF1U8IxRWqsRiLulkoidlN6KdnV85N8CzR1MAFMt1ns9+xTN29keVZwQjJnCl7/+m196u86AZt+/HhGLX6Fp9Ez+xKG2tvhlyne+Sg539MwtFZhaKYUweKA2zt/vKphPcuz58aKivXH+y7Zrz/4/lxMH/AZkfdCZZSiEsAAAAAElFTkSuQmCC"/> then each counter will be plotted in a separate window.

**Plot window manipulation**

Use mouse wheel to zoom in/out the plot. While zooming in/out the plot, press `ctrl` key to do the zooming only in *x* axis or `shift` key in *y* axis. The tool button <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB8AAAAeCAYAAADU8sWcAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAEXRFWHRTb2Z0d2FyZQBTbmlwYXN0ZV0Xzt0AAAN9SURBVEiJ1ZfPbxtFFMc/a3vtdVznJ03aQFDTNGmBiqoCKUVFQMsBhHqIEAdQ1XLggISQEP9BL9w4wQWJO2cESCiHEgKKokqAIEVqo6RN0oQkrp3E9tre9e784BDbxXHsmMhN1benmf3u+7x5M29m1kin05pHZIFHBX684EIJMm7i4OG+9LiVnOD35e8OFi6UYDY1yXz2J3zltAweagZ8M/Ejd+2fCQQNtGpdcTQcuS89bicnWbR/xQyZQGur0qhX50IJZpO/sJCbBMNHIpFaIFyTke6XS2FoNBrQhIMxDred4Im2AYKB4P7hUgn+ToyzZE8RMoNILZAIpJZI7aO0fNBGoLRAaIEnXCK0c7ZnjOGu87RHegkY9ZNbA/elx3xqijvZ64TMUMVxGSgq8O1MKLYDqrS1QCpFr3mS0d53Od75Yl14VVhCCeY3prmXmyZsRtC6nNQHDzt7yhqtQG/3KUOy6t9kfPVz7tuLdeFVq11JRT6Xw/c9zGCA/86pRqGUxFV5XFnAlS45bwtHZinKPEXpIHQRoQQaUCWfaTvPh6e/pjt2pDE8bIZ5rv8C/j3BujNFxLJAa/J+hi1vjbS3SaaYIi/SONKtXvvGDs+l9nJxhj8T41wYvIphVItq6jxqtXH22BtMz9ksZSdYF3OsFVZwVQ5PeXvCdrYdsszZNxj1xohFOhrDAaywxUvD71CYdfgjM4Ud2Kw4fFqd40zHpSq91IKEP8uKmGFDLSCCDuhSXgzNauE22WKqOXg5gIvDlwH49v41RCgPwEDHKd5+4eMaveu6rKUX+S3xAxPpr3CCqcq7gszg62LNNw2316jVxusnr2D7m1xPf1EJYNdgLYvBI6c42nWM+N0evk9+RiGYAgOklOy2O+55sFhhi7HTnzDWd404tSu2Rh+xeG3oPV7p+QAzYEEArFCcAGaNds+DpRzAxaHLoGHL+acp/TOd57lhf8MWy3SafURD8f3BoTQFw1fYyK01pe+O9BM146T9AH3RE8TMzv3DYTulT0YGm9IqJBiauNHLs+2vYoWjNZqHdofL+Ou4Osdg6BwjXaO7ah4KXCrJUuEvvILiraOf0nGo+2DgWmtuJae4szHD+099yUj/mbra/zXnzVjeybOSWODNgY8YOvx8Q23dm8x+zfd9PFEkFj20p7blIzdNE9Os3VB2s8fnj6XV9i8gR8p33fEcvAAAAABJRU5ErkJggg=="/> can be used to restore the zoomed plot.

Select the counter names in the legend box to hide the unselected counters. Press `ctrl` key to select more than one counter names. Press `alt` key when selecting counter names to rescale the *y* axis for the visible graphs.

The legend box is draggable and can be dragged at any place in the axis rectangle.

### Lua support

Click the button <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB8AAAAeCAYAAADU8sWcAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAEXRFWHRTb2Z0d2FyZQBTbmlwYXN0ZV0Xzt0AAAVmSURBVEiJ5ZfdaxzXGYefc8587axWq2glubIkS45sx43dotC6pmlcTEtwoJQ25KrQf6H/US9Lb0pvWnrhEEjSUJAxCUob+duKZe16Y1la7UqzOzsz56MXkh2EpI0KBl/0hXMzzHuec37v77xnRrTbbccrCvmqwK8c7h33xUIXJGmHnbRFmm3T7W/jgHJYIY6qVEqjVEpVPM9/ufDmxhorz5bY7H1Np9/AqRzP2xVNa4swAdVoitHSHK9PLHBy7NSx4GKQ4ay1/OfhTW5vfAhBguc7PKUQgEAA4HA4oDAGnYPNylyYeI8fzl9CysFVPRLe6yf869Y/aKQ3GKp4+EoRKA9f+SgpEUIQqJjAH6ZamqKbdahvLdMv+mxvF0yXLvOzN39FuVQ5En6k7Iu3r1NPb1AeUkR+QOQFBJ5PqHzK4SjV+BS1oXlG4lkiv0ovb2Fdn2b7PrZiWdteZPGOxy8WPkAIcXz4Sv0Od1ufMFoLif2QOAgp+RG1eIbp2iUq0RSRX8VT0bcSCkHJD4iDCAe4SsqdjU+Zq19kfub88eDbSZtP7/yVkVGfOIwYjqqcn3yH8cpZhkuzSPE8xWFsD2s1SX+F1Y1/ovUO5TDCOcA59GuWT27/hbGRP1CtjByAH3DEwye3KNQGURAQKo/JkTc4c+IakV/F2hTnLNp0yPUm2iQ4CgJvlFCJXYW8kEB5aOPoZwVbWYOHzeVDd74PnuUZ9dZ9VGjwlEegfCIvRiCQIqDQbbLiCQ5QMibwxgi8GnE4zUj8Op6QJGlKY2OD9c027U6XtEiob94ny7PB8KTXodWtE/qSQCp85ZGkqxjTx1dlouAkUTCNr6ooGSOE3Ku3pBz/mMebfR4/W6eXZRhjkVIglaO59Ygk7QyG57pPL99BSYWUEiUkxvbYSJb2IOpQ+QCGojG+P/1LSmEFKQRSSgQCIQS9fJss7w+GF7pA2wIhxIshhWS7e4tetnYkGEAKxezYAuen3tnL3T0BCMh1TmGKAzn73C5QmEKQ9FKw0BFdpJTcZY1apcnb535PrTJ75AI8FXJp/rc0Wivca3yx63oLwirkIXfYviehHyJtSPNZm+ZGi/VOh3Y3IdOaVtJgee06xuqBCoDkrbn38L0Y5yzWOiKvjK+CwfBSWKYajWO1wzqHtbvJxhi0say1vuLx5uffAYfQL+NJH+ssxjiGw7FD2+w+eFwqMzN6DlcEaGMojKHQBm0MuS7oFzlfrn5Iu/vkwETOWbpZh6/Xl7i+9EdayTO0sbjM51TtDeJS+UDOvporpTh76gI3742j9ROUlBRSo/TuGkvBMKPlGXp5m+H4BHLP/YXJeNC8yYNvPmfl6b/ZSRO0MWhtKJlJzp26iFIHT8qB9vq98ZMszF3h4wd/Qo5/63olFT858z6nJ94izdvkuoevSjQ2b/Pl6kfUW/dI0m0KrcmNpjCGnZbm6vwVJsYmDy3PAbjv+1y9fI1HzbusbS1hqwWOXQ8U2qCUTzdLeNpZ44uV6zS3HuKcQxtLYTTaGPp5QdpxzJZ+xNXL1wiCg2aDAff54/oj/v7Zn2n0v2KoJvGUYqI6zZnJH9DurvPN1gpZkeKcwzqLthaz55Nk03IyvMivf/47ZqdPHwoeCDfG0GjW+Wjxbyw//YzKCUUQKJSUSCletFbcriraGPLMsL2uuTB+hXff/g1Tk9OH1vo74c8jyzJuLi1yY/ljtoo6xClCWV6wLTgjoVfitWCay29e5dLCTwnDcNC0x4M/V6HdabNaX2Fl7R6ddJOsSAEIvRLVco3TU2eZm5lnpDoycLf/M/x5OOcoimJv5AB4nk8QBPi+f+Tn0kuBv+z4//1d+i9WXZQqMK1g3QAAAABJRU5ErkJggg=="/> in tool bar of plot window to open the script window. Below is the description of the exposed functions.

```lua
function plot.get_lastkey()
```

Returns the last key of the plot, i.e. the last coordinate of the *x* axis. The first key of a plot is 0, the second key is 1, and so on.

```lua
function plot.get_value(graph, key, default)
```

Returns the value of a graph at *key*. The first parameter *graph* is the index of the graph in legend box, the index starts from 0.  If the *key* of graph does not exist, the third optional parameter *default* will be returned. If the default is not given, a nil will be returned.

```lua
function plot.get_dt(key)
```

Returns the date time at *key* as the number of seconds that have passed since 1970-01-01T00:00:00.

```lua
function plot.get_dtstr(key)
```

Returns the date time at *key* as a string, e.g. 2020-08-22 20:07:12.

```lua
function plot.add_graph(name, data, r, g, b)
```

Adds a new graph in plot window with *name*. The parameter *data* is an array, the elements' index is represented as the key of the graph. The optional parameters *r*, *g*, *b* will be used as the color of the graph.

```lua
function plot.update(rescale)
```

Refreshes the plot area. It is necessary to call this function after adding one or more new graphs. If the optional parameter *rescale* is set to `true` then the *y* axis will be rescaled if necessary.

```lua
function print(text)
```

Prints out the *text* to the log pane of the script window.

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

