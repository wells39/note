# 1.  开源包编译

=======* RTOS config *======

../gdb-8.2/configure --build=x86_64-linux --host=aarch64-euler-linux --target=aarch64-euler-linux --prefix=/usr --exec_prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/libexec --datadir=/usr/share --sysconfdir=/etc --sharedstatedir=/com --localstatedir=/var --libdir=/usr/lib64 --includedir=/usr/include --oldincludedir=/usr/include --infodir=/usr/share/info --mandir=/usr/share/man --disable-silent-rules --disable-dependency-tracking --with-libtool-sysroot=/usr1/RTOS/build/workspace/workspace_arm64le-preempt-4.4/tmp-glibc/work/aarch64-euler-linux/gdb/8.2-rtos207.5.r6/recipe-sysroot --disable-gdbtk --disable-x --disable-werror --with-curses --disable-multilib --disable-sim --without-lzma --without-guile --program-prefix= --with-expat --with-libexpat-prefix=/usr1/RTOS/build/workspace/workspace_arm64le-preempt-4.4/tmp-glibc/work/aarch64-euler-linux/gdb/8.2-rtos207.5.r6/recipe-sysroot --enable-64-bit-bfd --disable-rpath --disable-gas --disable-binutils --disable-ld --disable-gold --disable-gprof --without-babeltrace --without-python --with-system-readline --disable-tui --enable-nls

 

../glibc-2.29/configure --build=x86_64-linux --host=aarch64-euler-linux --target=aarch64-euler-linux --prefix=/usr --exec_prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/libexec --datadir=/usr/share --sysconfdir=/etc --sharedstatedir=/com --localstatedir=/var --libdir=/usr/lib64 --includedir=/usr/include --oldincludedir=/usr/include --infodir=/usr/share/info --mandir=/usr/share/man --disable-silent-rules --disable-dependency-tracking --with-libtool-sysroot=/usr1/RTOS/build/workspace/workspace_arm64le-preempt-4.4/tmp-glibc/work/aarch64-euler-linux/glibc/2.29-rtos207.5.r22/recipe-sysroot --enable-kernel=4.4 --without-cvs --disable-profile --disable-debug --without-gd --enable-clocale=gnu --enable-add-ons=libidn --with-headers=/usr1/RTOS/build/workspace/workspace_arm64le-preempt-4.4/tmp-glibc/work/aarch64-euler-linux/glibc/2.29-rtos207.5.r22/recipe-sysroot/usr/include --without-selinux --enable-obsolete-rpc --enable-obsolete-nsl --enable-tunables --enable-bind-now --enable-stackguard-randomization libc_cv_ssp=yes --disable-mathvec --enable-stack-protector=strong --enable-nscd

 

export PATH=

../configure --prefix=/home/user/test/ppc/tools --target=powerpc-linux (--static )

../../configure --host=powerpc-linux-gnu --enable-64-bit-bfd --without-python --prefix=/home/lab/gdb-8.3.1/build/ppcbe/install

make clean;make CFLAGS+=-Wno-unused-but-set-variable CFLAGS+=-Wno-switch (CFLAGS+=-DDEBUG) CFLAGS+=-g CPPFLAGS+=-g  LDFLAGS+=-static

 

gdb

diff -purN gdb/c-typeprint.c.old gdb/c-typeprint.c

--- gdb/c-typeprint.c.old    2019-11-21 17:43:20.861917000 +0800

+++ gdb/c-typeprint.c  2019-11-21 17:43:34.229917000 +0800

@@ -1199,7 +1199,7 @@ c_type_print_base_struct_union (struct t

 

​     c_print_type_1 (TYPE_FIELD_TYPE (type, i),

​             TYPE_FIELD_NAME (type, i),

\-             stream, newshow, level + 4,

\+             stream, 0, level + 4, //modify to show all union content through ptype

​             language, &local_flags, &local_podata);

 

​     if (!is_static && TYPE_FIELD_PACKED (type, i))

*****

--- gdb/main.c.old   2020-02-04 11:33:53.158172841 +0800

+++ gdb/main.c 2020-02-04 11:33:45.789858833 +0800

@@ -859,6 +859,7 @@ captured_main_1 (struct captured_main_ar

​    }

  }

 

