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

#ifndef __GST_SYNC_TCP_CONTROL_SERVER_H
#define __GST_SYNC_TCP_CONTROL_SERVER_H

#include <glib.h>

G_BEGIN_DECLS
typedef struct _GstSyncTcpControlServer GstSyncTcpControlServer;
typedef struct _GstSyncTcpControlServerClass GstSyncTcpControlServerClass;

#define GST_TYPE_SYNC_TCP_CONTROL_SERVER \
  (gst_sync_tcp_control_server_get_type ())
#define GST_SYNC_TCP_CONTROL_SERVER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_SYNC_TCP_CONTROL_SERVER, \
                               GstSyncTcpControlServer))
#define GST_SYNC_TCP_CONTROL_SERVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_SYNC_TCP_CONTROL_SERVER, \
                            GstSyncTcpControlServerClass))
#define GST_IS_SYNC_TCP_CONTROL_SERVER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_SYNC_TCP_CONTROL_SERVER))
#define GST_IS_SYNC_TCP_CONTROL_SERVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_SYNC_TCP_CONTROL_SERVER))

GType gst_sync_tcp_control_server_get_type ();

/* API */

gboolean gst_sync_tcp_control_server_start (GstSyncTcpControlServer * self,
    GError ** error);

void gst_sync_tcp_control_server_stop (GstSyncTcpControlServer * self);

G_END_DECLS

#endif /* __GST_SYNC_TCP_CONTROL_SERVER_H */