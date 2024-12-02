# python学习笔记

1. 数字类型：整数、布尔型、浮点数和复数
   
   - int(整数) : 只有一种整数类型 int
   - bool(布尔) ： 如True
   - float(浮点数) : 如 1.23、3E-2
   - complex(复数) : 如 1 + 2j、1.1 + 2.2j

2. 字符串(string):
   
   1. '' 和 "" 使用完全相同
   2. 使用三引号(''' 或 """)可以指定一个多行字符串
   3. 转义符 \
   4. 反斜杠可以用来转义，使用`r`可以让反斜杠不发生转义
   5. 字符串可以用 `+` 连接在一起， 用 `*` 重复
   6. 字符串有两种索引方式，从左到右以 `0` 开始，从右往左从 `-1` 开始
   7. python 中的字符串不能改变
   8. 无单独的字符类型，一个字符就是长度为1的字符串
   9. 字符串的截取的语法格式： `变量[头下标：尾下标：步长]`

3. print 默认输出是换行的，如果要实现不换行需要在变量末尾加上 `end=""`：

4. 在 python 用 import 或者 from...import 来导入相应的模块。
   
        将整个模块(somemodule)导入，格式为： import somemodule
       
        从某个模块中导入某个函数,格式为： from somemodule import somefunction
       
        从某个模块中导入多个函数,格式为： from somemodule import firstfunc, secondfunc, thirdfunc
       
        将某个模块中的全部函数导入，格式为： from somemodule import *

5. Python3 中有六个标准的数据类型：
   
    <font color=blue>列表可以修改，而字符串和元组不能。</font>

| Num     | Str       | List           | Tuple(元组)         | Set(集合)           | Dictionary(字典)         |
|:-------:|:---------:|:--------------:|:-----------------:|:-----------------:|:----------------------:|
| int     | '' 或 ""   | int float str  | int float str     | 无序                | 任意类型对象                 |
| float   | str[0:]   | list(嵌套)       | Tuple(嵌套)         | 不重复元素序列           | 键唯一，值不必                |
| bool    | str[1:-1] | str[0:] [1:-1] | Tuple[0:] [1:-1]  | add，update添加      | 键：不可变；值：任意类型           |
| complex | 元素不可被改变   | 元素可被改变         | 元素不可改变            |                   | dic['键名']              |
|         | '' / ""   | [ ] 前包后不包      | ( )               | { } / set( )(空集合) | { }(空集合)               |
|         |           | append 添加list  |                   |                   | 键多次赋值，后一次保留            |
|         |           | +：组合 *：重复      | +：组合 *：重复 in 迭代   |                   | 可用num，str，list，不可用list |
|         |           |                | tup1 = (50,)(要加,) |                   |                        |

6. 数值的除法包含两个运算符：/ 返回一个浮点数，// 返回一个整数。

7. 在混合计算时，Python会把整型转换成为浮点数。

8. 字符串：用`单引号 ' `或`双引号 " `括起来，同时使用`反斜杠 \ `转义特殊字符。

9. 列表中元素的类型可以不相同，它支持数字，字符串甚至可以包含列表（所谓嵌套）

10. 类型转换
    
    1. 隐式 : 小范围往大范围转
    2. 显式 ： int() float() str()

11. Python 推导式： 可以从一个数据序列构建另一个新的数据序列的结构体 
    
    1. 列表推导式
       
       ```python
       [表达式 for 变量 in 列表]
       [out_exp_res for out_exit in input_list]
       或
       [表达式 for 变量 in 列表 if 条件]
       [out_exp_res for out_exit in input_list if condition]
       ```
       
       |列表推导式 | 字典推导式  | 集合推导式 | 元组推导式|
       | ：-----：| ：-------：| ：------：| ：-----：|
       |

12. Python 算术运算符 
    
    1. / : 返回浮点数
    2. // : 返回整数
    3. ** 返回幂 (a**b a^b)

13. 赋值运算符
    
    1. /=
    2. //=
    3. := 海象运算符

14. 成员运算符
    
    1. in : 在指定序列中找到值返回 True ，否则返回 False
    2. not in ： 未找到返回 True， 否则返回 False

15. 身份运算符  (ps: id() 函数用于获取对象内存地址)
    
    1. is ： 判断两个标识符是不是引用自一个对象
    2. is not ：判断两个标识符是不是引用自不同对象

16. 运算符优先级

| 函数                | 描述           |
|:-----------------:|:------------:|
| **                | 指数(最高优先级)    |
| ~ + =             | 按位翻转，一元加号和减号 |
| * / % //          | 乘 除 求余数 取整除  |
| + -               | 加减法          |
| >> <<             | 右移、左移        |
| &                 | 位与           |
| ^                 |              |
| <= < > >=         | 比较运算符        |
| == !=             | 等于运算符        |
| = %= /= -= *= **= | 赋值运算符        |
| is / is not       | 身份运算符        |
| in / not in       | 成员运算符        |
| not and or        | 逻辑运算符        |

17. 数学函数

| 函数            | 描述                         |
|:-------------:|:--------------------------:|
| abs(x)        | 返回数字绝对值                    |
| ceil(x)       | 返回数字的上入整数                  |
| cmp(x,y)      | python3 已废弃                |
| exp(x)        | 返回 e 的 x 次幂                |
| fabs(x)       | 返回数字的绝对值(float)            |
| floor(x)      | 返回数字的下舍整数                  |
| log(x)        | math.log(100,10) = 2.0     |
| log10(x)      | 返回以 10 为基数的 x 的对数          |
| max(x1,x2)    | 返回最大值，参数可以为序列              |
| min(x1,x2)    | 返回最小值，参数可以为序列              |
| modf(x)       | 返回 x 的整数部分和小数部分            |
| pow(x,y)      | 返回x^y                      |
| round(x,[.n]) | 返回浮点数x的四舍五入值，n代表舍入到小数点后的位数 |
| sqrt(x)       | 返回数字x的平方根                  |

18. 数学常量
    
    1. pi：数学常量 pi
    2. e：数学常量 e，自然常数

19. 进制数
    
    1. 8 进制数： `\yyy`
    2. 16 进制数： `、xyy`

20. python 三引号
    允许一个字符串跨多行，包含换行符、制表符以及其他特殊字符

21. end 关键字
    
    用于将结果输出到同一行，或者在输出的末尾添加不同的字符

22. if 语句
    
    ```python
    if condition_1:
        statement_block_1
    elif condition_2:
        statement_block_2
    else:
        statement_block_3
    ```

23. while 语句 
    
    1. 无 do while
    2. 有 else
    
    ```python
    while 判断条件(condition)：
        执行语句(statements)……
    ```

24. for 语句
    
    1. Python for 循环可以遍历任何可迭代对象，如一个列表或者一个字符串
    2. 有 else
    
    ```python
    for <variable> in <sequence>:
        <statements>
    else:
        <statements>
    ```

25. pass 语句
    
    pass 是空语句，为了保持程序结构的完整性。不做任何事情，一般用作占位语句

26. 迭代器 (not bad)
    
    1. 可以记住遍历的位置的对象
    2. 从集合的第一个元素开始访问，直到所有的元素被访问完结束。迭代器只能往前不会后退
    3. 迭代器有两个基本的方法：`iter()` 和 `next()`
    4. 字符串、列表或元组对象都可用于创建迭代器

27. 生成器 (not bad)
    
    1. 使用了 `yield` 的函数被称为 生成器
    2. 生成器是一个返回迭代器的函数，只能用于迭代操作
    3. 在调用生成器运行的过程中，每次遇到 `yield` 时，函数会暂停并保存当前所有的运行信息，返回 `yield` 的值，并在下一次执行 `next()` 方法时从当前位置继续运行
    4. 调用一个生成器函数，返回的是一个迭代器对象

28. 函数
    
    ```python
    def 函数名（参数列表）:
        函数体
    ```
    
    1. 参数传递：`a = [1, 2, 3]`     `a = 'Runoob'`
    2. 以上代码，a 仅仅是一个对象的引用(一个指针)，可以是指向 `list` 类型的对象，也可以指向 `string` 类型的对象
    3. python 函数的参数传递时：
       1. 不可变类型：类似与值传递，传递的只是值，在函数内部的修改，是新生成一个对象
       2. 可变类型：类似与引用(地址)传递，在函数内部的修改，改变了函数外部的对象
    4. 参数
       1. 必须参数：须以正确的顺序传入函数，调用时的数量必须和声明时一样
       2. 关键字参数：函数调用使用关键字参数来确定传入的参数值(调函数时进行初始化或赋值) `def printinfo(name,age): expression`   `printinfo(age = 20,name = 'bob')`
       3. 默认参数：函数调用时未传入的参数，使用函数实现时默认的参数 `def printinfo_def(name,age=23):expression`
       4. 不定长参数：
          1. 加上 `*` 号的参数以元组的形式传入，存放所有未命名的变量参数  `def print_tuple_info(arg1, *vartuple):expression`
          2. 加上 `**` 的参数以字典的形式传入  `def print_dic_info(arg1,**vardic):expression`
          3. 加上单独的 `*` 后面的参数，要以关键字形式传入(赋值/初始化) `def print_xing_info(a,b,*,c):expression`   `print_xing_info(1,2,c=3)`
          4. 匿名函数：`lambda` 函数只是一个表达式，拥有自己的命名空间，且不能访问自己参数列表之外或全局命名空间里的参数  `lambda [arg1 [,arg2,.....argn]]:expression`

29. 列表推导式
    
    1. 将一些操作应用于某个序列的每个元素，用其获得的结果作为新列表的元素，或者根据确定的判定条件创建子序列
    2. 如果希望表达式推导出一个元组，就必须使用括号

30. 模块
    
    1. 可以用 `import` 引入模块，可以是本身定义的模块，也可以是标准模块
    2. 使用模块时，不用担心不同模块之间的全局变量相互影响

31. 输入和输出
    
    1. 输出：
       
       1. 表达式语句和 `print()` 函数、使用文件对象的 `write()` 方法，标准输出文件可以用 `sys.stdout` 引用， `str.format()` 函数来格式化输出值，
       2. 将输出的值转为字符串 `repr():解释器易读` 、 `str():用户易读` 将输出的值转成字符串
       3. 在 `:` 后传入一个整数，可以保证该域至少有这么多的宽度
    
    2. 输入：
       
       `input()` 函数从标准输入读入一行文本，默认的标准输入是键盘
    
    3. 文件读写：
       
       | 模式    | r   | r+  | w   | w+  | a   | a+  |
       | ----- | --- | --- | --- | --- | --- | --- |
       | 读     | +   | +   |     | +   |     | +   |
       | 写     |     | +   | +   | +   | +   | +   |
       | 创建    |     |     | +   | +   | +   | +   |
       | 覆盖    |     |     | +   | +   |     |     |
       | 指针在开始 | +   | +   | +   | +   |     |     |
       | 指针在结尾 |     |     |     |     | +   | +   |
       
       文件属性配置为写时，如果不写，会相当于清空文件，读取出错，除非把文件指针用 `a+` 放在文件末尾

32. Python 面向对象
    
    1. 类(class) : 用来描述具有相同的属性和方法的对象的集合，定义了该集合中每个对象所共有的属性和方法。对象是类的实例。
    
    2. 方法：**类中定义的函数**
    
    3. 类变量：类变量在整个实例化的对象中是公用的。类变量定义在类中且在函数体之外。类变量通常不作为实例变量使用。
    
    4. 数据成员：类变量或者实例变量用于处理类及其实例对象的相关的数据
    
    5. 方法重写：如果从父类继承的方法不能满足子类的需求，可以对其进行改写，这个过程叫方法的覆盖(重写)
    
    6. 局部变量：定义在方法中的变量，只作用于当前实例的类
    
    7. 实例变量：在类的声明中，属性是用变量来表示的，这种变量就称为实例变量，实例变量就是一个用`self` 修饰的变量
    
    8. 继承：即一个派生类继承基类的字段和方法。继承也允许把一个派生类的对象作为一个基类对象对待
    
    9. 实例化：创建一个类的实例，类的具体对象
    
    10. 对象：通过类定义的数据结构实例。对象包括两个数据成员(类变量和实例变量)和方法
    
    11. `self` 代表类的实例，而非类。类的方法和普通的函数只有一个特别的区别--它们<font color=red>必须</font>有一个额外的**第一个参数名称**，按照惯例它的名称是 `self`
        
        ```python
        # 定义类
        class people:
            # 定义基本属性
            num = ''
            age = 0
            # 定义私有属性，私有属性在类外部无法直接进行访问
            __weight = 0
            # 定义构造方法，self是必须的
            def __init__(self, n, a, w):
                self.name = n
                self.age = a
                self.__weight = w
            def speak(self):
                print("%s 说： 我 %d 岁"%(self.name, self.age))
                print("weight:%d"%(self.__weight))
        ```

        # 实例化类
        # p = people('Bob', 10, 22)
        # p.speak()
        # print(people.age)
        # print(people.__weight) : 外部无法访问
    
    
        # 单继承 :  class DerivedClassName(modname.BaseClassName):
        class student(people):
            grade = ''
            def __init__(self,n,a,w,g):
                people.__init__(self,n,a,w)
                self.grade = g
            # 覆写父类的方法
            def speak(self):        
                print("%s 说：我 %d 岁了，我在读 %d 年级"%(self.name,self.age,self.grade))
    
        # s = student('ken',10,60,3)
        # s.speak()
    
        # 另一个类，多继承之前的准备
        class speaker():
            topic = ''
            name = ''
            def __init__(self,n,t):
                self.name = n
                self.topic = t
            def speak(self):
                print("我叫%s,我是一个演说家，我演讲的主题是 %s"%(self.name,self.topic))
    
        # sss = speaker('Bob','passion')
        # sss.speak()
    
    
        # 多重继承
        class sample(student,speaker):
            a = ''
            def __init__(self, n, a, w, g, t):
                student.__init__(self,n, a, w, g)
                speaker.__init__(self, n, t)
    
        # test = sample("Tom", 25, 80, 4, "Python")
        # test.speak()    # 默认调用的是在括号中参数位置排前父类的方法
    
    
        # 方法重写
        class Parent:   # 定义父类
            def myMethod(self):
                print("调用父类方法")
    
        # 子类名(父类名)
        class Child(Parent):    # 定义子类
            __Pri_Count = 0 # __开头表示，这是私有变量
            PubCount = 0    # 这是公开变量
    
            def myMethod(self): # 对父类的函数重写了
                self.PubCount += 1
                print("调用子类方法")
    
        c = Child()     # 子类实例
        c.myMethod()    # 子类调用重写方法
        super(Child,c).myMethod()   # 用子类对象调用父类已被覆盖的方法
    
        print(c.PubCount)
        # print(c.__Pri_Count)    # Error: 这是一个私有变量，外部无法调用
    
    
        class Site:
            def __init__(self, name, url):
                self.name = name    # public
                self.__url = url    # private
    
            def who(self):
                print('name :',self.name)
                print('url :',self.__url)
    
            def __foo(self):        # 私有方法
                print('这是私有方法')
    
            def foo(self):          # 公共方法
                print("这是公共方法")
                #self.__foo          # 类内部可以调用私有方法
    
    
        ccc = Site('Python','www.python.org')
        ccc.who()   # 正常输出
        ccc.foo()   # 正常输出
        # ccc.__foo() # Error ： 外部不能调用私有方法
    
    
    
        class Vector:
            def __init__(self, a, b):
                self.a = a
                self.b = b
    
            def __str__(self):      # 不懂
                return 'Vector (%d, %d)' % (self.a, self.b)
    
            def __add__(self, other):
                return Vector(self.a + other.a, self.b + other.b)
    
        v1 = Vector(2, 10)
        v2 = Vector(5, -2)
        print(v1 + v2)      # 类的重载
    
        ```
    
    12. 类的方法：使用`def` 关键字来定义一个方法
    
    13. 类的私有方法：`__private_method:`两个下划线开头，声明该方法为私有方法，只能在类的内部调用

33. 命名空间和作用域
    
    1. 命名空间：
       1. 命名空间是从名称到对象的映射，大部分的命名空间都是通过 python 字典来实现的
       2. 命名空间提供了在项目中避免名字冲突的一种方法。每个命名空间是独立的，没有任何关系的，所以一个命名空间中不能有重名，但不同的命名空间可以重名
       3. 一般有三种命名空间：
          1. 内置名称：python 语言内置的名称，比如函数名 `abs、char、BaseException`等
          2. 全局名称：模块中定义的名称，记录了模块的变量，包括函数、类、其它导入的模块、模块级的变量和常量
          3. 局部名称：函数中定义的名称，记录了函数的变量，包括函数的参数和局部定义的变量
          4. 查找顺序：局部的命名变量 --> 全局命名空间 --> 内置命名空间
    2. 作用域：就是一个Python程序可以直接访问命名空间的正文区域。一共有四种：
       1. L(local)：最内层，包含局部变量，比如一个函数/方法内部
       2. E(Enclosing)：包含了非局部也非全局的变量。比如两个嵌套函数，一个函数(或类)A中有包含了B，那么对于B中的名称来说A中的作用域就是非局部(nonlocal：外层非全局作用域)
       3. G(Global)：当前脚本的最外层，比如当前模块的全局变量
       4. B(Built-in)：包含了内建的变量/关键字等，最后被搜索
       5. 查找顺序：L(local) --> E(Enclosing) --> G(Global) --> (Built-in)
    3. Python 中只有模块(module)、类(class)以及函数(def、lambda)才会引入新的作用域，即产生局部变量，其他的代码块(如if/elif/else、try/except、for/while等)不会引入新的作用域，即全局变量
    4. 定义在函数内部的变量是局部变量
    5. 定义在函数外部的变量是全局变量
    6. 当内部作用域想修改外部作用域的变量时，就用`global`(全套作用域) 和 `nonlocal` (嵌套作用域)关键字

34. 正则表达式
    
    1. `^[0-9]+abc$`：`^` 为匹配输入字符串的开始位置；`[0-9]+`：匹配多个数字，`[0-9]`：匹配单个数字；`+`：匹配一个或多个；`abc$`：匹配字母 `abc` 并以 `abc` 结尾， `$`：为匹配输入字符串的结束为止
    
    2. 正则表达式是由普通字符以及特殊字符组成的文字模式
       
       1. 普通字符
          
          | 字符     | 描述                                     |
          |:------:|:--------------------------------------:|
          | [ABC]  | 匹配 [...] 中的所有字符                        |
          | [^ABC] | 匹配除了 [...] 中的所有字符                      |
          | [A-Z]  | 匹配所有大写字母                               |
          | .      | 匹配除换行符 (\r \n) 之外的任何单个字符               |
          | [\s\S] | 匹配所有。\s:匹配所有空白符(包括换行)，\S:匹配非空白符(不包括换行) |
          | \w     | 匹配字母、数字、下划线 ------- [A-Za-z0-9_]       |
          | \d     | 匹配一个数字符号。等价与[0-9]                      |
          | +      | 匹配一个或多个                                |
       
       2. 贪婪：表达式匹配从开始 `<` 到 `>` 之间的所有内容：`/<.*>/`
       
       3. 非贪婪：表达式匹配从开始 `<` 到 `>` 之间的第一个内容：`/<.*?>/`
    
    3. `re.match`只匹配字符串的开始，如果字符串开始不符合正则表达式，则匹配失败，函数返回`None`
    
    4. `re.search`匹配整个字符串，直到找到一个匹配
       
       ```python
       from ast import pattern
       import os
       print(os.getcwd())  # 返回当前的工作目录
       
       # os.chdir('/home/tanxzh/txz/gx/python/')
       
       # os.system('mkdir pyfile')
       
       '''
       from urllib.request import urlopen
       
       for line in urlopen('http://tycho.usno.navy.mil/cgi-bin/timer.pl'):
           line = line.decode('utf-8') # Decoding the binary data to text.
           if 'EST' in line or 'EDT' in line:  # look for Eastern Time
               print(line)
       '''
       ```

       '''
       from datetime import date
       now = date.today()
       print(now)
       print(now.strftime("%m-%d-%y. %d %b %Y is a %A on the %d day of %B."))
    
       birthday = date(1999, 11, 3)
       age = now - birthday
       print(age.days / 365)
    
    
       import zlib
       s = b'witch which has which witches wrist watch'
       print(len(s))   # 41
    
       t = zlib.compress(s)
       print(len(t))
    
    
    
       from timeit import Timer
       print(Timer('t=a;a=b;b=t','a=1;b=2').timeit())
    
    
       import re
       print(re.match('www', 'www.python.org').span())        # 尝试从字符串的起始位置匹配一个字符，如果不是起始位置匹配成功的话，就返回 none
    
       print('\n\n',re.match('www', 'www.python.org'))    # 不在起始位置匹配    
    
       '''
    
    
       '''
       import re
    
       line = "Cats are smarter than dogs"
    
       matchObj = re.match(r'(.*) are (.*?) .*',line, re.M|re.I)
    
       print('\n',matchObj)
    
       if matchObj:
           print ('\n',"matchObj.group() : ", matchObj.group())
           print ('\n',"matchObj.group(1) : ",matchObj.group(1))
           print ('\n',"matchObj.group(2) : ",matchObj.group(2))
       else:
           print ('\n',"No match!!")
    
    
       print(re.search('www','www.python.org').span())
    
       searchObj = re.search( r'(.*) are (.*?) .*', line, re.M | re.I )
    
       if searchObj:
           print ("searchObj.group() : ", searchObj.group())
           print ("searchObj.group(1) : ", searchObj.group(1))
           print ("searchObj.group(2) : ", searchObj.group(2))
       else:
           print ("Nothing found!")
    
       # re.match 只匹配字符串的开始，如果字符串年开始不符合正则表达式，则匹配失败，函数返回None
       # re.search 匹配整个字符串，直到找到一个匹配
       '''
    
       '''
       import re
    
       line = "Cats are smarter than dogs"
       line_dog = "dogs arm smarter than Cats"
    
       # 只匹配dogs 开头的
       matchObj = re.match( r'dogs', line, re.M|re.I ) # can't match
       # matchObj = re.match( r'dogs', line_dog, re.M|re.I ) # match success
       if matchObj:
           print("match --> matchObj.group() : ", matchObj.group())
       else:
           print("NO match!!")
    
       # 匹配整个字符串
       # re.M : 多行匹配
       # re.I : 大小写不敏感
       searchObj = re.search(r'dogs', line_dog, re.M|re.I)
       if searchObj:
           print("search --> matchObj.group() : ",searchObj.group())
       else:
           print("No search!!")
       '''
    
    
       # 检索和替换 re.sub
       '''
       import re
    
       phone = "137-2573-3860 # this is telephone number"
    
       # 删除注释 在 phone 字符串那种 以 #.*$ 规则 替换成""
       num = re.sub(r'#.*$',"",phone)
       print("telephone number:",num)
    
       # 删除非数字的内容
       num = re.sub(r'\D', "", phone)
       print("telephone number :", num)
       '''
    
    
       '''
       # repl
    
       import re
    
       # 将匹配的数字乘以 2
       def double(matched):
           value = int(matched.group('value'))
           return str(value * 2)
    
       s = 'A2312HASD123ASD'
    
       # \d:匹配一个数字符号。等价与[0-9] 
       # +:匹配一个或多个
       # (?<name>exp) 匹配 exp,并捕获文本到名称为 name 的组里
       # double:要替换的字符串，也可以是一个函数(类似函数指针)
    
       # 把数字取出来 * 2 进行替换
       print(re.sub('(?P<value>\d+)',double, s))
       '''
    
       '''
       # compile 函数
    
       import re
    
       pattern = re.compile(r'\d+')    # 用于匹配至少一个数字
    
       # 查找头部
       m = pattern.match('one12twuthree34four')    # can't match
       print(m)
    
       # 从 'e' 的位置开始匹配
       m = pattern.match('one12twuthree34four', 2, 10) # can't match
       print(m)
    
       # 从 '1' 的位置开始匹配
       m = pattern.match('one12twuthree34four', 3, 10) # match success
       print(m)
    
       print(m.start())    # 3 --> o n e 12    获取分组匹配的子串在整个字符串中的起始位置，参数默认为0
       print(m.end())      # 5 --> o n e 12    获取分组匹配的子串在整个字符串中的结束位置，参数默认为0
       print(m.span())     # (3, 5) 返回(start(group), end(group))
    
    
    
       import re
       # 匹配 [a-z]+ [a-z]+  忽略大小写
       pattern = re.compile(r'([a-z]+) ([a-z]+)', re.I)
       m = pattern.match('Hello World Wide Web')
    
       print (m)   # Hello World
    
       print(m.group(0))   # 返回匹配的整个子串 hello world 
       print(m.span(0))    # 返回匹配成功的整个子串的索引
    
       print(m.group(1))   # 返回第一个分组匹配成功的子串
       print(m.span(1))    # 返回第一个分组匹配成功的索引
    
       print(m.group(2))   # 返回第二个分组匹配成功的子串
       print(m.span(2))    # 返回第二个分组匹配成功的索引
    
       # print(m.group(3))   # 返回第三个分组匹配成功的子串  Error!!!
    
       print(m.groups())   # 等价与(m.group(1), m.group(2), ...)
       '''
    
    
    
       import re
    
       # findall:在字符串中找到正则表达式所匹配的所有子串，并返回一个列表
       # 如果有多个匹配模式，则返回元组列表，
       # 如果没有找到匹配的，则返回空列表
    
       # r'\d+':匹配模式
       # 'hello 123 world 234':待匹配的字符串
       result1 = re.findall(r'\d+','hello 123 world 234')
    
       pattern = re.compile(r'\d+')    # 查找数字
    
       # pattern.findall(string[, pos[, endpos]])
       # pos:可选参数，起始位置，默认为0
       # endpos:可选参数，结束位置，默认为字符串的长度
       result2 = pattern.findall('hello 123 world 234')
       result3 = pattern.findall('hello123world123', 0, 10)
    
       print(result1)
       print(result2)
       print(result3)
    
       # 多个匹配模式，返回元组列表
       result4 = re.findall(r'(\w+)=(\d+)', 'set width=20 and height=10')
       print(result4)
       ```

35. Python3 网络编程
    
    1. python 提供了两个级别访问的网络服务
       
       1. 低级别的网络访问服务支持基本的 `Socket`，她提供了标准的 `BSC Sockets API`，可以访问底层操作系统 `Socket` 接口的全部方法
       2. 高级别的网络服务模块 `SocketServer`，它提供了服务器中心类，可以简化网络服务器的开发
    
    2. `Socket`：套接字，应用程序通常通过 套接字 向网络发出请求或者应答网络请求，使主机间或者一台计算机上的进程间可以通讯
    
    3. `socket.socket([family[, type[, proto]]])` : family:套接字家族(AF_UNIX或AF_INET)；type:根据是面向连接(SOCK_STREAM)的还是非连接(SOCK_DGRAM)；protocol:一般不填默认为 0
    
    4. ```txt
       server:
           1. socket()创建socket,选取协议簇
           2. bind()绑定到地址和端口号
           3. listen()允许一个流socket接收其他socket的接入
           4. accpt()调用一个listen()接收app的连接
           5. connect()建议与另一个socket的连接
       
       client:
           1. 创建socket对象
           2. 连接到指定ip主机的端口
       ```

36. 多线程
    
    1. 线程使用
    
    ```python
    import threading
    import time
    
    exitFlag = 0
    ```

    class myThread (threading.Thread):
        def __init__(self, threadID, name, delay):  # 相当于构造函数
            threading.Thread.__init__(self)
            self.threadID = threadID
            self.name = name
            self.delay = delay
            print("create Thread in __init_")
        def run(self):
            print("start_42")
            print ("开始线程：" + self.name)
            print_time(self.name, self.delay, 5)
            print ("结束线程：" + self.name)
    
    def print_time(threadName, delay, counter):
        while counter:
            if exitFlag:
                threadName.exit()
            time.sleep(delay)
            print ("%s:%s" % (threadName, time.ctime(time.time())))
            counter -= 1
    
    # 创建新线程 --- 运行 __init__ 构造函数
    print("create Thread1 after")
    thread1 = myThread(1, "Thread-1", 1)
    print("create Thread1 before")
    thread2 = myThread(2, "Thread-2", 2)
    
    # 开启新线程
    print("start_60")
    thread1.start() # 相当于调用 run 方法
    print("start_62")
    thread2.start()
    thread1.join()  # 等待线程直至结束
    thread2.join()
    print("退出主线程")
    ```
    
    2. 线程特点
       1. 线程主要涉及到_thread(线程库)、threading(线程库，推荐使用的)、queue(线程队列)三个库
       2. 主要包括创建线程、运行线程、等待线程结束
       3. 由于线程间有自己的一组CPU寄存器，且可以通过共享程序中全局变量，来加快信息的共享，但因此也需要保证多个线程运行时，相互之间不能产生干扰
       4. python对于线程信息安全，有线程的上锁和解锁，并提供了线程队列(包含锁)

37. Python 发送各类邮件的主要方法
    1. smtplib 模块
       1. `smtplib.SMTP([host[,port[,local_hostname[,timeout]]]])`
       2. SMTP 类构造函数，表示与 SMTP 服务器之间的连接，通过这个链接可以向 smtp 服务器发送指令，执行相关操作(如：登录、发送邮件)
       3. `SMTP.connect([host,[,port]])`：连接到指定的 smtp 服务器
       4. `SMTP.login(user,password)`：登录到 smtp 服务器。
       5. `SMTP.sendmail(from_addr,to_addrs,msg[,mail_options,rcpt_options])`