\+    cli_styling = 0;

  save_original_signals_state (quiet);

 

  /* Try to set up an alternate signal stack for SIGSEGV handlers. */

 

apt-get install libexpat1-dev expat

./configure --enable-targets=all --enable-64-bit-bfd --prefix=`pwd`/install --without-python

../configure --host=arm-linux --enable-64-bit-bfd --without-python --prefix=`pwd`/install

make -j32 CFLAGS+='-Wno-unused-but-set-variable -Wno-switch -g' CPPFLAGS+=-g  LDFLAGS+=-static

 

交叉编译：

export PATH=$PATH:/home/lab/qemu_arm_vexpress/host/bin/;CC=arm-linux-gcc ./configure --host=arm-linux --enable-64-bit-bfd --with-expat --with-libexpat-prefix=/home/lab/qemu_arm_vexpress/host/ --with-python --with-python-prefix=/home/lab/qemu_arm_vexpress/host/ --prefix=`pwd`/gdb/install

make CFLAGS+=-Wno-unused-but-set-variable CFLAGS+=-Wno-switch CFLAGS+=-g CPPFLAGS+=-g LDFLAGS+=-static -j32

 

util-linux-2.28

主机编译：

CC=x86_64-target-linux-gnu-gcc ./configure --host=x86_64-target-linux-gnu --without-python

make -j8

交叉编译：

export CC=aarch64-linux-gnu-gcc

./configure --host=aarch64-linux-gnu LDFLAGS=-static --without-python --without-tinfo --without-ncursesw --without-ncurses

 

nmap

 

Zlib

PATH=/home/lab/qemu_arm_vexpress/host/bin:$PATH CC=arm-linux-gcc ./configure --prefix=/home/lab/qemu_arm_vexpress/host/arm-linux-gnueabi/

PATH=/home/lab/qemu_arm_vexpress/host/bin:$PATH make

make install

 

ssh

移植openssh需要三个包：openssh、openssl 和 zlib，地址如下：

zlib官方下载：http://www.zlib.net/

openssl官方下载：http://www.openssl.org/source

openssh官网下载：http://www.openssh.com/portable.html

首先编译zlib成镜像, 供最后编译 openssh 用

$ cd zlib-1.2.11

$ prefix=`pwd`/install CC=/home/user/xvdf/mopensource/buildroot-2011.05/qemu_arm_versatile/host/usr/bin/arm-linux-gcc ./configure

$ vi Makefile

$ make

$ make install

编译 openssl 成镜像, 也是供最后编译 openssh 用

$ cd openssl-1.0.2t/

$ ./Configure --prefix=`pwd`/install os/compiler:/home/user/xvdf/mopensource/buildroot-2011.05/qemu_arm_versatile/host/usr/bin/arm-linux-gcc

$ make

$ make install

其中./Configure第一个字母是大写的, 交叉编译使用os/compiler来指定.

$ cd ../openssh-8.1p1

$ ./configure --host=arm-linux --with-libs --with-zlib=/home/veryarm/ssh/zlib.install --with-ssl-dir=/home/veryarm/ssh/openssl.install --disable-etc-default-login CC=arm-none-linux-gnueabi-gcc AR=arm-none-linux-gnueabi-ar

$ make 

若需要静态编译, Makefile中需要修改 CC=xxxx-gcc -static LD=xxxx-gcc -static LDFLAGS+=-Wl,-Bstatic CFLAGS+=-static CPPFLAGS+=-static 同时LDFLAGS原本含有 -pie -fpic 等动态链接相关选项需要去掉

https://www.veryarm.com/892.html 

 

lsof

./Configure -n linux

make CC="/home/user/xvdf/mopensource/buildroot-2019.02.7/output/host/bin/powerpc64-linux-gcc -static"

 

busybox

make -j12 ARCH=arm CROSS_COMPILE=/home/user/xvdf/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- O=build defconfig

make -j12 ARCH=arm CROSS_COMPILE=/home/user/xvdf/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- O=build menuconfig

