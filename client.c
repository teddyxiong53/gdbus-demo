#include <gio/gio.h>
#include <glib.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <string.h>

// 生成的GDBus接口代码
#include "gdbus-generated.h"

static GMainLoop *loop = NULL;
static ComExampleGdbusdemo *proxy = NULL;

// 处理NameChanged信号
static void on_name_changed(ComExampleGdbusdemo *object,
                          const gchar *json_notification,
                          gpointer user_data) {
    printf("收到通知: %s\n", json_notification);
}

// 处理标准输入
static gboolean handle_stdin(GIOChannel *channel,
                            GIOCondition condition,
                            gpointer data) {
    gchar *line = NULL;
    gsize len = 0;
    GError *error = NULL;

    if (g_io_channel_read_line(channel, &line, &len, NULL, &error) != G_IO_STATUS_NORMAL) {
        if (error) {
            g_printerr("读取输入错误: %s\n", error->message);
            g_error_free(error);
        }
        return TRUE;
    }

    if (line) {
        g_strchomp(line);  // 移除尾部的换行符
        
        cJSON *input = cJSON_Parse(line);
        if (!input) {
            g_printerr("无效的JSON格式\n");
            g_free(line);
            return TRUE;
        }

        cJSON *cmd = cJSON_GetObjectItem(input, "cmd");
        if (!cmd || !cJSON_IsString(cmd)) {
            g_printerr("缺少cmd字段或格式错误\n");
            cJSON_Delete(input);
            g_free(line);
            return TRUE;
        }

        if (strcmp(cmd->valuestring, "get") == 0) {
            // 处理get命令
            GError *error = NULL;
            gchar *response = NULL;
            com_example_gdbusdemo_call_get_name_sync(proxy, &response, NULL, &error);
            
            if (error) {
                g_printerr("GetName调用失败: %s\n", error->message);
                g_error_free(error);
            } else {
                printf("GetName响应: %s\n", response);
                g_free(response);
            }
        } else if (strcmp(cmd->valuestring, "set") == 0) {
            // 处理set命令
            cJSON *name = cJSON_GetObjectItem(input, "name");
            if (!name || !cJSON_IsString(name)) {
                g_printerr("set命令缺少name字段或格式错误\n");
            } else {
                cJSON *request = cJSON_CreateObject();
                cJSON_AddStringToObject(request, "name", name->valuestring);
                char *json_str = cJSON_PrintUnformatted(request);

                GError *error = NULL;
                gchar *response = NULL;
                com_example_gdbusdemo_call_set_name_sync(proxy, json_str, &response, NULL, &error);

                if (error) {
                    g_printerr("SetName调用失败: %s\n", error->message);
                    g_error_free(error);
                } else {
                    printf("SetName响应: %s\n", response);
                    g_free(response);
                }

                g_free(json_str);
                cJSON_Delete(request);
            }
        } else {
            g_printerr("未知命令: %s\n", cmd->valuestring);
        }

        cJSON_Delete(input);
        g_free(line);
    }

    return TRUE;
}

int main(void) {
    GError *error = NULL;
    
    loop = g_main_loop_new(NULL, FALSE);
    
    // 连接到D-Bus会话总线
    proxy = com_example_gdbusdemo_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                        G_DBUS_PROXY_FLAGS_NONE,
                                                        "com.example.gdbusdemo",
                                                        "/com/example/gdbusdemo",
                                                        NULL,
                                                        &error);
    
    if (error != NULL) {
        g_printerr("无法创建代理: %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    
    // 连接信号处理函数
    g_signal_connect(proxy, "name-changed",
                    G_CALLBACK(on_name_changed), NULL);
    
    // 设置标准输入处理
    GIOChannel *stdin_channel = g_io_channel_unix_new(STDIN_FILENO);
    g_io_add_watch(stdin_channel, G_IO_IN, handle_stdin, NULL);
    
    printf("客户端已启动。请输入命令（JSON格式）:\n");
    printf("获取名称: {\"cmd\": \"get\"}\n");
    printf("设置名称: {\"cmd\": \"set\", \"name\": \"新名称\"}\n");
    
    g_main_loop_run(loop);
    
    g_object_unref(proxy);
    g_io_channel_unref(stdin_channel);
    g_main_loop_unref(loop);
    
    return 0;
}