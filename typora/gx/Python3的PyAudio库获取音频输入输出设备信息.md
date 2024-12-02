# Python3 的 PyAudio 库获取音频输入输出设备信息

​        此文章引用自 https://www.bilibili.com/read/cv10329442 出处：bilibili。

1. 安装 pyaudio ： 
   
   1. 安装依赖库：`sudo apt-get install libasound-dev portaudio19-dev libportaudio2 libportaudiocpp0`
   2. 使用 pip3 安装：`pip install pyaudio`

2. 编辑代码：
   
   ```python
   import pyaudio
   ```
   
   def get_audio_devices():
   
       p = pyaudio.PyAudio()
       devices = []
       for i in range(p.get_device_count()):
           # print(p.get_device_info_by_index(i).get('name'))
           devices.append(p.get_device_info_by_index(i))
       return devices
   
   def get_audio_input_devices():
   
       devices = []
       for item in get_audio_devices():
           if item.get('maxInputChannels') > 0:
               devices.append(item)
       return devices
   
   def get_audio_output_devices():
   
       devices = []
       for item in get_audio_devices():
           if item.get('maxOutputChannels') > 0:
               devices.append(item)
       return devices
   
   if __name__ == '__main__':
   
       # get_audio_devices()
       print('输入设备:')
       for item in get_audio_input_devices():
           print(item.get('name'))
       print('输出设备:')
       for item in get_audio_output_devices():
           print(item.get('name')) 

```
3. 使用 PyAudio 库获取输入设备时存在 bug ，获取到的采样率是输出设备的采样率，无法获取到输入设备的采样率.

4. 可以在 ubuntu 下使用命令 `cat /proc/asound/card1/stream0 ` 查看设备的参数
```