```
Busybox Settings` -> `Build Options` -> `Build Busybox as a static binary (no shared libs)
```

make -j12 ARCH=arm CROSS_COMPILE=/home/user/xvdf/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- O=build install

 

 

buildroot:

去掉toolchain对于vendor限制

![image-20200626230727500](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230727500.png)

![image-20200626230719692](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230719692.png)

低版本 :  grep -niIr --exclude-dir=qemu_ppc_mpc85xx_smp --exclude-dir=qemu_arm_versatile with-pkgversion .   ##./toolchain/gcc/gcc-uclibc-4.x.mk:42

https://wiki.tizen.org/OSDev/Bootstrapping

find configs/ | grep qemu

make O=qemu_ppc_virtex qemu_ppc_virtex_ml507_defconfig

make help

make O=qemu_arm_versatile uclibc-menuconfig          |-->  selected General Library Settings->Large File Support

make -j12 O=qemu_ppc_virtex                          |-->  toolchain

./stdio.h:456:1: error: 'gets' undeclared here (not in a function)

 _GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");

![image-20200626230649503](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230649503.png)

grep -nr "_GL_WARN_ON_USE (gets" /home/user/xvdf/mopensource/buildroot-2011.02/output/build/host-m4-1.4.15/lib/

将报错位置直接屏蔽掉即可

![image-20200626230631598](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230631598.png)

![image-20200626230430499](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230430499.png)

 

![image-20200626230420623](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230420623.png)

 

GEN   doc/aclocal-1.12.1

help2man: can't get `--help' info from automake-1.12

Try `--no-discard-stderr' if option outputs to stderr

Makefile:4048: recipe for target 'doc/automake-1.12.1' failed

make[1]: *** [doc/automake-1.12.1] Error 255

// the lower automake 1.15.1 version had this bug

 

configure.ac:89: installing './compile'

automake: warnings are treated as errors

 

build/host-mpc-0.9/configure.ac:30:AM_INIT_AUTOMAKE([1.9 -Wall -Werror])  // remove -Werror cflags

 

https://forums.xilinx.com/t5/AI-Engine-DSP-IP-and-Tools/Vivado-2018-2-on-Ubuntu-17-10-quot-awk-symbol-lookup-error-awk/td-p/872001

https://forums.xilinx.com/t5/Installation-and-Licensing/Vivado-2016-4-on-Ubuntu-16-04-LTS-quot-awk-symbol-lookup-error/td-p/747165

https://www.xilinx.com/support/answers/66998.html

![image-20200626230411597](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230411597.png)

去掉LD_LIBRARY_PATH,  gawk链接到不正确的库文件

![image-20200626230358310](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230358310.png)

https://stackoverflow.com/questions/19166500/im-getting-errors-error-2-while-building-cross-compiler-toolchain-for-arm

https://osmocom.org/issues/1916

Texinfo 必须使用4.13a以下的版本， 否则会报大量txtinfo的错误。 同时gcc必须使用4.9以下的版本，否则交叉编译工具链4.5.3版本会有大量编译出错

Info  texinfo 查看texinfo版本  ---编译texinfo可能遇到的坑

 https://www.linuxquestions.org/questions/linux-from-scratch-13/help-resolving-error-related-texinfo-4-13a-in-cross-lfs-4175458411/

 https://www.phpfans.net/ask/fansa1/8585780132.html

 

![image-20200626230348059](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230348059.png)

参考http://blog.5ibc.net/p/48570.html

查了一下更新，发现其中有一项是perl版本升级到了 v5.22.1，然后查了perl官方文档，发现官网因为一个bug，该版本将defined(@array)去掉了。可以直接使用数组判断非空。

![image-20200626230339488](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230339488.png)

 

\----------------

In file included from ../../../../libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc:190:0:

../../../../libsanitizer/sanitizer_common/sanitizer_internal_defs.h:261:72: error: size of array ‘assertion_failed__1150’ is negative

   typedef char IMPL_PASTE(assertion_failed_##_, line)[2*(int)(pred)-1]

​                                    ^

../../../../libsanitizer/sanitizer_common/sanitizer_internal_defs.h:255:30: note: in expansion of macro ‘IMPL_COMPILER_ASSERT’

 \#define COMPILER_CHECK(pred) IMPL_COMPILER_ASSERT(pred, __LINE__)

​               ^~~~~~~~~~~~~~~~~~~~

../../../../libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.h:1461:3: note: in expansion of macro ‘COMPILER_CHECK’

  COMPILER_CHECK(sizeof(((__sanitizer_##CLASS *) NULL)->MEMBER) == \

  ^~~~~~~~~~~~~~

../../../../libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc:1150:1: note: in expansion of macro ‘CHECK_SIZE_AND_OFFSET’

 CHECK_SIZE_AND_OFFSET(ipc_perm, mode);

 ^~~~~~~~~~~~~~~~~~~~~

Makefile:523: recipe for target 'sanitizer_platform_limits_posix.lo' failed

\----------------

grep -nIr ipc_perm .  //找到__sanitizer_ipc_perm 中mode的偏移有点问题

 

==

## 2. Linux 设置

问题描述：

linux的命令行界面显示的不是路径，而是-bash-4.1#：https://blog.51cto.com/10950710/2301360

出现这个问题的原因是因为没有配置.bash_profile的问题，或者是我们不小心清空或删除了.bash_profile文件。配置文件中PS1="${HOST}:"'${PWD}'" # "中的PS1环境变量就是设置命令行提示符的显示

在suse上使用/etc/bash.bashrc:196 行bash的设置PS1="${_t}${_u}>\w${_p} " 通过readlink /proc/$$/exe 2>/dev/null 查看当前shell使用的是何种shell

 

 

/var/log/messages 显示没权限的原因是请求rca之类的权限表明系统使用了sellinux应用

 

gitbash中使用telnet需要加winpty  winpty telnet 10.61.66.107 1404

git bash 配置：(echo;echo 'export PS1="\[\033]0;$TITLEPREFIX:$PWD\007\]\n\[\033[32m\]\u@\h \[\033[35m\]$MSYSTEM \[\033[33m\]\w\[\033[36m\]`__git_ps1`\[\033[0m\]$ "';echo "export TERM=cygwin") >> ~/.bash_profile

reverse-search-history (C-r)

​    Search backward starting at the current line and moving `up' through the history as necessary. This is an incremental search.

