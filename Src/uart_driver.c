#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "uart_driver.h"


/*
 * Can have only one instance of uart_driver because it will be used
 * by _read and _write functions
 * */
struct uart_driver drv;

struct uart_driver *init_uart_driver(UART_HandleTypeDef *huart, rx_callback callback) {
    drv.driver_huart = huart;
    drv.callback = callback;
    drv.drv_busy = false;
    init_cbuf(&drv.tx_cbuf);
    setvbuf(stdout, NULL, _IONBF, 0);
    return &drv;
}

drv_state init_uart_rx(uint16_t len) {
    if (len < 2) {
        return DRV_ERR;
    }
    if (drv.drv_busy) {
        return DRV_BUSY;
    }
    drv.rx_len = len;
    drv.cur_rx_len = 0;
    drv.last_rx_char = NULL;
    return DRV_OK;
}

drv_state non_blocking_rx(uint8_t *ptr) {
    if (drv.drv_busy) {
        return DRV_BUSY;
    }
    drv.drv_busy = true;
    HAL_UART_Receive_DMA(drv.driver_huart, ptr, 1);
    drv.last_rx_char = (uint8_t *)ptr;
    return DRV_OK;
}

int try_to_transmit(uint8_t *ptr, int len) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    HAL_StatusTypeDef hstatus;
    hstatus = HAL_UART_Transmit_DMA(drv.driver_huart, ptr, len);
    if (hstatus != HAL_OK) {
            if (cbuf_write(&drv.tx_cbuf, ptr, len) != CBUF_OK)
                return -1;
    }
    __set_PRIMASK(primask);
    return len;
}

int _read(int file, char *ptr, int len) {
    bool no_newline = true;
    if (drv.cur_rx_len == drv.rx_len)
        return 0;
    if (drv.drv_busy) {
        errno = EBUSY;
        return -1;
    }
    drv.drv_busy = true;
    while (drv.cur_rx_len < drv.rx_len) {
        if (HAL_UART_Receive(drv.driver_huart, (uint8_t*)ptr, 1, HAL_MAX_DELAY) != HAL_OK)
            break;
        drv.cur_rx_len++;
        if (*ptr == '\r') {
            no_newline = false;
            break;
        }
        try_to_transmit((uint8_t*)ptr, 1);
        ptr++;
    }

    if (no_newline)
        *(ptr - 1) = '\r';

    uint8_t tmp[] = "\r\n";
    try_to_transmit(tmp, 2);
    drv.drv_busy = false;
    return drv.cur_rx_len;
}

int _write(int file, char *ptr, int len) {
    return try_to_transmit((uint8_t *)ptr, len);
}

/*
 * Echoes character back to user and starts a reception of the new byte
 * */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (*drv.last_rx_char == '\r') {
        uint8_t tmp = '\n';
        try_to_transmit(&tmp, 1);
    }
    else {
        try_to_transmit(drv.last_rx_char, 1);
    }
    drv.cur_rx_len++;
    if (*drv.last_rx_char != '\r' && drv.cur_rx_len < drv.rx_len) {
        drv.last_rx_char++;
        HAL_UART_Receive_DMA(drv.driver_huart, drv.last_rx_char, 1);
        return;
    }
    if (drv.callback != NULL)
        drv.callback(&drv);
    drv.drv_busy = false;
}

/*
 * Transmit all waiting data from circular buffer
 * */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (cbuf_empty(&drv.tx_cbuf))
        return;
    uint8_t tmp[CBUF_LEN];
    uint16_t len;
    if (cbuf_flush(&drv.tx_cbuf, tmp, &len) != CBUF_OK)
        return;
    HAL_UART_Transmit(drv.driver_huart, tmp, len, HAL_MAX_DELAY);
}
