/*
 * dot_matrix.c
 *
 *  Created on: Apr 2, 2020
 *      Author: gez
 */

#include "disp_hd44780u.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DISP_TAG "DISP" /* Used for debugging when calling ESP_LOGI(). */

display_t *display = NULL;

void display_clear(display_t *display)
{
	display_write_single(display, DISP_CLEAR_INSTR, true);
}


void display_wait_ready(display_t *display)
{
	esp_err_t err;

//	gpio_config_t gpios;
//	gpios.mode = GPIO_MODE_INPUT;
//	gpios.pull_up_en = GPIO_PULLUP_ENABLE;
//	gpios.pull_down_en = GPIO_PULLDOWN_DISABLE;
//	gpios.intr_type = GPIO_INTR_DISABLE;
//	for (uint8_t i = 0; i < display->db_mode; ++i)
//	{
//		gpios.pin_bit_mask |= 1UL << (display->db_pins[i]);
//	}
//	err = gpio_config(&gpios);
//	ESP_ERROR_CHECK(err);


	err = gpio_set_level(display->rs_pin, 0);
	ESP_ERROR_CHECK(err);
	err = gpio_set_level(display->rw_pin, 1); //set = reading
	ESP_ERROR_CHECK(err);

	err = gpio_set_level(display->enable_pin, 1);
	ESP_ERROR_CHECK(err);

//	printf("PINS=%x%x%x%x%x%x%x%x\n\r",
//		   gpio_get_level(display->db_pins[7]),
//		   gpio_get_level(display->db_pins[6]),
//		   gpio_get_level(display->db_pins[5]),
//		   gpio_get_level(display->db_pins[4]),
//		   gpio_get_level(display->db_pins[3]),
//		   gpio_get_level(display->db_pins[2]),
//		   gpio_get_level(display->db_pins[1]),
//		   gpio_get_level(display->db_pins[0]));

	while (gpio_get_level(display->db_pins[display->db_mode - 1]))
	{
		printf("WAITING\r\n");
		__asm__ __volatile__("nop");
	}

	err = gpio_set_level(display->enable_pin, 0);
	ESP_ERROR_CHECK(err);

//	gpios.mode = GPIO_MODE_OUTPUT;
//	gpios.pull_up_en = GPIO_PULLUP_DISABLE;
//	for (uint8_t i = 0; i < display->db_mode; ++i)
//	{
//		gpios.pin_bit_mask |= 1UL << (display->db_pins[i]);
//	}
//	err = gpio_config(&gpios);
//	ESP_ERROR_CHECK(err);
}


void display_write_single(display_t *display, uint8_t data, uint8_t is_instr)
{
	display_write_multi(display, &data, 1, is_instr);
}


void display_write_multi(display_t *display, uint8_t *data, uint32_t data_len, uint8_t is_instr)
{
	esp_err_t err;
	uint32_t set_mask = 0;
	uint32_t clear_mask = 0;

	display_wait_ready(display);

	for (uint32_t i = 0; i < data_len; ++i)
	{
		for (uint8_t j = 0; j < display->db_mode; ++j)
		{
			if ((data[i] >> j) & 0x1)
			{
				set_mask |= 1UL << (display->db_pins[j]);
			}
			else
			{
				clear_mask |= 1UL << (display->db_pins[j]);
			}
		}

		if (!is_instr)
		{
			err = gpio_set_level(display->rs_pin, 1);
		}
		else
		{
			err = gpio_set_level(display->rs_pin, 0);
		}
		ESP_ERROR_CHECK(err);
		err = gpio_set_level(display->rw_pin, 0); //clear = writing
		ESP_ERROR_CHECK(err);

//		debug_print_bin(&set_mask, sizeof(set_mask), 1, "set_mask: ");
//		debug_print_bin(&clear_mask, sizeof(clear_mask), 1, "clr_mask: ");

		ESP_LOGI(DISP_TAG,"SET_MASK: %08x", set_mask);
		ESP_LOGI(DISP_TAG,"CLR_MASK: %08x", clear_mask);

		gpio_output_set(set_mask, clear_mask, set_mask | clear_mask, 0);
		set_mask = 0;
		clear_mask = 0;

		err = gpio_set_level(display->enable_pin, 1);
		ESP_ERROR_CHECK(err);
		err = gpio_set_level(display->enable_pin, 0);
		ESP_ERROR_CHECK(err);
	}
}


void display_init(display_t *display)
{
	esp_err_t err;

	gpio_config_t gpios =
	{
		.pin_bit_mask 	= 0,
		.mode 			= GPIO_MODE_OUTPUT,
		.pull_up_en 	= GPIO_PULLUP_DISABLE,
		.pull_down_en 	= GPIO_PULLDOWN_DISABLE,
		.intr_type 		= GPIO_INTR_DISABLE,
	};

	if (display->db_mode != DISP_DB_MODE_8_PIN && display->db_mode != DISP_DB_MODE_4_PIN)
	{
		ESP_LOGE(DISP_TAG, "INVALID DB MODE: 0x%02x", display->db_mode);
		ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
	}

	for (uint8_t i = 0; i < display->db_mode; ++i)
	{
		gpios.pin_bit_mask |= 1UL << (display->db_pins[i]);
	}

	err = gpio_config(&gpios);
	ESP_ERROR_CHECK(err);

	gpios.pull_up_en 	= GPIO_PULLUP_DISABLE;
	gpios.pull_down_en 	= GPIO_PULLDOWN_DISABLE;
	gpios.mode 			= GPIO_MODE_OUTPUT;

	gpios.pin_bit_mask = 0;
	gpios.pin_bit_mask |= 1UL << (display->enable_pin);
	gpios.pin_bit_mask |= 1UL << (display->rw_pin);
	gpios.pin_bit_mask |= 1UL << (display->rs_pin);

	err = gpio_config(&gpios);
	ESP_ERROR_CHECK(err);

	display_clear(display);
	display_write_single(display, 0x38, true);//function set
	display_write_single(display, 0x0f, true);//display on/off control
	display_write_single(display, 0x06, true);//entry mode set
}
