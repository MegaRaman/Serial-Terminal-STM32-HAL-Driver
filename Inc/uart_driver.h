#ifndef INC_UART_DRIVER_H_
#define INC_UART_DRIVER_H_

#include <stdbool.h>
#include "circular_buffer.h"
#include "main.h"

typedef enum {
    DRV_OK = 0x0,
    DRV_BUSY,
    DRV_ERR
} drv_state;

struct uart_driver;
typedef void (*rx_callback)(struct uart_driver *);

/*
 * struct uart_driver - All data related to serial terminal driver
 * @driver_huart: UART handle, which driver will use to tx/rx data
 * @callback: function that will be called after non_blocking_rx is finished
 * @tx_cbuf: circular buffer, where output data is written to
 * @last_rx_char: pointer to last received char, which then will be echoed back to user
 * @rx_len: reception length, which is set in init_uart_rx
 * @cur_rx_len: counter of already received bytes
 * @drv_busy: indicates if driver is already engaged in data reception
 * */
struct uart_driver {
    UART_HandleTypeDef *driver_huart;
    rx_callback callback;
    struct cbuf tx_cbuf;
    uint8_t *last_rx_char;
    uint16_t rx_len;
    uint16_t cur_rx_len;
    bool drv_busy;
};

/*
 * init_uart_driver - Serial terminal driver initialization, which must be
 * used before any terminal operations
 * @huart: UART handle, which driver will use
 * @callback: non_blocking_rx callback
 *
 * */
struct uart_driver *init_uart_driver(UART_HandleTypeDef *huart, rx_callback callback);

/*
 * init_uart_rx - Pass the driver wanted reception length.
 * Must be used each time program wants to receive data
 * @len: reception length, which will be used in the next scanf/non_blocking_rx call
 *
 * Returns DRV_OK on success, DRV_ERR if len is less than 2, DRV_BUSY if previous reception isn't finished
 * */
drv_state init_uart_rx(uint16_t len);

/*
 * non_blocking_rx - Receive data using DMA
 * @ptr: pointer to the buffer that will store received data
 *
 * Reception length should be initialized with init_uart_rx
 *
 * Returns DRV_OK on success, DRV_BUSY if previous reception isn't finished
 * */
drv_state non_blocking_rx(uint8_t *ptr);

#endif /* INC_UART_DRIVER_H_ */
