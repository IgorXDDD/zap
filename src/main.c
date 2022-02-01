#include <drivers/gpio.h>
#include <errno.h>
#include <soc.h>
#include <stddef.h>
#include <string.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <zephyr.h>
#include <zephyr/types.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>


#include "svc.h"

#include <settings/settings.h>

#include <dk_buttons_and_leds.h>

#include <device.h>
#include <drivers/led.h>
#include <sys/util.h>

#if DT_NODE_HAS_STATUS(DT_INST(0, pwm_leds), okay)
#define LED_PWM_NODE_ID DT_INST(0, pwm_leds)
#define LED_PWM_DEV_NAME DEVICE_DT_NAME(LED_PWM_NODE_ID)
#else
#error "No LED PWM device found";
#endif

#define LED_PWM_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),

const char *led_label[] = {DT_FOREACH_CHILD(LED_PWM_NODE_ID, LED_PWM_LABEL)};

const int num_leds = ARRAY_SIZE(led_label);

const struct device *led_pwm;

#define MAX_BRIGHTNESS 100
#define FADE_DELAY_MS 10
#define FADE_DELAY K_MSEC(FADE_DELAY_MS)

#define DEVICE_NAME "ZAP_PROJEKT"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_LED_BLINK_INTERVAL 500
#define RUN_LED_BLINK_DELAY 500

#define USER_BUTTON DK_BTN1_MSK

#define STATE_OFF 0
#define STATE_PWM 1
#define STATE_BLINKING 2
#define STATE_SWITCHING 3
#define STATE_SNAKE 4
#define STATE_LONG_SNAKE 5
#define STATE_ALWAYS_ON 6

#define MAX_STATE 6

static void run_led_test(const struct device *led_pwm, uint8_t led) {
  int err;
  uint16_t level;
  /* Increase LED brightness gradually up to the maximum level. */
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
}

static void turn_off_leds() {
  dk_set_led(DK_LED1, false);
  dk_set_led(DK_LED2, false);
  dk_set_led(DK_LED3, false);
  dk_set_led(DK_LED4, false);
}

static unsigned int state;
// static void handle_states(const struct device *led_pwm)
// {
// 	switch (state)
// 	{
// 		case STATE_OFF:
// 		{
// 			turn_off_leds();
// 			break;
// 		}
// 	case STATE_PWM:
// 		{
// 			run_led_test(led_pwm,0);
// 			break;
// 		}
// 	case STATE_BLINKING:
// 	{
// 		dk_set_led(DK_LED1,true);
// 		dk_set_led(DK_LED2,true);
// 		dk_set_led(DK_LED3,true);
// 		dk_set_led(DK_LED4,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_DELAY));
// 		dk_set_led(DK_LED1,false);
// 		dk_set_led(DK_LED2,false);
// 		dk_set_led(DK_LED3,false);
// 		dk_set_led(DK_LED4,false);

// 		break;
// 	}

// 	case STATE_SWITCHING:
// 	{
// 		turn_off_leds();
// 		dk_set_led(DK_LED1,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED1,false);
// 		dk_set_led(DK_LED2,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED2,false);
// 		dk_set_led(DK_LED4,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED4,false);
// 		dk_set_led(DK_LED3,true);
// 		break;
// 	}
// 	case STATE_SNAKE:
// 	{
// 		dk_set_led(DK_LED3,false);
// 		dk_set_led(DK_LED2,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED1,false);
// 		dk_set_led(DK_LED4,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED2,false);
// 		dk_set_led(DK_LED3,true);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED4,false);
// 		dk_set_led(DK_LED1,true);
// 		break;
// 	}
// 	case STATE_LONG_SNAKE:
// 	{
// 		dk_set_led(DK_LED1,true);
// 		dk_set_led(DK_LED2,false);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED2,true);
// 		dk_set_led(DK_LED4,false);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED4,true);
// 		dk_set_led(DK_LED3,false);
// 		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
// 		dk_set_led(DK_LED1,false);
// 		dk_set_led(DK_LED3,true);
// 		break;
// 	}

// 	case STATE_ALWAYS_ON:
// 	{
// 		dk_set_led(DK_LED1,true);
// 		dk_set_led(DK_LED2,true);
// 		dk_set_led(DK_LED3,true);
// 		dk_set_led(DK_LED4,true);
// 		break;
// 	}

// 	default:
// 		{
// 			state = 0;
// 			break;
// 		}
// 	}

// }

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err) {
  if (err) {
    printk("Connection failed (err %u)\n", err);
    return;
  }

  printk("Connected\n");
  printk("1.led_pwm = %p\n", led_pwm);
  led_pwm = device_get_binding(LED_PWM_DEV_NAME);
  if (led_pwm) {
    printk("Found device %s", LED_PWM_DEV_NAME);
  } else {
    printk("Device %s not found", LED_PWM_DEV_NAME);
    return;
  }

  if (!num_leds) {
    printk("No LEDs found for %s", LED_PWM_DEV_NAME);
    return;
  }
  printk("2.led_pwm = %d\n", led_pwm);
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  printk("Disconnected (reason %u)\n", reason);

  turn_off_leds();
}

static struct bt_conn_cb connection_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};


void set_brightness(uint8_t led_num, uint8_t led_state) {
	if (led_state == 200) {
		run_led_test(led_pwm, led_num);
	} else {
		uint8_t brightness = MIN(led_state, MAX_BRIGHTNESS);
		printk("LED %d received PWM value: %d\n", led_num + 1, led_state);
		led_set_brightness(led_pwm, led_num, brightness);
	}
}

static void application_led1_callback(uint8_t led_state) {
	set_brightness(0, led_state);
}

static void application_led2_callback(uint8_t led_state) {
	set_brightness(1, led_state);
}

static void application_led3_callback(uint8_t led_state) {
	set_brightness(2, led_state);
}

static void application_led4_callback(uint8_t led_state) {
	set_brightness(3, led_state);
}

static struct bt_our_cv lbs_callbacks = {
    .led1_cb = application_led1_callback,
    .led2_cb = application_led2_callback,
    .led3_cb = application_led3_callback,
    .led4_cb = application_led4_callback,
    // .button_cb = NULL,
};

void main(void) {
  int err;
  state = STATE_OFF;

  err = dk_leds_init();
  if (err) {
    printk("LEDs init failed (err %d)\n", err);
    return;
  }

  bt_conn_cb_register(&connection_callbacks);

  err = bt_enable(NULL);
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }

  printk("Bluetooth initialized\n");

  err = bt_our_init(&lbs_callbacks);
  if (err) {
    printk("Failed to init LBS (err:%d)\n", err);
    return;
  }

  err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
  if (err) {
    printk("Advertising failed to start (err %d)\n", err);
    return;
  }

  printk("Advertising successfully started\n");
  printk("There are %d PWM leds\n", num_leds);

  for (;;) {
    // handle_states(led_pwm);
    k_sleep(K_MSEC(RUN_LED_BLINK_DELAY));
  }
}
