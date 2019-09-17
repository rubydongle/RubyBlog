---
title: pythonQt
categories:
  - 技术文章
date: 2019-09-15 21:59:34
tags:
  - python
---

## 环境
https://blog.csdn.net/weiaitaowang/article/details/52048462
https://github.com/cxinping/PyQt5/
用virtualenv构建一个python3环境。  
```
pip install virtualenv
virtualenv -p pythonpath D:/py3env
```
然后执行Script目录下的activate.bat,激活环境。  

## Windows 安装qtdesigner
pip3.6 install PyQt5-tools

## Ubuntu 18 安装qtdesigner
sudo apt-get install qt5-defalut qttools5-dev-tools

## 使用Qt Designer
blog.csdn.net/qq_33571896/article/details/79507843
编辑好的ui文件通过pyuic5进行转换成py文件。  
pyuic5 test.ui -o test.py -x 生成界面程序  
或者执行pyuic5 test.ui -o test.py然后在生成的py文件末尾添加  
```
if __name__=="__main__":
    import sys
    from PyQt5.QtGui import QIcon
    app = QtWidgets.QApplication(sys.argv)
    widget=QtWidgets.QWidget()
    ui = Ui_Form()
    ui.setupUi(widget)
#    widget.setWindowIcon(QIcon('web.png'))
    widget.show()
    sys.exit(app.exec_())
```
![img](/files/pythonqt/qtdesignerdemo.png)

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


