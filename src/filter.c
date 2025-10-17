#include <stdlib.h>
#include <string.h>
#include "../include/filter.h"
#include "../include/theremin.h"

extern system_state_t sys;

// struct for buffer entry
typedef struct {
    uint16_t value;
    uint16_t age;
} entry_t;

static entry_t buffer[15];
static uint8_t buf_size = 5; // default
static uint8_t filled = 0;

// comparison for qsort (ascending by value)
static int cmp_entry_value(const void *a, const void *b) {
    const entry_t *ea = (const entry_t*)a;
    const entry_t *eb = (const entry_t*)b;
    if (ea->value < eb->value) return -1;
    if (ea->value > eb->value) return 1;
    return 0;
}

void filter_init(void) {
    buf_size = 5;
    filled = 0;
    memset(buffer, 0, sizeof(buffer));
    sys.filter_size = buf_size;
}

void filter_set_size(uint8_t size) {
    if (size < FILTER_MIN) size = FILTER_MIN;
    if (size > FILTER_MAX) size = FILTER_MAX;
    if ((size & 1) == 0) size = size | 1; // make odd
    buf_size = size;
    sys.filter_size = buf_size;
    // reset ages
    for (uint8_t i = 0; i < 15; ++i) buffer[i].age = 0;
    filled = 0;
}

uint8_t filter_get_size(void) {
    return buf_size;
}

uint16_t filter_add_sample_and_get_median(uint16_t sample) {
    // increment ages
    for (uint8_t i = 0; i < buf_size; ++i) {
        if (buffer[i].age > 0) buffer[i].age++;
    }

    // find oldest or first free slot
    int idx_oldest = 0;
    uint16_t max_age = 0;
    for (uint8_t i = 0; i < buf_size; ++i) {
        if (buffer[i].age == 0) {
            idx_oldest = i;
            break;
        }
        if (buffer[i].age > max_age) {
            max_age = buffer[i].age;
            idx_oldest = i;
        }
    }
    // place new sample
    buffer[idx_oldest].value = sample;
    buffer[idx_oldest].age = 1;
    if (filled < buf_size) filled++;

    // prepare array copy for sorting
    entry_t temp[15];
    memcpy(temp, buffer, sizeof(temp));

    // qsort only first buf_size entries
    qsort(temp, buf_size, sizeof(entry_t), cmp_entry_value);

    // median index
    uint8_t mid = buf_size / 2;
    uint16_t median = temp[mid].value;
    return median;
}
