# High level serial terminal driver for STM32 MCUs

## Overview

This project is a driver that uses ST HAL to implement standart `_read` and `_write` system calls, which in turn allow for
`printf()` and `scanf()` usage.

## Features

- Echo received characters back to user
- Use circular buffer to transmit messages in non-blocking mode with DMA
- Blocking reception using `scanf()`
- Non-blocking reception using `non_blocking_rx()` with the possibility to set callback on rx finish
- Reception will be finished on a new line

## Usage

- This driver isn't intended to be built, rather just copied in `Src` and `Inc` directories of STM32 project
- `main.c` contains example of driver application
- Driver should be initialized with `struct uart_driver *init_uart_driver(UART_HandleTypeDef *huart, rx_callback callback)`, 
where `rx_callback` is of type `void (*rx_callback)(struct uart_driver *)` and it will be called only in case of `non_blocking_rx`
- Reception should be initialized with `drv_state init_uart_rx(uint16_t len)`, where `len` is the length of expected message.

