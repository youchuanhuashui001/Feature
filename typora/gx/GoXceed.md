# GoXceed

## 1.DVB系统

1. 名词解释
   - 上行频率: 指发射站把信号发射到卫星的频率
   - 下行频率: 指卫星向地面发射信号所使用的频率
   - Transponder: 转发器。指卫星上用于接收地面发射来的信号，并对该信号进行放大，再以另一个频率向地面进行发射的设备，一个卫星上有多个转发器，每个转发器的频率不同以保证信号不会相互干扰
   - Symbol Rate: 符号率。指数据传输的速率，与信号的比特率及信道的参数有关。单位MB/S
   - 传输流格式:
     - TS (Transport Stream) 常见的传输流
     - GSE (Generic Encapsulated Stream)
     - GCS (Generic Continuous Stream)
     - GFPS (Generic Fixed-lengthPacketized Stream)
   - SIS/MIS: Single Input Stream or Multiple Input Stream 单输入流/多输入流。此单或双是针对TS来说，不同频点只能是单输入流或多输入流，多输入流可理解为该频点下有多个单输入流，每个输入流有对应的tsid用来区分不同的输入流，并且每个输入流下有不同的数据，需要通过切换tsid来达到切换输入流的目的。
   - T2MI: 由进行广播的调制器接收到的第二代数字地面电视信号(DVB-T2)中集成的通信接口。该信号一般是基站将DVBT2信号通过上行链路发给卫星，卫星转发给地面基站，然后基站再进行地面调制，因此在卫星转发的过程中也可以当成DVBS2信号被接收到，只要将T2的封装去掉之后就是完整的TS流，与正常解出来的TS一样了。
   - PLPID: DVB-T2系统的输入包括一个或多个逻辑数据流，每个数据流被一个PLP承载，切换这些数据流可以通过PLPID，类似于DVBS2系统中的tsid概念。

## 播放器：

> 音视频是怎么播出来的?

1. 任何影片的播放都分为3个步骤：拆包->解码->渲染
2. 分别对应播放器的 分离器(demuxer)->解码器(decoder)->渲染器(render)

1. 音频基础
   
   - 声音
     - 声音是由物体振动产生的声波，通过介质传播并能被人或动物听觉器官所感知到的波动现象
   - 音频
     - 人类能听到的所有声音都称之为音频
   - 语音
     - 人类通过发音器官发出来的，具有一定意义
   - 音频采集过程
     - 音频通过采集设备，从连续的模拟信号通过编码存储于计算机等设备上
   - 音频数字化过程
     - 采样
       - 音频的采样是按照固定的频率，在时间轴上对模拟信号的振幅进行取值，这个频率就是采样率(Hz),表示每秒钟取得的采样的个数
       - 奈奎斯特采样定理：采样频率要大学信号最高频率的2倍，才能完整的保留信号的信息。
       - 量化：为了更高效的保存和传输每个采样点的数值，将这些振幅值进行规整，这一过程称为量化
       - 量化的过程会损失一定的精度，按照精度可以将量化分为8位、16位、32位量化等
       - 编码：将量化后离散整数序列咋混华为计算机实际存储所用的二进制字节序列的过程叫做音频编码

2. 脉冲调制编码
   
   - PCM(Pulse Code Modulation,脉冲编码调制)音频数据是未经压缩的音频采样数据裸流,它是由模拟信号经过采样,量化,编码转换成的标准数字音频数据.
   
   - 描述PCM数据的参数:
     
     - 采样频率: 通常8kHz(电话),44.1kHz(CD),48kHz(DVD).
     - 量化位数: 通常8bit,16bit,24bit.
     - 通道个数: 常见的音频有立体声(stereo)和单声道(mono)两种类型,立体声包含左声道和右声道.
   
   - 描述PCM数据的格式:
     
     - 大小端.
     - 交织/非交织.
