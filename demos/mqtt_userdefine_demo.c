/*
 * 这个例程适用于`Linux`这类支持pthread的POSIX设备, 它演示了用SDK配置MQTT参数并建立连接, 之后创建2个线程
 *
 * + 一个线程用于保活长连接
 * + 一个线程用于接收消息, 并在有消息到达时进入默认的数据回调, 在连接状态变化时进入事件回调
 *
 * 需要用户关注或修改的部分, 已经用 TODO 在注释中标明
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_dm_api.h"

/* TODO: 替换为自己设备的设备信息、接入地址(mqtt_host)、端口(port)、证书(user_ca_cert) */
char *username          = "username_*******";
char *password          = "password_*******";
char *client_id         = "client_*******";
char *mqtt_host         = "iot-*******.igw.iothub.aliyuncs.com";
char *product_key       = "*******";
uint16_t port           = 1883;

const char *user_ca_cert = \
{
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDhDCCAmwCCQCs****************************ADCBgzELMAkGA1UEBhMC\r\n" \
    "Q04xETAPBgNVBAgM****************************YW5nemhvdTEQMA4GA1UE\r\n" \
    "CgwHQWxpYmFiYTEM****************************hvcNAQkBFh9taWFvemhh\r\n" \
    "b3hpYS5tenhAYWxp****************************NDA2MjM0OVoXDTMzMDIx\r\n" \
    "MTA2MjM0OVowgYMx****************************DAhaaGVqaWFuZzERMA8G\r\n" \
    "A1UEBwwISGFuZ3po****************************DDAKBgNVBAsMA0lvVDEu\r\n" \
    "MCwGCSqGSIb3DQEJ****************************aWJhYmEtaW5jLmNvbTCC\r\n" \
    "ASIwDQYJKoZIhvcN****************************TMsUOOKv3Bq+380jAAez\r\n" \
    "oEVpdp4xU/fKLAsQ****************************0/oBUercc29bLG4HwHE\r\n" \
    "moaUuLzK8VjpTtOE****************************aFdL5aeCXjfkRcF6dUO\r\n" \
    "ks3EwLIHqe2NsmtC****************************9pVL2W/pedbumsgmkq3l\r\n" \
    "2eGmXqXR3MbNUyd0****************************DjUMW0YS5E1e6DkaT7wc\r\n" \
    "bOFXrBb3BBJT+37U****************************qKolLloc6oDKaa9P8zMC\r\n" \
    "AwEAATANBgkqhkiG****************************J6nskBzSlxntJ7BOlek8\r\n" \
    "xYea8Y43z4tIkOGR****************************1+oi/1dD+bOlVNie1TeB\r\n" \
    "IddHujeL9mGqwVXf****************************lfBaReekvnViCDt+BIk\r\n" \
    "2ZoUubd10huLbk9w****************************/NlZyAv6mSLAAqpFC94/\r\n" \
    "d2g4/qbtp8BIgB/j****************************f2FnWZDfmpWaxqqWsTIa\r\n" \
    "QGaCoMHGBiWegUr9****************************+hjCNH+iCw==\r\n" \
    "-----END CERTIFICATE-----\r\n" \
};
/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 运行时全局变量 */
static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] mqtt_basic_demo&gb80sFmX7yX
 *
 * 上面这条日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h
 *
 */

/* 日志回调函数, SDK的日志会从这里输出 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    printf("%s", message);
    return 0;
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
            /* TODO: 处理SDK建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
            /* TODO: 处理SDK重连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
            /* TODO: 处理SDK被动断连, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        default: {

        }
    }
}

/* MQTT默认消息处理回调, 当SDK从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    switch (packet->type) {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
            printf("heartbeat response\n");
            /* TODO: 处理服务器对心跳的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK: {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
            /* TODO: 处理服务器对订阅请求的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_PUB: {
            printf("pub, qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            /* TODO: 处理服务器下发的业务报文 */
        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: {
            printf("puback, packet id: %d\n", packet->data.pub_ack.packet_id);
            /* TODO: 处理服务器对QoS1上报消息的回应, 一般不处理 */
        }
        break;

        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int32_t     res = STATE_SUCCESS;
    void       *mqtt_handle = NULL;
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;    /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384;                        /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                                 /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = user_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(user_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* TODO: 如果以下代码不被注释, 则例程会用TCP而不是TLS连接云平台, 端口号也需要改为30183 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */

    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)mqtt_host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备username */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_USERNAME, (void *)username);
    /* 配置设备password */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PASSWORD, (void *)password);
    /* 配置设备client_id */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_CLIENTID, (void *)client_id);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)username);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);
    /* 关闭对topic的以'/'为开头的校验 */
    uint8_t topic_check = 0;
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_TOPIC_HEADER_CHECK, (void *)&topic_check);

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n\r\n", -res);
        printf("please check variables like mqtt_host, produt_key, device_name, device_secret in demo\r\n");
        return -1;
    }

    /* MQTT 订阅topic功能示例, 请根据自己的业务需求进行使用 */
    /* {
        char *sub_topic = "sub_topic_example";

        res = aiot_mqtt_sub(mqtt_handle, sub_topic, NULL, 1, NULL);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    } 
    */
    

    /* MQTT 发布消息功能示例, 请根据自己的业务需求进行使用 */
    /* {
        char *pub_topic = "pub_topic_example";
        char *pub_payload = "hello world\r\n";

        res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)pub_payload, (uint32_t)strlen(pub_payload), 0);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    } 
    */

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_process_thread failed: %d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_recv_thread failed: %d\n", res);
        return -1;
    }

    /* 主循环进入休眠 */
    while (1) {
        sleep(1);
    }

    /* 断开MQTT连接, 一般不会运行到这里 */
    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    sleep(1);
    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_disconnect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 销毁MQTT实例, 一般不会运行到这里 */
    res = aiot_mqtt_deinit(&mqtt_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_mqtt_deinit failed: -0x%04X\n", -res);
        return -1;
    }

    return 0;
}

