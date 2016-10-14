/*
 * Copyright (C) 2016 Samsung Electronics
 *   Author: Arun Raghavan <arun@osg.samsung.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <string.h>

#include <glib-object.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>

#include "sync-server.h"
#include "sync-client.h"
#include "sync-tcp-control-client.h"

struct _GstSyncTcpControlClient {
  GObject parent;

  gchar *addr;
  gint port;
  GstSyncServerInfo *info;

  GSocketConnection *conn;
};

struct _GstSyncTcpControlClientClass {
  GObjectClass parent;
};

#define gst_sync_tcp_control_client_parent_class parent_class
G_DEFINE_TYPE (GstSyncTcpControlClient, gst_sync_tcp_control_client,
    G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_ADDRESS,
  PROP_PORT,
  PROP_SYNC_INFO,
};

#define DEFAULT_PORT 0

static void
gst_sync_tcp_control_client_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSyncTcpControlClient *self = GST_SYNC_TCP_CONTROL_CLIENT (object);

  switch (property_id) {
    case PROP_ADDRESS:
      if (self->addr)
        g_free (self->addr);

      self->addr = g_value_dup_string (value);
      break;

    case PROP_PORT:
      self->port = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_sync_tcp_control_client_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstSyncTcpControlClient *self = GST_SYNC_TCP_CONTROL_CLIENT (object);

  switch (property_id) {
    case PROP_ADDRESS:
      g_value_set_string (value, self->addr);
      break;

    case PROP_PORT:
      g_value_set_int (value, self->port);
      break;

    case PROP_SYNC_INFO:
      g_value_set_boxed (value, self->info);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_sync_tcp_control_client_dispose (GObject * object)
{
  GstSyncTcpControlClient *self = GST_SYNC_TCP_CONTROL_CLIENT (object);

  if (self->conn) {
    g_io_stream_close (G_IO_STREAM (self->conn), NULL, NULL);
    g_object_unref (self->conn);
    self->conn = NULL;
  }

  g_free (self->addr);
  self->addr = NULL;

  if (self->info) {
    gst_sync_server_info_free (self->info);
    self->info = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
read_sync_info (GstSyncTcpControlClient * self)
{
  GInputStream *istream;
  gchar msg[4096] = { 0, };
  JsonNode *node;
  GError *err;

  istream = g_io_stream_get_input_stream (G_IO_STREAM (self->conn));

  if (g_input_stream_read (istream, msg, sizeof (msg) - 1, NULL, &err) < 1) {
    g_warning ("Could not read sync info: %s", err->message);
    g_error_free (err);
    return;
  }

  node = json_from_string (msg, &err);
  if (!node) {
    g_warning ("Could not parse JSON: %s", err->message);
    g_error_free (err);
    return;
  }

  self->info = json_boxed_deserialize (GST_TYPE_SYNC_SERVER_INFO, node);

  json_node_unref (node);

  g_object_notify (G_OBJECT (self), "sync-info");
}

static void
gst_sync_tcp_control_client_constructed (GObject * object)
{
  /* We have address and port set, so we can start the socket service */
  GstSyncTcpControlClient *self = GST_SYNC_TCP_CONTROL_CLIENT (object);
  GSocketClient *client;
  GError *err = NULL;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  client = g_socket_client_new ();

  self->conn =  g_socket_client_connect_to_host (client, self->addr,
      self->port, NULL, &err);

  if (!self->conn) {
    g_warning ("Could not connect to server: %s", err->message);
    g_error_free (err);
    goto done;
  }

  read_sync_info (self);

done:
  g_object_unref (client);
}

static void
gst_sync_tcp_control_client_class_init (GstSyncTcpControlClientClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = gst_sync_tcp_control_client_dispose;
  object_class->set_property = gst_sync_tcp_control_client_set_property;
  object_class->get_property = gst_sync_tcp_control_client_get_property;
  object_class->constructed = gst_sync_tcp_control_client_constructed;

  g_object_class_install_property (object_class, PROP_ADDRESS,
      g_param_spec_string ("address", "Address", "Address to listen on", NULL,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_int ("port", "Port", "Port to listen on", 0, 65535,
        DEFAULT_PORT,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_SYNC_INFO,
      g_param_spec_boxed ("sync-info", "Sync info",
        "Sync parameters for clients to use", GST_TYPE_SYNC_SERVER_INFO,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}

static void
gst_sync_tcp_control_client_init (GstSyncTcpControlClient *self)
{
  self->addr = NULL;
  self->port = 0;

  self->info = NULL;

  self->conn = NULL;
}