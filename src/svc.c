/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief LED Button Service (LBS) sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "svc.h"
#include <logging/log.h>

// LOG_MODULE_REGISTER(bt_lbs, CONFIG_BT_LBS_LOG_LEVEL);

static bool                   notify_enabled;
// static bool                   button_state;
static struct bt_our_cv       lbs_cb;

static void lbslc_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

static ssize_t check_input(struct bt_conn *conn, const struct bt_gatt_attr *attr, uint16_t len, uint16_t offset) {
	printk("Attribute write, handle: %u, conn: %p\n", attr->handle,
		(void *)conn);
	if (len != 1U) {
		printk("Write led: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		printk("Write led: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}
	return 0;
}

static ssize_t write_led1(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	ssize_t err = check_input(conn, attr, len, offset);
	if( err != 0) {
		return err;
	}

	if (lbs_cb.led1_cb) {
		uint8_t val = *((uint8_t *)buf);
		lbs_cb.led1_cb(val);
	}

	return len;
}

static ssize_t write_led2(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	ssize_t err = check_input(conn, attr, len, offset);
	if( err != 0) {
		return err;
	}

	if (lbs_cb.led2_cb) {
		uint8_t val = *((uint8_t *)buf);
		lbs_cb.led2_cb(val);
	}

	return len;
}

static ssize_t write_led3(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	ssize_t err = check_input(conn, attr, len, offset);
	if( err != 0) {
		return err;
	}

	if (lbs_cb.led3_cb) {
		uint8_t val = *((uint8_t *)buf);
		lbs_cb.led3_cb(val);
	}

	return len;
}

static ssize_t write_led4(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	ssize_t err = check_input(conn, attr, len, offset);
	if( err != 0) {
		return err;
	}

	if (lbs_cb.led4_cb) {
		uint8_t val = *((uint8_t *)buf);
		lbs_cb.led4_cb(val);
	}

	return len;
}

// #ifdef CONFIG_BT_LBS_POLL_BUTTON
// static ssize_t read_button(struct bt_conn *conn,
// 			  const struct bt_gatt_attr *attr,
// 			  void *buf,
// 			  uint16_t len,
// 			  uint16_t offset)
// {
// 	const char *value = attr->user_data;

// 	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
// 		(void *)conn);

// 	if (lbs_cb.button_cb) {
// 		button_state = lbs_cb.button_cb();
// 		return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
// 					 sizeof(*value));
// 	}

// 	return 0;
// }
// #endif

/* LED Button Service Declaration */
BT_GATT_SERVICE_DEFINE(our_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
// #ifdef CONFIG_BT_LBS_POLL_BUTTON
// 	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
// 			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
// 			       BT_GATT_PERM_READ, read_button, NULL,
// 			       &button_state),
// #else
// 	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
// 			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
// 			       BT_GATT_PERM_READ, NULL, NULL, NULL),
// #endif
	BT_GATT_CCC(lbslc_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED1,
			       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE,
			       NULL, write_led1, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED2,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led2, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED3,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led3, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED4,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led4, NULL),
);

int bt_our_init(struct bt_our_cv *callbacks)
{
	if (callbacks) {
		lbs_cb.led1_cb    = callbacks->led1_cb;
		lbs_cb.led2_cb    = callbacks->led2_cb;
		lbs_cb.led3_cb    = callbacks->led3_cb;
		lbs_cb.led4_cb    = callbacks->led4_cb;
		// lbs_cb.button_cb = callbacks->button_cb;
	}

	return 0;
}

// int bt_lbs_send_button_state(bool button_state)
// {
// 	if (!notify_enabled) {
// 		return -EACCES;
// 	}

// 	return bt_gatt_notify(NULL, &lbs_svc.attrs[2],
// 			      &button_state,
// 			      sizeof(button_state));
// }
