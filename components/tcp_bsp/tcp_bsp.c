/*
* @file         tcp_bsp.c 
* @brief        wifi连接事件处理和socket收发数据处理
* @details      在官方源码的基础是适当修改调整，并增加注释
* @author       hx-zsj 
* @par Copyright (c):  
*               红旭无线开发团队，QQ群：671139854
* @par History:          
*               Ver0.0.1:
                     hx-zsj, 2018/08/08, 初始化版本\n 
*/

/* 
=============
头文件包含
=============
*/
#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "tcp_bsp.h"
#include "Smartconfig.h"
#include "Json_parse.h"

#define TAG "User_Wifi" //打印的tag
/*
===========================
全局变量定义
=========================== 
*/
// EventGroupHandle_t Net_sta_group; //wifi建立成功信号量
//socket
static int server_socket = 0;                      //服务器socket
static struct sockaddr_in server_addr;             //server地址
static struct sockaddr_in client_addr;             //client地址
static unsigned int socklen = sizeof(client_addr); //地址长度
static int connect_socket = 0;                     //连接socket
bool g_rxtx_need_restart = false;                  //异常后，重新连接标记

TaskHandle_t tx_rx_task = NULL;
// int g_total_data = 0;

// #if EXAMPLE_ESP_TCP_PERF_TX && EXAMPLE_ESP_TCP_DELAY_INFO

// int g_total_pack = 0;
// int g_send_success = 0;
// int g_send_fail = 0;
// int g_delay_classify[5] = {0};

// #endif /*EXAMPLE_ESP_TCP_PERF_TX && EXAMPLE_ESP_TCP_DELAY_INFO*/

/*
===========================
函数声明
=========================== 
*/

