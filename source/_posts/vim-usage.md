---
title: vim-usage
categories:
  - 技术文章
date: 2019-11-25 10:17:10
tags:
---

## 快捷键定义
http://haoxiang.org/2011/09/vim-modes-and-mappin/
noremap是不会递归的映射 (大概是no recursion)例如noremap Y y noremap y Y不会出现问题
前缀代表生效范围
inoremap就只在插入(insert)模式下生效
vnoremap只在visual模式下生效
nnoremap就在normal模式下(狂按esc后的模式)生效
这样可以减少快捷键所用到的键位组合的个数一个组合可以有多种用途 就不用费劲心思思考 该映射哪个没被绑定过的键了


## YouCompleteMe
代码补全工具
网站:https://github.com/ycm-core/YouCompleteMe
安装:https://linux.cn/article-6312-1.html
安装:https://vimjc.com/vim-youcompleteme-install.html
https://blog.csdn.net/freeking101/article/details/78539005

1. 编译安装 --all选项指所有
```
vivo@vivo-ThinkCentre-M6600t-N000:~/.vim/plugged/YouCompleteMe$ ./install.sh --clang-completer --clangd-completer --ts-completer --java-completer
```
--java-completer中如果jdt无法下载修改文件:
third_party/ycmd/build.py
```
diff --git a/build.py b/build.py
index 5b31c1f2..2b0bf4fb 100755
--- a/build.py
+++ b/build.py
@@ -961,7 +961,9 @@ def EnableJavaCompleter( switches ):
   REPOSITORY = p.join( TARGET, 'repository' )
   CACHE = p.join( TARGET, 'cache' )
 
-  JDTLS_SERVER_URL_FORMAT = ( 'http://download.eclipse.org/jdtls/snapshots/'
+  JDTLS_SERVER_URL_FORMAT = ( 'https://saimei.ftp.acc.umu.se/mirror/eclipse.org/jdtls/snapshots/'
+  # jdt-language-server-0.45.0-201910031256.tar.gz
+  #JDTLS_SERVER_URL_FORMAT = ( 'http://download.eclipse.org/jdtls/snapshots/'
                               '{jdtls_package_name}' )
   JDTLS_PACKAGE_NAME_FORMAT = ( 'jdt-language-server-{jdtls_milestone}-'
                                 '{jdtls_build_stamp}.tar.gz' )
```

2.配置文件
默认的配置文件在下面的路径
YouCompleteMe/third_party/ycmd/.ycm_extra_conf.py


## tagbar
可取代taglist  
网站: http://majutsushi.github.io/tagbar/
代码: https://github.com/majutsushi/tagbar



## C/C++开发环境搭建
参考:http://www.skywind.me/blog/archives/2084

## Vundle管理插件

## vim-plug插件管理
特性:全异步插件安装
网站:https://github.com/junegunn/vim-plug

使用方法：
### 1.获取
```
curl -fLo ~/.vim/autoload/plug.vim --create-dirs \
    https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim

# 下载后目录内容
ruby@batman-ThinkCentre-M6600t-N000:~$ tree .vim
.vim
└── autoload
    └── plug.vim

1 directory, 1 file

```

### 2.配置
在.vimrc中添加vim-plug配置区
1. 配置区以`call plug#begin()`开始，以`call plug#end()`结束。
2.在begin和end之间用`Plug`命令列出插件。

```
" Specify a directory for plugins
" - For Neovim: stdpath('data') . '/plugged'
" - Avoid using standard Vim directory names like 'plugin'
call plug#begin('~/.vim/plugged')

" Make sure you use single quotes

" Shorthand notation; fetches https://github.com/junegunn/vim-easy-align
Plug 'junegunn/vim-easy-align'

" Any valid git URL is allowed
Plug 'https://github.com/junegunn/vim-github-dashboard.git'

" Multiple Plug commands can be written in a single line using | separators
Plug 'SirVer/ultisnips' | Plug 'honza/vim-snippets'

" On-demand loading
Plug 'scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
Plug 'tpope/vim-fireplace', { 'for': 'clojure' }

" Using a non-master branch
Plug 'rdnetto/YCM-Generator', { 'branch': 'stable' }

" Using a tagged release; wildcard allowed (requires git 1.9.2 or above)
Plug 'fatih/vim-go', { 'tag': '*' }

" Plugin options
Plug 'nsf/gocode', { 'tag': 'v.20150303', 'rtp': 'vim' }

" Plugin outside ~/.vim/plugged with post-update hook
Plug 'junegunn/fzf', { 'dir': '~/.fzf', 'do': './install --all' }

" Unmanaged plugin (manually installed and updated)
Plug '~/my-prototype-plugin'

" Initialize plugin system
call plug#end()
```
### 3.安装插件
通过`:PlugInstall`命令安装插件。

# LeaderfFunction
替换taglist和tagbar

# ctags
exuberant-ctags -> universal-ctags
用后者替换前者
```
snap install universal-ctags
```
