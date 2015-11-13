Neko SQL
===============
This is the course project of *Database System* by MtMoon & magicwish
----------------------------------


###1. Database Structure ###

>根目录下，每个database为一个子目录，database名即目录名

>在每个database目录下，每个文件为一个表

###2. Table Structure ###

>每个表文件的第0页为表元数据页,元数据页存储表相关信息，如下:

+ Byte 0~1: fixed-length column number FN
+ Byte 2~3: variable-length column number VN
+ FN*24个Byte，定长列名
+ VF*24Byte 变长列名
+ FN*4Byte 定长数据长度
+ VN*4Byte 变长数据长度
+ FN+VN个Byte 表示各个字段键类型 0为不是key，1为一般key，2为primary key
+ FN+VN个Byte 各个字段的类型  目前：0 int 1 char 2 varchar，其余非法
+ ceil((FN+VN)/8) Byte bull位图

>每个列名48Byte, 即24个char，不足时补0(null)，**建表时需对列名长度进行检查**

###3. Page Structure ###

>页结构类似SQR Server 2000结构，即每个数据页前96个Byte作为页头，尾部为列偏移数组

>目前使用的页头信息如下:

+ Byte 0~1: 该页剩余可用字节数
+ Byte 2~3: 该页槽数 **包括被回收(值为-1)的槽**

###4. Record Line Structure ###

>数据行结构类似SQL Server 2000，顺序及具体内容具体如下：

+ 1 Byte: Tag 各bit信息含义同SQL Server 2000的Tag A
+ 2 Byte: 该行数据总长度
+ 2 Byte: 行定长数据总长度(同SQL Server 2000)
+ n Byte: 定长数据
+ 2 Byte: 定长列数
+ ceil(总列数/8): NULL位图
+ 2 Byte: 变长列数
+ 2*变长列数: 列偏移数组 **注意，列偏移数组中存储的是每个变长数据列结束位置相对数据行首地址的偏移量**
+ m Byte: 变长列数据 