forward-search-history (C-s)

​    Search forward starting at the current line and moving `down' through the history as necessary. This is an incremental search.

ctrl+r / ctrl+s 正向和反向搜索，正向搜索需要 关闭流控 stty -ixon

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 100 --slave /usr/bin/g++ g++ /usr/bin/g++-5

update-alternatives --config gcc

 

win10

1、添加端口转发
netsh interface portproxy add v4tov4 listenport=4000 listenaddress=127.0.0.1 connectport=4000 connectaddress=172.31.217.198
2、删除端口转发
netsh interface portproxy del v4tov4listenport=4000 listenaddress=127.0.0.1
3、查看已存在的端口映射
netsh interface portproxy show v4tov4
可以通过命令 netstat -ano|find 4000 查看端口是否已在监听
telnet 127.0.0.1 4000 测试端口是否连通
4、git-bash下使用telnet交互式命令需要增加winpty前缀
echo "alias docker='winpty docker'" >> ~/.bash_profile

suse12
systemctl stop SuSEfirewall2.service
systemctl disable SuSEfirewall2.service
修改sshd为 root 登录
systemctl enable sshd.service //开机启动sshd
====

![计算机生成了可选文字: 拢 已 经 懵 好 啦 ， 还 疋 修 改 注 册 表 方 式 。  输 入 regedit ， 打 开 注 册 表 聳 辑 器  2 1 夜 冫 欠 展 于 FH KEY CURRENT_LlSERNSOftwareXM lcrosoftÅOfficeÅ15.OÅOneNOteXOptionsXOthero  」 在 右 窗 囗 右 击 新 建 DWORD(32-Üfä(D)" 命 名 力 ： ScreenChppingShortcutKey"c  4 双 击 新 建 值 ， 修 改 为 41 ， 表 示 将 Onenote 的 屏 截 图 的 快 睫 改 为 ： Win + A （ 以 此 类 挂 42 为 B  43 为 C 等 等 ）  5 ． 兀 成 以 后 ， 重 新 后 动 计 算 机 。  / / 我 尝 讠 式 了 上 述 方 法 没 有 用 。  目 前 罪 近 win 周 围 也 就 z 没 有 热 谖 占 用 了  DWORD 阝 2 位 〕 值  數 值 名 称 （ N ， ：  ScreenCllpplngShortcutKey  數 直 据 (V) ：  @十六迸制（H，  O 十 迸 制 （ [ ))  所 以 我 的 热 设 置 力 了 7 键 。  这 里 因 为 是 16 进 制 ， 所 以 改 砹 了 5A ， 因 为 41 对 应 字 母 A ， 所 以 字 母 Z 对 应 的 是 5A 。  俣 存 后 重 后 圭 脑 丨 芩 来 以 为 win + Z 可 以 实 现 截 图 发 现 没 有 用  亻 日 看 到 onenote 陡 小 的 截 屏 快 睫 是 win ÷ ， t + 5 我 一 试 了 还 是 ， 殳 有 用  最 后 ÉWln+Shlft+Z  砹 功 了 。 欣 喜 狂 一 一 ]

![image-20200626230223671](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230223671.png)

====

设置无文件名拓展默认打开方式为nodepad++

![image-20200626231143038](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626231143038.png)

Windows Registry Editor Version 5.00 

[HKEY_CLASSES_ROOT*\Shell\NotePad++]

[HKEY_CLASSES_ROOT*\Shell\NotePad++\Command]

@="\"D:\software\Notepad++\notepad++.exe\"\"%1\""

[HKEY_CLASSES_ROOT.] @="No Extension"

[HKEY_CLASSES_ROOT\No Extension]

[HKEY_CLASSES_ROOT\No Extension\Shell]

[HKEY_CLASSES_ROOT\No Extension\Shell\Open]

[HKEY_CLASSES_ROOT\No Extension\Shell\Open\Command] @="\"D:\software\Notepad++\notepad++.exe\"\"%1\""

https://superuser.com/questions/13653/how-to-set-the-default-program-for-opening-files-without-an-extension-in-windows

=====

https://my.oschina.net/xDreamYY/blog/228814 sourceinsight设置快捷键入自定义内容

=====

suse 配置代理：

export https_proxy=https://xxxxxx:lyy%401991@proxyhk.huawei.com:8080/

export http_proxy=http://xxxxxx:lyy%401991@proxyhk.huawei.com:8080/

export ftp_proxy=ftp://xxxxxxx:lyy%401991@proxyhk.huawei.com:8080/

unset https_proxy http_proxy ftp_proxy

suse 添加/删除源

添加 zypper ar http://******/**.repo aaa (ps:aaa是别名)

删除 zypper rr http://******

 

这里配置源不要配置opensuse12.1的源，opensuse和suse版本有点隔开：

zypper ar http://download.opensuse.org/distribution/leap/15.1/repo/oss/ openSUSE-15.1-main

zypper ar http://download.opensuse.org/distribution/leap/15.1/repo/non-oss/ openSUSE-15.1-nonoss

zypper ar http://download.opensuse.org/update/leap/15.1/oss openSUSE-15.1-update

 

zypper ar http://download.opensuse.org/distribution/leap/42.3/repo/oss/sese/ main

zypper ar http://download.opensuse.org/distribution/leap/42.3/repo/non-oss/suse/ nonoss

zypper ar http://download.opensuse.org/update/leap/42.3/oss update

 

zypper ar [file:///mnt/](file:///\\mnt\) local

 

zypper refresh

zypper update

使用命令 zypper addrepo -f [URL] [Alias] （Alias是自己给这个源随便定义的英文名）添加软件源并开启自动刷新

使用命令 zypper modifyrepo -d [URL或者Alias] 禁用一个软件源，如：

zypper modifyrepo -d Packman

使用命令 zypper removerepo [URL或者Alias] 删除一个软件源：

zypper removerepo http://packman.inode.at/suse/openSUSE_Leap_42.2/

列出配置的软件源，显示详情（优先级、网址等等）：

zypper repos -d

![image-20200626231121110](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626231121110.png)

zypper dup --allow-vendor-change

/etc/zypp/zypp.conf can configurate default behavior

https://www.tecmint.com/zypper-commands-to-manage-suse-linux-package-management/

https://en.opensuse.org/SDB:Vendor_change_update

===================================

vi没有语法高亮解决办法

zypper install vim-data

https://superuser.com/questions/969939/how-to-get-vim-syntax-highlighting-to-work

===============

 

Suse bash修改显示

/etc/bash.bashrc  line 176 \ line 197

![image-20200626230256840](C:\Users\xxxxxxxx\AppData\Roaming\Typora\typora-user-images\image-20200626230256840.png)

 

======

Samba 配置

https://jingyan.baidu.com/article/215817f74c5e1e1edb142341.html

samba没有权限配置

http://www.360doc.com/content/16/1118/00/7991404_607415863.shtml

samba密码不正确

[https://www.linuxquestions.org/questions/slackware-14/can%27t-connect-from-windows-to-samba-4-6-7-on-current-same-config-works-fine-on-sw14-1-samba-4-2-1-x86_64-1-a-4175612921/](https://www.linuxquestions.org/questions/slackware-14/can't-connect-from-windows-to-samba-4-6-7-on-current-same-config-works-fine-on-sw14-1-samba-4-2-1-x86_64-1-a-4175612921/)

======

### 3. kasan

https://github.com/aryabinin/linux/commit/0b54f17e70ff50a902c4af05bb92716eb95acefe

社区patch 

使能kasan在内核中有几个非常重要的工作：

\1. 定义好shadow_offset的偏移和分配对应shadow 物理page，例如内核使用虚拟空间占用1G，需要1/8，需要125M的，shadow内存。

\2. 找到kasan_init初始化shadow page及其对应的global 变量的红区。

\2. 在各个对应的内存操作上函数增加kasan检测。

 

从Google Group的信息

https://groups.google.com/forum/#!msg/kasan-dev/JKYk2uUBriY/J6voOi7iAgAJ

上述patch的主要缺点是：

\1. I didn't set set any cache policy (L_PTE_MT_*) on shadow mapping (see set_pte_at() calls ) 

which means it's L_PTE_MT_UNCACHED 

 

https://lwn.net/Articles/736018/

https://lkml.org/lkml/2019/7/18/89

#### 4. qemu

  Qumu 编译

../configure --prefix=`pwd`/install --target-list=arm-softmmu,aarch64-softmmu,ppc-softmmu,ppc64-softmmu,x86_64-softmmu,riscv32-softmmu --audio-drv-list= --enable-fdt --enable-kvm --enable-debug --enable-debug-info --enable-debug-tcg --enable-hvf --enable-hax --enable-system --enable-user --enable-membarrier --enable-numa  // if not target-list option will build all target

1. 下载最新qemu-4.1.0-rc2 源码
2. 安装必要依赖包
3. mkdir     qemu-4.1.0-rc2_build; 

cd qemu-4.1.0-rc2_build; mkdir install

../qemu-4.1.0-rc2/configure --help

 ../qemu-4.1.0-rc2/configure --prefix=`pwd`/install --target-list='arm-softmmu,aarch64-softmmu,ppc-softmmu,ppc64-softmmu,aarch64-linux-user,arm-linux-user,ppc-linux-user,ppc64-linux-user' --audio-drv-list= --enable-fdt --enable-kvm --enable-debug --enable-vnc --enable-sdl

make -j32 && make install

(https://community.arm.com/developer/tools-software/oss-platforms/w/docs/415/spawn-a-linux-virtual-machine-on-arm-using-qemu-kvm) 

4.

 

( [drivers/mtd/devices/block2mtd.c] block2mtd\mtdram用于将普通文件\内存 模拟norflash  **## [drivers/mtd/nand/nandsim.c]nandsim用于将普通文件和内存模拟nandflash )

insmod block2mtd.ko block2mtd=/dev/loop0,2048   #simulate norflash

insmod nandsim.ko first_id_byte=0x20 second_id_byte=0xa2 third_id_byte=0x00 fourth_id_byte=0x15  #64MiB, 2048 bytes page

 

验证 

1. unset     QEMU_AUDIO_DRV (当QEMU正在运行时候，可以通过先按Ctrl+A，在按X退出 使用Ctrl+A、Z退出仿真器)
2. qemu原生日志，[/home/user/xvdf/labrary/qemu-4.1.0-rc2/util/log.c]+241 显示所有支持的log，通常 -d in_asm 记录输入汇编target代码 -d ?(-d help)显示help日志信息 all表示所有种类日志 -d trace:help to get a list of trace events. -D记录到文件否则为stdout
3.  

 

https://stackoverflow.com/questions/54251855/virtualbox-enable-nested-vtx-amd-v-greyed-out in order to use kvm in virtualbox, cmd open it -- VBoxManage modifyvm vm-name --nested-hw-virt on

egrep '(vmx|svm)' /proc/cpuinfo  ## 当然光CPU支持VT虚拟化还不够的，还需要在bios里面将VT-X或VT-D选项打开才能真正使用。

lsmod | grep kvm ##查看是否安装KVM驱动

 

Virtual Development Board

https://www.elinux.org/Virtual_Development_Board

 

qemu网络

https://zhuanlan.zhihu.com/p/41258581

https://www.cnblogs.com/hugetong/p/8808752.html

https://github.com/smilejay/kvm-book/blob/master/scripts/qemu-ifup-NAT

https://wiki.qemu.org/Documentation/ISAManuals qemu all architech Instruction Set Manuals

 

 

使用tap的方式建立vm， 建立一个网桥virbr0， 把这个网桥作为网关192.168.122.1， 把tap绑定到这个网桥virbr0上， tap设备是通到vm里面的eth0的， 所以建立的所有vm都在网桥上就可以互相访问， 在主机上使用dnsmasq建立dhcp服务，绑定到virbr0上， 这样所有绑定到网桥的vm即可使用dhcp服务， 在vm中使用dhclient就可以获取到主机dhcp服务提供的ip地址， 在主机上设置nat的iptables， 然后打开系统的转发功能。

[https://wiki.archlinux.org/index.php/QEMU_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)](https://wiki.archlinux.org/index.php/QEMU_(简体中文))

1. qemu-system-arm -M     vexpress-a9 -m 512M -smp 4 -kernel     arch/arm/boot/zImage -dtb arch/arm/boot/dts/vexpress-v2p-ca9.dtb     -nographic -append "root=/dev/mmcblk0 console=ttyAMA0 rw     init=/linuxrc" -sd /home/user/xvdf/labrary/vexpress/rootfs.ext3 -vnc     0.0.0.0:1 -nic tap,ifname=tap0,mac=52:54:98:76:54:32,script=no,downscript=no
2. 建立虚拟机后， 主机 ip link show 发现多了个tap0设备 建立一个网桥， 把tap0绑定到桥上。
3. 安装bridge-utils工具, zypper in     bridge-utils bridge-utils-devel

brctl addbr virbr0                                                  #建立网桥 

ip addr add 172.20.0.1/16 dev virbr0

ip link set virbr0 up

dnsmasq --interface=virbr0 --bind-interfaces --dhcp-range=172.20.0.2,172.20.0.254

ip link set tap0 up     #put to qemu-ifup.sh     

brctl addif virbr0 tap0                                               #把tap绑定到网桥 

iptables -t nat -A POSTROUTING -s "172.20.0.0/255.255.255.0" ! -d "172.20.0.0/255.255.255.0" -j MASQUERADE         #设置nat的iptables 

echo 1 >/proc/sys/net/ipv4/ip_forward                 #设置linux内核的转发 

\#ifconfig eth0 promisc 

1. 安转dnsmasq工具, zypper in dnsmasq, 主机上启动dnsmasq服务，提供dhcp的server功能, 注意参数指向刚建的virbr0桥上  #关键点是 --interface=virbr0 --dhcp-range 设置网段范围 
2. dnsmasq     --strict-order --except-interface=lo --interface=virbr0     --listen-address=192.168.122.1 --bind-interfaces --dhcp-range=172.20.0.2,172.20.0.254     --conf-file=""      --pid-file=/var/run/qemu-dhcp-virbr0.pid      --dhcp-leasefile=/var/run/qemu-dhcp-virbr0.leases     --dhcp-no-override

======================================================  ARM 启动命令 ===================================================================

 /etc/init.d/boot.apparmor stop               #若发现dnsmasq权限不够是可能是security linux工具限制，比如这里是apparmor，可以通过 cat /proc/kallsyms | grep path_mknod找出有哪些sellinux app

unset QEMU_AUDIO_DRV

qemu-system-arm -M vexpress-a9 -m 512M -smp 4 -kernel arch/arm/boot/zImage -dtb arch/arm/boot/dts/vexpress-v2p-ca9.dtb -nographic -append "root=/dev/mmcblk0 console=ttyAMA0 rw init=/linuxrc" -sd /home/user/xvdf/labrary/vexpress/rootfs.ext3 -vnc 0.0.0.0:1 -nic tap,ifname=tap1,mac=52:54:98:76:54:32,script=/home/user/xvdf/labrary/vexpress/qemu-ifup-nat.sh,downscript=/home/user/xvdf/labrary/vexpress/qemu-ifdown-nat.sh -s  # -s 启动gdbsever在 tcp:1234 ip地址省略 -d in_asm -D log -serial [file:/home/log](file:///\\home\log) 重定向串口

======================================================  ARM64 启动命令 ===================================================================

qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 2 -m 512 -kernel Image --append "console=ttyAMA0 root=/dev/vda" -serial stdio -drive file=rootfs.img,if=none,id=blk -device virtio-blk-device,drive=blk -nic tap,ifname=tap1,mac=52:54:98:76:54:32,script=/home/user/xvdf/labrary/vexpress/qemu-ifup-nat.sh,downscript=/home/user/xvdf/labrary/vexpress/qemu-ifdown-nat.sh -s

 

https://blog.csdn.net/caspiansea/article/details/23022583 QEMU + KGDB调试内核模块

 

Linux 2.6.34 内核:

qemu-system-arm -M versatilepb -m 128M -initrd rootfs.ext2 -kernel zImage -append "root=/dev/ram rdinit=/sbin/init console=ttyAMA0 user_debug=31 ip=dhcp mtdparts=armflash.0:64M@0x0(ext2fs)" -nographic -pflash block.img  //通过-pflash可以初始化flash设备 -hda初始化IDE设备 -mtdblock初始化块设备

https://blog.csdn.net/tycoon1988/article/details/46532283 https://www.raspberrypi.org/forums/viewtopic.php?&t=45118 https://balau82.wordpress.com/2010/03/27/busybox-for-arm-on-qemu/

 

qemu内存管理

https://www.binss.me/blog/qemu-note-of-memory/

 

study on qemu:

源代码实现机制在函数 mux_proc_byte

C-a h  print this help

C-a x  exit emulator

C-a s  save disk data back to file (if -snapshot)

C-a t  toggle console timestamps

C-a b  send break (magic sysrq)

C-a c  switch between console and monitor

C-a C-a sends C-a

 

Qemu: trace MMU operation :https://stackoverflow.com/questions/31187709/qemu-trace-mmu-operation

target/ppc/mmu_helper.c: get_physical_address 获取物理地址的函数

exec.c:94:CPUTailQ cpus 定义CPU状态变量

cpus.tqh_first->node.tqe_next 通过cpus获取next cpu状态

p cpus.tqh_first->env_ptr 对于PPC而言获取CPUPPCState *指针

p ((PowerPCCPU *)cpus.tqh_first)->env

p ((CPUPPCState*)cpus.tqh_first->env_ptr)->spr[0x3F7]  //target/ppc/cpu.h:1936:#define SPR_MMUCFG      (0x3F7)

 

 

 

 

==



