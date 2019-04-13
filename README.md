# fdwriter
fat12格式的软盘写入工具

使用方法：

这是命令行程序，exe可执行文件可改为其他任意名称，如writetoimage

输入格式：  writetoimage  xxx.img xxxfile  

作用： 将xxxfile文件加入软盘xxx.img 

注意：
      
      writetoimage是生成的可执行文件，名字不唯一，可以自己修改
      暂时只支持空软盘写入一个文件，要删除就用dos操作系统
	  bochs虚拟机的bximage命令可以生成一个空软盘
      这里提供了dos操作系统（freeos),建议使用bochs虚拟机
