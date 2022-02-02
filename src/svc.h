#ifndef BT_LED_SVC_H_
#define BT_LED_SVC_H_



#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

#define BT_UUID_LED_SVC_VAL \
	BT_UUID_128_ENCODE(0x00000001, 0x1212, 0xefde, 0x1523, 0x785feabcd123)


#define BT_UUID_LED_SVC_LED1_VAL \
	BT_UUID_128_ENCODE(0x00000002, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

#define BT_UUID_LED_SVC_LED2_VAL \
	BT_UUID_128_ENCODE(0x00000003, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

#define BT_UUID_LED_SVC_LED3_VAL \
	BT_UUID_128_ENCODE(0x00000004, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

#define BT_UUID_LED_SVC_LED4_VAL \
	BT_UUID_128_ENCODE(0x00000005, 0x1212, 0xefde, 0x1523, 0x785feabcd123)


#define BT_UUID_LED_SVC           BT_UUID_DECLARE_128(BT_UUID_LED_SVC_VAL)
#define BT_UUID_LED_SVC_LED1       BT_UUID_DECLARE_128(BT_UUID_LED_SVC_LED1_VAL)
#define BT_UUID_LED_SVC_LED2       BT_UUID_DECLARE_128(BT_UUID_LED_SVC_LED2_VAL)
#define BT_UUID_LED_SVC_LED3       BT_UUID_DECLARE_128(BT_UUID_LED_SVC_LED3_VAL)
#define BT_UUID_LED_SVC_LED4       BT_UUID_DECLARE_128(BT_UUID_LED_SVC_LED4_VAL)


typedef void (*led_cb_t)(const uint8_t led_state);


struct bt_led_svc_cbs {
	/** LED state change callback. */
	led_cb_t    led1_cb;
	led_cb_t    led2_cb;
	led_cb_t    led3_cb;
	led_cb_t    led4_cb;
};

int bt_led_svc_init(struct bt_led_svc_cbs *callbacks);


#ifdef __cplusplus
}
#endif


#endif /* BT_LED_SVC_H_ */