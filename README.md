# filehelp
递归文件夹，转换文件编码，统一文件路径大小写
 #### 使用方法
 
 ```shell
用法: filehelp [选项] ...
选项:
  -d, --dir     需要处理的目录路径 (string [=])
  -n, --name    匹配文件名的通配符表达式 (string [=])
  -f, --from    源编码格式 (string [=])
  -t, --to      目标编码格式 (string [=])
  -b, --bom     UTF-8 with BOM (bool [=0])
  -u, --up      文件路径转大写 (bool [=0])
  -?, --help    打印这条信息
```

 #### 当前文件gbk转utf-8 with bom 并且路径转小写的示例
 
```
[root@vm test]# tree
.
├── A
│   └── A
└── B

1 directory, 2 files
[root@vm test]# filehelp -f gbk -t utf-8 -b 1 -u 0
"/home/test/A/A" -> "/home/test/A/a"
"/home/test/A" -> "/home/test/a"
"/home/test/B" -> "/home/test/b"
正在转码:"/home/test/b" GBK->UTF-8 with BOM 完成
正在转码:"/home/test/a/a" GBK->UTF-8 with BOM 完成
全部完成
[root@vm test]# tree
.
├── a
│   └── a
└── b

1 directory, 2 files
```

 #### g++ 生成命令
 
 `g++ src/filehelp.cpp src/handle.cpp -std=c++2a -liconv -o filehelp`
 
 
