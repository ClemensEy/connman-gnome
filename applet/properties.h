/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2008  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

typedef void (* property_callback_t) (DBusGProxy *proxy, const char *property,
					GValue *value, gpointer user_data);

void properties_create(DBusGProxy *proxy, property_callback_t callback,
							gpointer user_data);
void properties_enable(DBusGProxy *proxy);
void properties_disable(DBusGProxy *proxy);
void properties_destroy(DBusGProxy *proxy);
