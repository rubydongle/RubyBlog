---
title: pythonQt
categories:
  - 技术文章
date: 2019-09-15 21:59:34
tags:
  - python
---

## hello world
```
from PyQt5.QtWidgets import QWidget, QApplication, QLabel
import sys

class MyWindow(QWidget):
    def __init__(self):
        super().__init__()

def show_mywindow():
    app = QApplication(sys.argv)
    mywindows = MyWindow()
    QLabel(mywindows).setText("<p style='color: red; margin-left: 20px'><b>hell world</b></p>")
    mywindows.show()
    # app.exec_()
    sys.exit(app.exec_())

show_mywindow()
```
## 参考  
https://pypi.org/project/PyQt5/
https://www.riverbankcomputing.com/software/pyqt/intro
https://www.kancloud.cn/digest/py-qt/119452


