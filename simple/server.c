#include <gio/gio.h>

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.example.TestInterface'>"
    "    <method name='Echo'>"
    "      <arg type='s' name='msg' direction='in'/>"
    "      <arg type='s' name='reply' direction='out'/>"
    "    </method>"
    "    <signal name='Ping'>"
    "      <arg type='s' name='msg'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

static GDBusNodeInfo *introspection_data;

static void on_method_call(GDBusConnection *conn, const gchar *sender,
                           const gchar *obj_path, const gchar *iface_name,
                           const gchar *method_name, GVariant *params,
                           GDBusMethodInvocation *invocation, gpointer user_data) {
    if (g_strcmp0(method_name, "Echo") == 0) {
        const gchar *msg;
        g_variant_get(params, "(&s)", &msg);
        g_print("Server received: %s\n", msg);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", msg));
    }
}

static const GDBusInterfaceVTable vtable = {
    on_method_call, NULL, NULL
};

static gboolean send_ping(gpointer user_data) {
    GDBusConnection *conn = user_data;
    g_dbus_connection_emit_signal(conn, NULL, "/org/example/TestObject",
        "org.example.TestInterface", "Ping",
        g_variant_new("(s)", "Hello from server!"), NULL);
    return TRUE; // keep sending
}

static void on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data) {
    GError *err = NULL;
    g_dbus_connection_register_object(conn,
        "/org/example/TestObject",
        introspection_data->interfaces[0],
        &vtable, NULL, NULL, &err);
    g_timeout_add_seconds(2, send_ping, conn);
}

int main() {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);

    g_bus_own_name(G_BUS_TYPE_SESSION,
                   "org.example.TestService",
                   G_BUS_NAME_OWNER_FLAGS_NONE,
                   on_bus_acquired, NULL, NULL, NULL, NULL);

    g_main_loop_run(loop);
    g_dbus_node_info_unref(introspection_data);
    return 0;
}
