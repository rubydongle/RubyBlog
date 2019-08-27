---
title: Hexo安装使用
date: 2019-08-26 23:00:04
categories:
- 技术文章
tags:
- javascript
- hexo
---
## 1.安装Hexo
```
root@batman:~/work/myblog# hexo init RubyBlog
INFO  Cloning hexo-starter to /mnt/f/workdir/myblog/RubyBlog
Cloning into '/mnt/f/workdir/myblog/RubyBlog'...
remote: Enumerating objects: 4, done.
remote: Counting objects: 100% (4/4), done.
remote: Compressing objects: 100% (4/4), done.
remote: Total 81 (delta 0), reused 1 (delta 0), pack-reused 77
Unpacking objects: 100% (81/81), done.
Checking connectivity... done.
Submodule 'themes/landscape' (https://github.com/hexojs/hexo-theme-landscape.git) registered for path 'themes/landscape'
Cloning into 'themes/landscape'...
remote: Enumerating objects: 1018, done.
remote: Total 1018 (delta 0), reused 0 (delta 0), pack-reused 1018
Receiving objects: 100% (1018/1018), 3.20 MiB | 640.00 KiB/s, done.
Resolving deltas: 100% (555/555), done.
Checking connectivity... done.
Submodule path 'themes/landscape': checked out '73a23c51f8487cfcd7c6deec96ccc7543960d350'
INFO  Install dependencies
Usage: yarn [options]

yarn: error: no such option: --production
WARN  Failed to install dependencies. Please run 'npm install' manually!
```

可以看到安装好的RubyBlog目录结构如下：

```
root@batman:~/work/myblog# tree -L 3 RubyBlog/
RubyBlog/
├── _config.yml
├── package.json
├── scaffolds
│   ├── draft.md
│   ├── page.md
│   └── post.md
├── source
│   └──_posts
│         └── hello-world.md
└── themes
    └── landscape
        ├── _config.yml
        ├── Gruntfile.js
        ├── languages
        ├── layout
        ├── LICENSE
        ├── package.json
        ├── README.md
        ├── scripts
        └── source

9 directories, 11 files
```

- config.yuml是配置文件，用来配置我们博客网站的信息。
- 脚手架目录scaffolds提供了一些用来创建文章的模板。
- source目录存放博客文章。
- themes目录存放网站主题，此时我们看到自动安装了一个名为landscape的主题。

## 2.初始化git仓库
后面我们需要通过使用git来跟踪修改，将RubyBlog目录初始化成一个git仓库，并将里面的文件加入跟踪。
```
root@batman:~/work/myblog/RubyBlog# git log
commit c834cd95e5f11986ff8ab17bfba9fda234dd795a
Author: ruby.dongyu <ruby.dongyu@gmail.com>
Date:   Tue Aug 27 11:18:35 2019 +0800

     init blog
```

## 3.安装依赖模块
上面安装hexo时提示安装依赖失败(Failed to install dependencies. Please run 'npm install' manually!), 这时候我们要进入RubyBlog目录，手动执行"npm install"安装依赖。
```
root@batman:~/work/myblog/RubyBlog# ls
_config.yml  node_modules  package.json  package-lock.json  scaffolds  source  themes
```

## 4.启动Hexo
首先我们通过"hexo g"生成静态网站，然后通过"hexo s"启动网站。
```
root@batman:~/work/myblog/RubyBlog# hexo g                     
INFO  Start processing                                                
INFO  Files loaded in 746 ms                                          
INFO  Generated: index.html                                           
INFO  Generated: archives/index.html                                  
INFO  Generated: fancybox/fancybox_loading@2x.gif                     
INFO  Generated: fancybox/blank.gif                                   
INFO  Generated: fancybox/jquery.fancybox.css                         
INFO  Generated: fancybox/fancybox_loading.gif                        
INFO  Generated: fancybox/fancybox_sprite@2x.png                      
INFO  Generated: fancybox/fancybox_overlay.png                        
INFO  Generated: fancybox/fancybox_sprite.png                         
INFO  Generated: archives/2019/index.html                             
INFO  Generated: archives/2019/08/index.html                          
INFO  Generated: css/fonts/fontawesome-webfont.woff                   
INFO  Generated: js/script.js                                         
INFO  Generated: fancybox/helpers/jquery.fancybox-thumbs.js           
INFO  Generated: fancybox/jquery.fancybox.pack.js                     
INFO  Generated: fancybox/helpers/jquery.fancybox-thumbs.css          
INFO  Generated: css/fonts/fontawesome-webfont.eot                    
INFO  Generated: fancybox/helpers/jquery.fancybox-buttons.css         
INFO  Generated: css/fonts/FontAwesome.otf                            
INFO  Generated: fancybox/helpers/jquery.fancybox-buttons.js          
INFO  Generated: fancybox/helpers/fancybox_buttons.png                
INFO  Generated: fancybox/helpers/jquery.fancybox-media.js            
INFO  Generated: css/style.css                                        
INFO  Generated: css/images/banner.jpg                                
INFO  Generated: css/fonts/fontawesome-webfont.ttf                    
INFO  Generated: css/fonts/fontawesome-webfont.svg                    
INFO  Generated: 2019/08/27/hello-world/index.html                    
INFO  Generated: fancybox/jquery.fancybox.js                          
INFO  28 files generated in 1.24 s                                    
root@batman:~/work/myblog/RubyBlog# hexo s                     
INFO  Start processing                                                
INFO  Hexo is running at http://localhost:4000 . Press Ctrl+C to stop.
```
此时打开"http://localhost:4000"就能看到我们的博客网站运行起来了。  
![img](/images/Hexo安装使用/hexo本地运行.PNG)

