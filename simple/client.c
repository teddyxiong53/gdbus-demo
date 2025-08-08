#include <gio/gio.h>

static void on_signal_received(GDBusConnection *conn, const gchar *sender,
                               const gchar *obj_path, const gchar *iface_name,
                               const gchar *signal_name, GVariant *params,
                               gpointer user_data) {
    const gchar *msg;
    g_variant_get(params, "(&s)", &msg);
    g_print("Client received signal: %s\n", msg);
}

int main() {
    GError *err = NULL;
    GDBusConnection *conn;
    GVariant *res;
    const gchar *reply;

    conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);

    // 调用 Echo 方法
    res = g_dbus_connection_call_sync(conn,
        "org.example.TestService",          // bus name
        "/org/example/TestObject",          // object path
        "org.example.TestInterface",        // interface
        "Echo",                             // method
        g_variant_new("(s)", "Hello Server"),
        NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &err);

    if (res) {
        g_variant_get(res, "(&s)", &reply);
        g_print("Client got reply: %s\n", reply);
        g_variant_unref(res);
    }

    // 订阅 Ping 信号
    g_dbus_connection_signal_subscribe(conn,
        "org.example.TestService",
        "org.example.TestInterface",
        "Ping",
        NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
        on_signal_received, NULL, NULL);

    g_main_loop_run(g_main_loop_new(NULL, FALSE));
    return 0;
}
