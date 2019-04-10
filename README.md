# fdwriter
fat12格式的软盘写入工具

使用方法：

输入格式：  writetoimage  xxx.img xxxfile  

作用： 将xxxfile文件加入软盘xxx.img 

注意：
      
      writetoimage是生成的可执行文件，名字不唯一，可以自己修改
      暂时只支持空软盘写入一个文件
      空软盘使用前应该要格式化（即是写入引导扇区),这里提供了dos操作系统（freeos),建议使用bochs虚拟机