/*
* 接收数据任务
* @param[in]   void  		       :无
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/

void tcp_send_buff(char *send_buff, uint16_t buff_len)
{
    send(connect_socket, send_buff, buff_len, 0);
}

void recv_data(void *pvParameters)
{
    int len = 0;         //长度
    char databuff[1024]; //缓存
    // char *send_buf = "{\"status\":0,\"code\": 0}";

    while (1)
    {
        //清空缓存
        memset(databuff, 0x00, sizeof(databuff));
        //读取接收数据
        len = recv(connect_socket, databuff, sizeof(databuff), 0);
        g_rxtx_need_restart = false;
        if (len > 0)
        {
            //g_total_data += len;
            //打印接收到的数组
            ESP_LOGI(TAG, "recvData: %s", databuff);
            if (ParseTcpUartCmd(databuff) == ESP_OK) //数据解析成功
            {
                // send(connect_socket, send_buf, strlen(send_buf), 0);
                close_socket(); //删除任务前，需要断开连接
                // vTaskDelete(my_tcp_connect_Handle);
                vTaskDelete(tx_rx_task);
            }
            //接收数据回发
            send(connect_socket, databuff, strlen(databuff), 0);

            //sendto(connect_socket, databuff , sizeof(databuff), 0, (struct sockaddr *) &remote_addr,sizeof(remote_addr));
        }
        else
        {
            //打印错误信息
            show_socket_error_reason("recv_data", connect_socket);
            //服务器故障，标记重连
            g_rxtx_need_restart = true;
        }
    }
    close_socket();
    //标记重连
    g_rxtx_need_restart = true;
    vTaskDelete(NULL);
}

/*
* 建立tcp server
* @param[in]   isCreatServer  	    :首次true，下次false
* @retval      void                 :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
esp_err_t create_tcp_server(bool isCreatServer)
{
    //首次建立server
    if (isCreatServer)
    {
        ESP_LOGI(TAG, "server socket....,port=%d", TCP_PORT);
        //新建socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0)
        {
            show_socket_error_reason("create_server", server_socket);
            //新建失败后，关闭新建的socket，等待下次新建
            close(server_socket);
            return ESP_FAIL;
        }
        //配置新建server socket参数
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TCP_PORT);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        //bind:地址的绑定
        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            show_socket_error_reason("bind_server", server_socket);
            //bind失败后，关闭新建的socket，等待下次新建
            close(server_socket);
            return ESP_FAIL;
        }
    }
    //listen，下次时，直接监听
    if (listen(server_socket, 5) < 0)
    {
        show_socket_error_reason("listen_server", server_socket);
        //listen失败后，关闭新建的socket，等待下次新建
        close(server_socket);
        return ESP_FAIL;
    }
    //accept，搜寻全连接队列
    connect_socket = accept(server_socket, (struct sockaddr *)&client_addr, &socklen);
    if (connect_socket < 0)
    {
        show_socket_error_reason("accept_server", connect_socket);
        //accept失败后，关闭新建的socket，等待下次新建
        close(server_socket);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "tcp connection established!");
    return ESP_OK;
}

void my_tcp_connect(void)
{
    //等待WIFI连接信号量，死等
    // xEventGroupWaitBits(tcp_event_group, AP_STACONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "start tcp connected");

    //延时3S准备建立server
    vTaskDelay(3000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "create tcp server");
    //建立server
    int socket_ret = create_tcp_server(true);
    if (socket_ret == ESP_FAIL)
    {
        //建立失败
        ESP_LOGI(TAG, "create tcp socket error,stop...");
    }
    else
    {
        //建立成功
        ESP_LOGI(TAG, "create tcp socket succeed...");
        //建立tcp接收数据任务
        if (pdPASS != xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task))
        {
            //建立失败
            ESP_LOGI(TAG, "Recv task create fail!");
        }
        else
        {
            //建立成功
            ESP_LOGI(TAG, "Recv task create succeed!");
        }
    }
}

/*
* 获取socket错误代码
* @param[in]   socket  		       :socket编号
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
int get_socket_error_code(int socket)
{
    int result;
    u32_t optlen = sizeof(int);
    int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen);
    if (err == -1)
    {
        //WSAGetLastError();
        ESP_LOGE(TAG, "socket error code:%d", err);
        ESP_LOGE(TAG, "socket error code:%s", strerror(err));
        return -1;
    }
    return result;
}

/*
* 获取socket错误原因
* @param[in]   socket  		       :socket编号
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
int show_socket_error_reason(const char *str, int socket)
{
    int err = get_socket_error_code(socket);

    if (err != 0)
    {
        ESP_LOGW(TAG, "%s socket error reason %d %s", str, err, strerror(err));
    }

    return err;
}
/*
* 检查socket
* @param[in]   socket  		       :socket编号
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
int check_working_socket()
{
    int ret;
#if EXAMPLE_ESP_TCP_MODE_SERVER
    ESP_LOGD(TAG, "check server_socket");
    ret = get_socket_error_code(server_socket);
    if (ret != 0)
    {
        ESP_LOGW(TAG, "server socket error %d %s", ret, strerror(ret));
    }
    if (ret == ECONNRESET)
    {
        return ret;
    }
#endif
    ESP_LOGD(TAG, "check connect_socket");
    ret = get_socket_error_code(connect_socket);
    if (ret != 0)
    {
        ESP_LOGW(TAG, "connect socket error %d %s", ret, strerror(ret));
    }
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}
/*
* 关闭socket
* @param[in]   socket  		       :socket编号
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
void close_socket()
{
    close(connect_socket);
    close(server_socket);
}

void my_tcp_connect_task(void *pvParameters)
{

    while (1)
    {
        g_rxtx_need_restart = false;
        //等待WIFI连接信号量，死等
        // xEventGroupWaitBits(tcp_event_group, AP_STACONNECTED_BIT, false, true, portMAX_DELAY);

        ESP_LOGI(TAG, "start tcp connected");

        TaskHandle_t tx_rx_task = NULL;

        //延时3S准备建立server
        vTaskDelay(3000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "create tcp server");
        //建立server
        int socket_ret = create_tcp_server(true);

        if (socket_ret == ESP_FAIL)
        {
            //建立失败
            ESP_LOGI(TAG, "create tcp socket error,stop...");
            continue;
        }
        else
        {
            //建立成功
            ESP_LOGI(TAG, "create tcp socket succeed...");
            //建立tcp接收数据任务
            if (pdPASS != xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task))
            {
                //建立失败
                ESP_LOGI(TAG, "Recv task create fail!");
            }
            else
            {
                //建立成功
                ESP_LOGI(TAG, "Recv task create succeed!");
            }
        }

        while (1)
        {

            vTaskDelay(3000 / portTICK_RATE_MS);

            //重新建立server，流程和上面一样
            if (g_rxtx_need_restart)
            {
                ESP_LOGI(TAG, "tcp server error,some client leave,restart...");
                //建立server
                if (ESP_FAIL != create_tcp_server(false))
                {
                    if (pdPASS != xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task))
                    {
                        ESP_LOGE(TAG, "tcp server Recv task create fail!");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "tcp server Recv task create succeed!");
                        //重新建立完成，清除标记
                        g_rxtx_need_restart = false;
                    }
                }
            }
        }
    }

    vTaskDelete(NULL);
}

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0)
        {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        }
        else if (len == 0)
        {
            ESP_LOGW(TAG, "Connection closed");
        }
        else
        {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            int to_write = len;
            while (to_write > 0)
            {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0)
                {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                to_write -= written;
            }
        }
    } while (len > 0);
}
