#include <gio/gio.h>
#include <glib.h>
#include <cjson/cJSON.h>
#include <string.h>

// 生成的GDBus接口代码
#include "gdbus-generated.h"

// 全局变量
static GMainLoop *loop = NULL;
static char *current_name = NULL;
static ComExampleGdbusdemo *skeleton = NULL;

// 处理GetName方法调用
static gboolean handle_get_name(ComExampleGdbusdemo *interface,
                               GDBusMethodInvocation *invocation,
                               gpointer user_data) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "name", current_name ? current_name : "");
    char *json_str = cJSON_PrintUnformatted(response);
    
    com_example_gdbusdemo_complete_get_name(interface, invocation, json_str);
    
    g_free(json_str);
    cJSON_Delete(response);
    return TRUE;
}

// 处理SetName方法调用
static gboolean handle_set_name(ComExampleGdbusdemo *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *json_input,
                               gpointer user_data) {
    cJSON *input = cJSON_Parse(json_input);
    cJSON *response = cJSON_CreateObject();
    gboolean success = FALSE;
    
    if (input && cJSON_HasObjectItem(input, "name")) {
        const char *new_name = cJSON_GetObjectItem(input, "name")->valuestring;
        g_free(current_name);
        current_name = g_strdup(new_name);
        success = TRUE;
        
        // 发送NameChanged信号
        cJSON *notification = cJSON_CreateObject();
        cJSON_AddStringToObject(notification, "name", current_name);
        char *notify_str = cJSON_PrintUnformatted(notification);
        com_example_gdbusdemo_emit_name_changed(skeleton, notify_str);
        g_free(notify_str);
        cJSON_Delete(notification);
    }
    
    cJSON_AddBoolToObject(response, "success", success);
    if (success) {
        cJSON_AddStringToObject(response, "name", current_name);
    } else {
        cJSON_AddStringToObject(response, "error", "Invalid input format");
    }
    
    char *json_str = cJSON_PrintUnformatted(response);
    com_example_gdbusdemo_complete_set_name(interface, invocation, json_str);
    
    g_free(json_str);
    cJSON_Delete(response);
    if (input) cJSON_Delete(input);
    return TRUE;
}

static void on_bus_acquired(GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data) {
    GError *error = NULL;
    
    skeleton = com_example_gdbusdemo_skeleton_new();
    g_signal_connect(skeleton, "handle-get-name",
                    G_CALLBACK(handle_get_name), NULL);
    g_signal_connect(skeleton, "handle-set-name",
                    G_CALLBACK(handle_set_name), NULL);
    
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton),
                                    connection,
                                    "/com/example/gdbusdemo",
                                    &error);
    
    if (error != NULL) {
        g_printerr("Error: %s\n", error->message);
        g_error_free(error);
        g_main_loop_quit(loop);
    }
}

static void on_name_lost(GDBusConnection *connection,
                        const gchar *name,
                        gpointer user_data) {
    g_main_loop_quit(loop);
}

int main(void) {
    guint owner_id;
    
    loop = g_main_loop_new(NULL, FALSE);
    
    owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                             "com.example.gdbusdemo",
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             on_bus_acquired,
                             NULL,
                             on_name_lost,
                             NULL,
                             NULL);
    
    g_main_loop_run(loop);
    
    g_bus_unown_name(owner_id);
    g_main_loop_unref(loop);
    g_free(current_name);
    
    return 0;
}