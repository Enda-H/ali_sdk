# 文件说明

记录`Link SDK-4.x`的更新历史

# 更新内容

+ 2020-05-06: SDK 4.0.0版本正式发布
+ 2021-04-20: SDK 4.1.0版本正式发布
  +  新增安全隧道和远程调试功能
  +  新增AT模组驱动框架,支持模组快速适配
  +  mbedtls安全层抽象
  +  支持单报文多url的OTA
  +  新增基于mqtt的动态注册功能
  +  支持MQTT 5.0协议
+ 2022-10-28:
  +  支持coap协议接入及动态注册
  +  mbedtls版本升级至2.13.1
+ 2023-01-17:
  +  支持数据压缩的功能
  +  DTLS支持裁剪

# 模块状态


| 模块名                                      | 更新时间    | Commit ID
|---------------------------------------------|-------------|---------------------------------------------
| 核心模块(core)                              | zhijian     | faf2fb55be60356d0ae5832369c48c2ffc0215c9
| (components/mqtt_upload)                    | 2022-03-15  | 0947d5ecbe5d8fccbdc22374f24cfe44abd58aae
| 基于MQTT的动态注册(components/dynreg-mqtt)  | 2023-05-22  | 0c9e1eb5e15da19ed56edc57bc9a2d658c2439f5
| 物模型模块(components/data-model)           | 2021-09-06  | 1d9270de816f7ff0f60c0b2a53d08ca4da8bab66
| 时间同步模块(components/ntp)                | 2021-09-16  | cb96f929c231ad8ee8c48dcf82167f3f6eb66dad
| 设备日志模块(components/logpost)            | 2021-09-15  | d0e41935909d0c7f593f9225e119f7698db67b2d
| 固件升级模块(components/ota)                | 2022-10-27  | fed5f95ad503a56c8d6413495d0c3f391fbfb6c0
| 动态注册(components/dynreg)                 | 2022-12-13  | cd069167a8f93afa0950b0fb03593001e4c29ddf



