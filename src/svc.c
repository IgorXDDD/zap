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


static bool                   notify_enabled;
static struct bt_led_svc_cbs       lbs_cb;

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

static ssize_t run_callback(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags,
			 led_cb_t callback) {
	ssize_t err = check_input(conn, attr, len, offset);
	if( err != 0) {
		return err;
	}

	if (callback) {
		uint8_t val = *((uint8_t *)buf);
		callback(val);
	}

	return len;

}

static ssize_t write_led1(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	return run_callback(conn, attr, buf, len, offset, flags, lbs_cb.led1_cb);
}

static ssize_t write_led2(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	return run_callback(conn, attr, buf, len, offset, flags, lbs_cb.led2_cb);
}

static ssize_t write_led3(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{

	return run_callback(conn, attr, buf, len, offset, flags, lbs_cb.led3_cb);
}

static ssize_t write_led4(struct bt_conn *conn,
			 const struct bt_gatt_attr *attr,
			 const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	return run_callback(conn, attr, buf, len, offset, flags, lbs_cb.led4_cb);
}


BT_GATT_SERVICE_DEFINE(our_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_LED_SVC),
	BT_GATT_CCC(lbslc_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_LED_SVC_LED1,
			       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE,
			       NULL, write_led1, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LED_SVC_LED2,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led2, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LED_SVC_LED3,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led3, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_LED_SVC_LED4,
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE,
				NULL, write_led4, NULL),
);

int bt_led_svc_init(struct bt_led_svc_cbs *callbacks)
{
	if (callbacks) {
		lbs_cb.led1_cb    = callbacks->led1_cb;
		lbs_cb.led2_cb    = callbacks->led2_cb;
		lbs_cb.led3_cb    = callbacks->led3_cb;
		lbs_cb.led4_cb    = callbacks->led4_cb;
	}

	return 0;
}


