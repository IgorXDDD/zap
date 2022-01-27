#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <drivers/gpio.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/lbs.h>

#include <settings/settings.h>

#include <dk_buttons_and_leds.h>


#include <device.h>
#include <drivers/led.h>
#include <sys/util.h>


#if DT_NODE_HAS_STATUS(DT_INST(0, pwm_leds), okay)
#define LED_PWM_NODE_ID		DT_INST(0, pwm_leds)
#define LED_PWM_DEV_NAME	DEVICE_DT_NAME(LED_PWM_NODE_ID)
#else
#error "No LED PWM device found";
#endif

#define LED_PWM_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),

const char *led_label[] = {
	DT_FOREACH_CHILD(LED_PWM_NODE_ID, LED_PWM_LABEL)
};

const int num_leds = ARRAY_SIZE(led_label);

const struct device *led_pwm;

#define MAX_BRIGHTNESS	100
#define FADE_DELAY_MS	50
#define FADE_DELAY	K_MSEC(FADE_DELAY_MS)


#define DEVICE_NAME             "ZAP_PROJEKT"
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

#define RUN_LED_BLINK_INTERVAL  500
#define RUN_LED_BLINK_DELAY		500


#define USER_BUTTON             DK_BTN1_MSK

#define STATE_OFF				0
#define STATE_PWM				1
#define STATE_BLINKING			2
#define STATE_SWITCHING			3
#define STATE_SNAKE				4
#define STATE_LONG_SNAKE		5
#define STATE_ALWAYS_ON			6


static void run_led_test(const struct device *led_pwm, uint8_t led)
{
	int err;
	uint16_t level;
	/* Increase LED brightness gradually up to the maximum level. */
	printk("  Increasing brightness gradually\n");
	for (level = 0; level <= MAX_BRIGHTNESS; level++) {
		err = led_set_brightness(led_pwm, led, level);
		if (err < 0) {
			return;
		}
		k_sleep(FADE_DELAY);
	}
	k_sleep(K_MSEC(1000));
	for (level = MAX_BRIGHTNESS; level >= 0; level--) {
		err = led_set_brightness(led_pwm, led, level);
		if (err < 0) {
			return;
		}
		k_sleep(FADE_DELAY);
	}
	k_sleep(K_MSEC(1000));
	/* Turn LED off. */
	err = led_off(led_pwm, led);
	if (err < 0) {
		return;
	}
	printk("  Turned off, loop end");
}

static void turn_off_leds()
{
	dk_set_led(DK_LED1,false);
	dk_set_led(DK_LED2,false);
	dk_set_led(DK_LED3,false);
	dk_set_led(DK_LED4,false);
}

static int state;
static void handle_states(const struct device *led_pwm)
{
	switch (state)
	{
		case STATE_OFF:
		{
			turn_off_leds();
			break;
		}
	case STATE_PWM:
		{
			run_led_test(led_pwm,0);
			break;
		}
	case STATE_BLINKING:
	{
		dk_set_led(DK_LED1,true);
		dk_set_led(DK_LED2,true);
		dk_set_led(DK_LED3,true);
		dk_set_led(DK_LED4,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_DELAY));
		dk_set_led(DK_LED1,false);
		dk_set_led(DK_LED2,false);
		dk_set_led(DK_LED3,false);
		dk_set_led(DK_LED4,false);

		break;
	}

	case STATE_SWITCHING:
	{
		turn_off_leds();
		dk_set_led(DK_LED1,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED1,false);
		dk_set_led(DK_LED2,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED2,false);
		dk_set_led(DK_LED4,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED4,false);
		dk_set_led(DK_LED3,true);
		break;
	}
	case STATE_SNAKE:
	{
		dk_set_led(DK_LED3,false);
		dk_set_led(DK_LED2,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED1,false);
		dk_set_led(DK_LED4,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED2,false);
		dk_set_led(DK_LED3,true);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED4,false);
		dk_set_led(DK_LED1,true);
		break;
	}
	case STATE_LONG_SNAKE:
	{
		dk_set_led(DK_LED1,true);
		dk_set_led(DK_LED2,false);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED2,true);
		dk_set_led(DK_LED4,false);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED4,true);
		dk_set_led(DK_LED3,false);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		dk_set_led(DK_LED1,false);
		dk_set_led(DK_LED3,true);
		break;
	}
	
	case STATE_ALWAYS_ON:
	{
		dk_set_led(DK_LED1,true);
		dk_set_led(DK_LED2,true);
		dk_set_led(DK_LED3,true);
		dk_set_led(DK_LED4,true);
		break;
	}
	
	default:
		{
			state = 0;
			break;
		}
	}

}

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");
	printk("1.led_pwm = %p\n",led_pwm);
	led_pwm = device_get_binding(LED_PWM_DEV_NAME);
	if (led_pwm) {
		printk("Found device %s", LED_PWM_DEV_NAME);
	} else {
		// LOG_ERR("Device %s not found", LED_PWM_DEV_NAME);
		printk("Device %s not found", LED_PWM_DEV_NAME);
		return;
	}

	if (!num_leds) {
		printk("No LEDs found for %s", LED_PWM_DEV_NAME);
		return;
	}
	printk("2.led_pwm = %d\n",led_pwm);

	//dk_set_led_on(CON_STATUS_LED);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	turn_off_leds();
}


static struct bt_conn_cb conn_callbacks = {
	.connected        = connected,
	.disconnected     = disconnected,
};


static struct bt_conn_auth_cb conn_auth_callbacks;


static void app_led_cb(bool led_state)
{
	if(led_state)
	{		// run_led_test(led_pwm,0);
		state++;
		printk("current state: %d\n",state);
	}
	else
	{
		state=0;
		printk("current state: %d\n",state);
	}
}


static struct bt_lbs_cb lbs_callbacs = {
	.led_cb    = app_led_cb,
	.button_cb = NULL,
};


void main(void)
{
	int err;
	state=0;

	err = dk_leds_init();
	if (err) {
		printk("LEDs init failed (err %d)\n", err);
		return;
	}


	bt_conn_cb_register(&conn_callbacks);
	if (IS_ENABLED(CONFIG_BT_LBS_SECURITY_ENABLED)) {
		bt_conn_auth_cb_register(&conn_auth_callbacks);
	}

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_lbs_init(&lbs_callbacs);
	if (err) {
		printk("Failed to init LBS (err:%d)\n", err);
		return;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
	printk("jest dokladnie %d tyle ledow PWM\n",num_leds);

	for (;;) {
		handle_states(led_pwm);
		k_sleep(K_MSEC(RUN_LED_BLINK_DELAY));
	}
}
