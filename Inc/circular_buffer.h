#ifndef INC_CIRCULAR_BUFFER_H_
#define INC_CIRCULAR_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

#define CBUF_LEN 50

typedef enum {
    CBUF_OK = 0x0,
    CBUF_ERR
} cbuf_state;

/*
 * struct cbuf - Circular buffer
 * @data: actual storage, that has one special element that allows
 *        to distinguish between full ring buffer and empty
 * @head: pointer to index where next read will happen
 * @tail: pointer to index where next write will happen
 * @size: actual size of the circular buffer
 * */
struct cbuf {
    uint8_t data[CBUF_LEN + 1];
    uint8_t head;
    uint8_t tail;
    uint16_t size;
};

/*
 * init_cbuf - Initialize circular buffer
 * @cbuf: pointer to the circular buffer, which will be initialized
 * */
void init_cbuf(struct cbuf *cbuf);
bool cbuf_empty(struct cbuf *cbuf);
bool cbuf_full(struct cbuf *cbuf);
uint16_t cbuf_free_space(struct cbuf *cbuf);

/*
 * cbuf_write - Write to the circular buffer byte by byte
 * @cbuf: circular buffer to write to
 * @buf: buffer that will be written to cbuf
 * @len: length of data to write
 *
 * Returns CBUF_OK on success, CBUF_ERR if buffer is full
 * */
cbuf_state cbuf_write(struct cbuf *cbuf, uint8_t *buf, uint16_t len);

/*
 * The same as cbuf_write, just reads from circular buffer
 *
 * Returns CBUF_OK on success, CBUF_ERR if buffer is empty
 * */
cbuf_state cbuf_read(struct cbuf *cbuf, uint8_t *buf, uint16_t len);

/*
 * cbuf_flush - read all ready data from circular buffer
 * @len: pointer to variable where length of read data will be written.
 *       Can be calculated as buffer size minus free space
 * */
cbuf_state cbuf_flush(struct cbuf *cbuf, uint8_t *buf, uint16_t *len);

/*
 * Analogs of previous functions that operate with blocks and don't disable interrupts, hence not thread-safe
 * */
cbuf_state cbuf_write_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t len);
cbuf_state cbuf_read_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t len);
cbuf_state cbuf_flush_linear(struct cbuf *cbuf, uint8_t *buf, uint16_t *len);
#endif /* INC_CIRCULAR_BUFFER_H_ */
