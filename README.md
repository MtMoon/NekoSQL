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

>每个列名24Byte, 即24个char，不足时补0(null)，**建表时需对列名长度进行检查**

###3. Page Structure ###

>页结构类似SQR Server 2000结构，即每个数据页前96个Byte作为页头，尾部为列偏移数组

>目前使用的页头信息如下:

+ Byte 0~1: 该页剩余可用字节数
+ Byte 2~3: 该页槽数 **包括被回收(值为-1)的槽**
+ Byte 4: 页类型，0 非叶级索引页，1叶级索引页(非簇集索引叶级页), 2 数据页
+ Byte 7~10:B+数上页指针，若不建所以则忽略此项

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

###5. Index Structure ###

>每个索引文件的第0页为元文件页，储存相关信息，如下：

+ 24Byte 索引所属的表名
+ 24Byte 索引所在的字段名
+ 24Byte 索引名
+ 1Byte 建索引字段的类型，0int，1 char
+ 1Byte Tag 第0bit 表示建索引的字段是否是定长，0为定长，1为变长， 第1bit表示该字段见表是是否允许为NULL 0为不允许，1为允许
+ 1Byte 索引类型 0 非簇集不唯一，1非簇集唯一，2簇集索引(默认唯一)
+ 2Byte 定长码值长度，如索引码为变长字段，则此值为0



###6. Index Page Structure ###

>簇集索引的叶级页即数据页，此外的索引页结构类似SQL Server 2000结构。每个索引页前96Byte为页头，无槽

>目前使用的页头信息如下：

+ Byte 0~1: 该页剩余可用字节数
+ Byte 2: Tag A 表示索引lever，0,1,...n 0为根页
+ Byte 4: 页类型，0 非叶级索引页，1叶级索引页(非簇集索引叶级页), 2 数据页 **这里写在第四个byte，为兼容数据页格式**
+ Byte 5~6:该索引页中的已有的索引行数量
+ Byte 7~10:上页指针，根页此值为0

###7. Index Line Structure ###

>非叶级页的索引行结构，与SQL Server 2000 有一定区别     

>顺序及具体内容具体如下：

+ 1 Byte: Tag 第0bit，1表示索引码值为定长，0表示索引码值为变长，第1bit，0表示下页为中间页，1表示下页为叶级页，下页类型2，表示该行是叶节点索引行
+ 2 Byte: 该行数据总长度
+ 4 Byte: 下页指针, 存储页号
+ 1 Byte: NULL位图 **如果建索引的字段不允许null，则无此项** 
+ 2 Byte: 定长码值长度
+ n Byte: 定长码值数据 **若码值为变长，则此部分为0byte**
+ 2 Byte: 变长列偏移量 **注意，列偏移数组中存储的是每个变长数据列结束位置相对数据行首地址的偏移量**
+ m Byte: 变长列数据，存储变长码值
+ 2 Byte: 数据记录指针，页号，用于非簇集索引 **簇集索引无此部分**
+ 2 Byte: 数据记录指针，槽号，用于非簇集索引 **簇集索引无此部分** 

###8. Index command ###

>支持的索引操作语句如下：

+ create [UNIQUE|CLUSTERED|NONCLUSTERED] index 索引名称 on 表名(列名) 。
   当选用CLUSTERED时，默认为UNIQUE的，当选用NONCLUSTERED时，可选择是否为UNIQUE