## 5.上传到github
在github上创建一个参考，作为存储项目RubyBlog的仓库，然后点击Create repository。  
![img](/images/Hexo安装使用/github创建项目.PNG)  
将本地的创建的RubyBlog目录推送到github上去。  
```
root@batman:~/work/myblog/RubyBlog# git remote add origin https://github.com/RubyDongyu/RubyBlog.git
root@batman:~/work/myblog/RubyBlog# git push -u origin master
Username for 'https://github.com': ruby.dongyu@gmail.com
Password for 'https://ruby.dongyu@gmail.com@github.com':
Counting objects: 113, done.
Delta compression using up to 4 threads.
Compressing objects: 100% (103/103), done.
Writing objects: 100% (113/113), 522.92 KiB | 0 bytes/s, done.
Total 113 (delta 0), reused 0 (delta 0)
To https://github.com/RubyDongyu/RubyBlog.git
 * [new branch]      master -> master
Branch master set up to track remote branch master from origin.
```
此时我们的博客网站成功上传到了github上。
![img](/images/Hexo安装使用/github_push项目.PNG)

## 6.使用github pages展示markdown内容
在github上创建一个存储博客静态网页的仓库rubydong.github.io  
![img](/images/Hexo安装使用/github创建项目2.PNG)  
创建好后，点击Settings按钮  
![img](/images/Hexo安装使用/githubsettings.PNG)  
进入Settings后向下滑动我们可以看到GitHub Pages设置项,设置后我们就看到了我们的网页设置到了 https://rubydongyu.github.io/。  
![img](/images/Hexo安装使用/设置githubpages.PNG)  

## 7.发布Hexo到github pages上
发布Hexo博客到github pages上需要一个插件hexo-deployer-git，通过命令npm install hexo-deployer-heroku --save安装。
安装完成后通过修改配置文件_config.yml来这是我们deploy的位置。
```
# Deployment
## Docs: https://hexo.io/docs/deployment.html
deploy:
  type:
```
修改成如下：
```
# Deployment
## Docs: https://hexo.io/docs/deployment.html
deploy:
  type: git
  repo: https://github.com/rubydongle/rubydongle.github.io
```
下面我们通过hexo g生成静态网页，然后通过hexo d将静态网页传输到github上去。
```
root@batman:~/work/myblog/RubyBlog# hexo d
INFO  Deploying: git
INFO  Clearing .deploy_git folder...
INFO  Copying files from public folder...
INFO  Copying files from extend dirs...
On branch master
nothing to commit, working directory clean
Username for 'https://github.com': ruby.dongle@gmail.com
Password for 'https://ruby.dongle@gmail.com@github.com':
Counting objects: 54, done.
Delta compression using up to 4 threads.
Compressing objects: 100% (40/40), done.
Writing objects: 100% (54/54), 508.28 KiB | 0 bytes/s, done.
Total 54 (delta 5), reused 0 (delta 0)
remote: Resolving deltas: 100% (5/5), done.
To https://github.com/rubydongle/rubydongle.github.io
 + b623f3f...544474c HEAD -> master (forced update)
Branch master set up to track remote branch master from https://github.com/rubydongle/rubydongle.github.io.
INFO  Deploy done: git
```
此时我们的静态网页就能在github pages上正常工作了。  
![img](/images/Hexo安装使用/githubpages完成.PNG)

## 8.插入图片  
Hexo有多种图片插入方式，可以将图片存放在本地引用或者将图片放在CDN上引用。
- 本地引用-绝对路径  
    当Hexo项目中只用到少量图片时，可以将图片统一放在source/images文件夹中，通过markdown语法访问它们。
    ```
    source/images/image.jpg
    1 ![](/images/image.jpg)
    ```
    图片既可以在首页内容中访问到，也可以在文章正文中访问到。
- 本地引用-相对路径  
    图片除了可以放在统一的images文件夹中，还可以放在文章自己的目录中。文章的目录可以通过配置_config.yml来生成。
    ```
    _config.yml
    1 post_asset_folder: true
    ```
    将_config.yml文件中的配置项post_asset_folder设为true后，执行命令$ hexo new post_name，在source/_posts中会生成文章post_name.md和同名文件夹post_name。将图片资源放在post_name中，文章就可以使用相对路径引用图片资源了。
    ```
    _posts/post_name/image.jpg
    1 ![](image.jpg)
    ```
    上述是markdown的引用方式，图片只能在文章中显示，但无法在首页中正常显示。如果希望图片在文章和首页中同时显示，可以使用标签插件语法。
    ```
    _posts/post_name/image.jpg
    1 {% asset_img image.jpg This is an image %}
    ```
- CDN引用  
    除了在本地存储图片，还可以将图片上传到一些免费的CDN服务中。比如Cloudinary提供的图片CDN服务，在Cloudinary中上传图片后，会生成对应的url地址，将地址直接拿来引用即可。
