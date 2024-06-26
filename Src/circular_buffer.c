#include <string.h>
#include <stdint.h>
#include "cmsis_gcc.h"
#include "circular_buffer.h"


void init_cbuf(struct cbuf *cbuf) {
    cbuf->head = cbuf->tail = 0;
    cbuf->size = CBUF_LEN + 1;
}

bool cbuf_empty(struct cbuf *cbuf) {
    return (cbuf->head == cbuf->tail);
}

bool cbuf_full(struct cbuf *cbuf) {
    return ((cbuf->tail + 1) % cbuf->size == cbuf->head);
}

uint16_t cbuf_free_space(struct cbuf *cbuf) {
    return (cbuf->head < cbuf->tail) ?
            cbuf->size - cbuf->tail + cbuf->head - 1 :
            cbuf->head - cbuf->tail - 1;
}

cbuf_state cbuf_write_char(struct cbuf *cbuf, uint8_t ch) {
    if (cbuf_full(cbuf))
        return CBUF_ERR;
    cbuf->data[cbuf->tail] = ch;
    cbuf->tail = (cbuf->tail + 1) % cbuf->size;
    return CBUF_OK;
}

cbuf_state cbuf_read_char(struct cbuf *cbuf, uint8_t *ch) {
    if (cbuf_empty(cbuf))
        return CBUF_ERR;

    *ch = cbuf->data[cbuf->head];
    cbuf->head = (cbuf->head + 1) % cbuf->size;
    return CBUF_OK;
}

cbuf_state cbuf_write(struct cbuf *cbuf, uint8_t *msg, uint16_t len) {
    uint32_t primask = __get_PRIMASK();
        __disable_irq();

    for (int i = 0; i < len; i++) {
        if (cbuf_write_char(cbuf, msg[i]) != CBUF_OK)
            return CBUF_ERR;
    }
    __set_PRIMASK(primask);
    return CBUF_OK;
}

cbuf_state cbuf_read(struct cbuf *cbuf, uint8_t *buf, uint16_t len) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    for (int i = 0; i < len; i++) {
        if (cbuf_read_char(cbuf, buf + i) != CBUF_OK)
            return CBUF_ERR;
    }
    __set_PRIMASK(primask);
    return CBUF_OK;
}

cbuf_state cbuf_flush(struct cbuf *cbuf, uint8_t *buf, uint16_t *len) {
    *len = cbuf->size - 1 - cbuf_free_space(cbuf);
    cbuf_state ret = cbuf_read(cbuf, buf, *len);
    return ret;
}

cbuf_state cbuf_write_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t len) {
    if (cbuf_free_space(cbuf) < len)
        return CBUF_ERR;

    uint16_t cur = 0;
    if (cbuf->tail < cbuf->head) {
        memcpy(cbuf->data + cbuf->tail, buf, len);
        cbuf->tail = (cbuf->tail + len) % cbuf->size;
    }
    else {
        if (cbuf->tail + len < cbuf->size) {
            memcpy(cbuf->data + cbuf->tail, buf, len);
            cbuf->tail = (cbuf->tail + len) % cbuf->size;
            return CBUF_OK;
        }

        // copy first block of data to the end of buffer
        memcpy(cbuf->data + cbuf->tail, buf, cbuf->size - cbuf->tail);
        cur = cbuf->size - cbuf->tail;
        len -= cbuf->size - cbuf->tail;
        cbuf->tail = 0;

        // copy second block of data
        memcpy(cbuf->data + cbuf->tail, buf + cur, len);
        cbuf->tail = (cbuf->tail + len) % cbuf->size;
    }
    return CBUF_OK;
}

cbuf_state cbuf_read_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t len) {
    if (cbuf->size - 1 - cbuf_free_space(cbuf) < len)
        return CBUF_ERR;

    uint16_t cur = 0;
    if (cbuf->tail < cbuf->head) {
        if (cbuf->head + len < cbuf->size) {
            memcpy(buf, cbuf->data + cbuf->head, len);
            cbuf->head = (cbuf->head + len) % cbuf->size;
            return CBUF_OK;
        }

        // copy first block of data to the end of buffer
        memcpy(buf, cbuf->data + cbuf->head, cbuf->size - cbuf->head);
        cur = cbuf->size - cbuf->head;
        len -= cbuf->size - cbuf->head;
        cbuf->head = 0;

        // copy second block of data
        memcpy(buf + cur, cbuf->data + cbuf->head, len);
        cbuf->head = (cbuf->head + len) % cbuf->size;
    }
    else {
        memcpy(buf, cbuf->data + cbuf->head, len);
        cbuf->head = (cbuf->head + len) % cbuf->size;
    }
    return CBUF_OK;
}

cbuf_state cbuf_flush_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t *len) {
    *len = cbuf->size - 1 - cbuf_free_space(cbuf);
    cbuf_state ret = cbuf_read_linear(cbuf, buf, *len);
    return ret;
}

