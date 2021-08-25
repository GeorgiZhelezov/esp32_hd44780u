/*
 * disp_hd44780u.h
 *
 *  Created on: Apr 19, 2021
 *      Author: gez
 */

#ifndef MAIN_DISP_HD44780U_H_
#define MAIN_DISP_HD44780U_H_

#include <stdint.h>

#define DISP_DB_MODE_8_PIN 	0x8 /* 8 pin mode. */
#define DISP_DB_MODE_4_PIN 	0x4 /* 4 pin mode. */

/* All bits that define instruction types, lower bits are parameters for the instruction. */
#define DISP_CLEAR_INSTR				(1<<0)
#define DISP_RETURN_HOME_INSTR 			(1<<1)
#define DISP_ENTRY_MODE_INST 			(1<<2)
#define DISP_ON_OFF_CONTROL_INST 		(1<<3)
#define DISP_CURSOR_DISP_SHIFT_INST 	(1<<4)
#define DISP_FUNCTION_SET_INST 			(1<<5)
#define DISP_SET_CGRAM_INST 			(1<<6)
#define DISP_SET_DDRAM_INST 			(1<<7)

/*
 * @brief Struct containing all pins and display mode.
 *
 * @details db_mode MUST be either DISP_DB_MDOE_8_PIN or DISP_DB_MODE_4_PIN.
 */
typedef struct
{
	uint8_t rs_pin; 		/**< Register select pin. */
	uint8_t rw_pin; 		/**< Read/Write pin. */
	uint8_t enable_pin; 	/**< Enable pin. */
	uint8_t db_mode; 		/**< Data pins mode */
	uint8_t db_pins[]; 	/**< Data pins */
} display_t;

extern display_t *display;

/*
 * @brief Configures all GPIOs contained in the struct to output or output + input.
 *
 * @details Sets RS, RW and EN pins to output only and data pins D0-D7 as input + output.
 *
 * @param[in] display Pointer to display_t struct containing HD44780U pins.
 */
void display_init(display_t *display);

/*
 * @brief Sends one or more bytes to the display be it data or instructions.
 *
 * @details Sets or clears the corresponding data pins depending on the bytes from the byte data array,
 * 			sets or clears the RS pin depending on if the data being passed is an instruction or not,
 * 			clears RW pin because the function is for writing to the display,
 * 			toggles EN pin so that the data (whatever it is, instruction or normal data) is passed to the HD44780U MCU.
 *
 * @param[in] display 	Pointer to display_t struct containing HD44780U pins.
 * @param[in] data 		Pointer to data byte array. Bytes in this array will affect the corresponding data pins.
 * @param[in] data_len 	Lenght of the data byte array.
 * @param[in] is_instr 	Flag for identifying if the data byte array is an instruction/s or not.
 * 						1 - byte data array contains instructions
 * 						0 - byte data array is normal data
 */
void display_write_multi(display_t *display, uint8_t *data, uint32_t data_len, uint8_t is_instr);

/*
 * @brief Sends only one byte to the dislay.
 *
 * @details This function calls display_write_multi() with 1 as the data_lenght parameter.
 *
 * @param[in] display 	Pointer to display_t struct containing HD44780U pins.
 * @param[in] data 		Pointer to data byte array. Bytes in this array will affect the corresponding data pins.
 * @param[in] data_len 	Lenght of the data byte array.
 * @param[in] is_instr 	Flag for identifying if the data byte array is an instruction/s or not.
 * 						1 - byte data array contains instructions
 * 						0 - byte data array is normal data
 */
void display_write_single(display_t *display, uint8_t data, uint8_t is_instr);

/*
 * @brief Waits for the BUSY FLAG to be cleared by the HD44780U MCU
 *
 * @details Clears RS pin, sets RW (for reading) and EN pins, then waits for pin D7 (on the display, busy flag is there)
 * 			to be cleared. After that the EN pin is cleared.
 *
 * @param[in] display 	Pointer to display_t struct containing HD44780U pins.
 */
void display_wait_ready(display_t *display);

/*
 * @brief Clears the contents of the display.
 *
 * @details Calles display_write_single() with DISP_CLEAR_INSTR as the data byte and sets is_instr to true.
 *
 * @param[in] display Pointer to display_t struct containing HD44780U pins.
 */
void display_clear(display_t *display);

#endif /* MAIN_DISP_HD44780U_H_ */
