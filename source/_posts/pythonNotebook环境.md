---
title: pythonNotebook环境
categories:
  - 技术文章
date: 2019-09-15 22:10:34
tags:
  - python
---

## 安装
sudo apt-get install jupyter-notebook
或者  
pip3 install jupyter

## 启动
```bash
ruby@batman:~$ jupyter -h
usage: jupyter [-h] [--version] [--config-dir] [--data-dir] [--runtime-dir]
               [--paths] [--json]
               [subcommand]

Jupyter: Interactive Computing

positional arguments:
  subcommand     the subcommand to launch

optional arguments:
  -h, --help     show this help message and exit
  --version      show the jupyter command's version and exit
  --config-dir   show Jupyter config dir
  --data-dir     show Jupyter data dir
  --runtime-dir  show Jupyter runtime dir
  --paths        show all Jupyter paths. Add --json for machine-readable
                 format.
  --json         output paths as machine-readable json

Available subcommands: bundlerextension migrate nbextension notebook
serverextension troubleshoot
```

```bash
ruby@batman:~$ jupyter notebook
[I 22:41:19.450 NotebookApp] Writing notebook server cookie secret to /run/user/1000/jupyter/notebook_cookie_secret
[I 22:41:19.624 NotebookApp] Serving notebooks from local directory: /home/ruby/aosp/android-9.0.0_r47
[I 22:41:19.624 NotebookApp] 0 active kernels
[I 22:41:19.624 NotebookApp] The Jupyter Notebook is running at:
[I 22:41:19.624 NotebookApp] http://localhost:8888/?token=686bdc14fb4d4bd3665fcfd4cc0c68cbc7743c2e3eb3e7ff
[I 22:41:19.624 NotebookApp] Use Control-C to stop this server and shut down all kernels (twice to skip confirmation).
[C 22:41:19.626 NotebookApp] 
    
    Copy/paste this URL into your browser when you connect for the first time,
    to login with a token:
        http://localhost:8888/?token=686bdc14fb4d4bd3665fcfd4cc0c68cbc7743c2e3eb3e7ff
[31935:31935:0915/224120.811235:ERROR:viz_main_impl.cc(167)] Exiting GPU process due to errors during initialization
[I 22:41:20.979 NotebookApp] Accepting one-time-token-authenticated connection from 127.0.0.1
正在现有的浏览器会话中打开。
[W 22:41:21.223 NotebookApp] 404 GET /i18n/zh-CN/LC_MESSAGES/nbjs.json?v=20190915224119 (127.0.0.1) 5.09ms referer=http://localhost:8888/tree
[W 22:41:21.303 NotebookApp] 404 GET /static/components/moment/locale/zh-cn.js?v=20190915224119 (127.0.0.1) 0.82ms referer=http://localhost:8888/tree
```

