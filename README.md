# LZW算法及其改进
练习用，可直接对文件操作，使用C++实现的LZW压缩算法，以及对中文文本压缩的改进，改进方案参考陈庆辉的中文文本压缩的LZW算法一文。
## 介绍
* LZW_Compress.cpp为纯原生的LZW压缩算法，完全按照LZW算法的流程实现，没有任何关于词典重构等方面的优化。主要适用于英文等单字节文本，词典初始化为所有ASCII码，码字最大为32位（4B）。
#### 输出示例
##### 控制台输入
![image](https://user-images.githubusercontent.com/47845988/168083392-17fa886b-8407-423d-9d61-0716bc66e056.png)
##### 文件输入
![image](https://user-images.githubusercontent.com/47845988/168082650-6c0ad4f5-091a-45c8-9c0d-1150206a318a.png)

* LZWCH_Compress.cpp为在上一份代码的基础上对中文文本压缩的优化实现，可以对GB2312编码、GBK编码、ASCII编码混合文本实现压缩与解压，词典基本码集为常用GB2312编码，并在基本码集中预留自定义数量的空条目，因此，其对GB2312编码的压缩率最好，对GBK次之。不支持utf-8与Unicode编码。存储在词典中的条目内容为码值+编码的二进制字符串，且字符串中有分割符，因此占用存储空间较大。
#### 输出示例
##### 控制台输入
![控制台输入_压缩](https://user-images.githubusercontent.com/47845988/168085353-051f10e9-5077-4da4-8dd5-bfcabd225c20.png)
##### 文件输入
![三国演义_压缩](https://user-images.githubusercontent.com/47845988/168085425-70a9d13a-8ce0-4092-8bcf-d97bce24d8a2.png)
