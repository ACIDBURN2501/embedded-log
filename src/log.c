#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../include/log.h"

static struct log_entry log_buffer[LOG_ENTRIES];
static uint16_t log_head = 0u;
static uint16_t log_count = 0u;
static uint32_t (*get_timestamp)(void) = NULL;

void
log_init(uint32_t (*timestamp_fn)(void))
{
        log_head = 0U;
        log_count = 0U;
        (void)memset((void *)log_buffer, 0, sizeof(log_buffer));
        get_timestamp = timestamp_fn;
}

void
log_event(enum log_level level, const char *fmt, ...)
{
        if ((get_timestamp == NULL) || (fmt == NULL)) {
                return;
        }

        struct log_entry *entry = &log_buffer[log_head];
        entry->timestamp = get_timestamp();
        entry->level = (uint16_t)level;

        va_list args;
        va_start(args, fmt);
        (void)vsnprintf(entry->msg, (size_t)LOG_MSG_LEN, fmt, args);
        va_end(args);

        /* overwrite last byte with null terminator */
        entry->msg[LOG_MSG_LEN - 1] = '\0';

        log_head++;
        if (log_head >= LOG_ENTRIES) {
                log_head = 0U;
        }
        if (log_count < LOG_ENTRIES) {
                log_count++;
        }
}

uint16_t
log_get_count(void)
{
        return log_count;
}

/*
 * Return a pointer to the Nth oldest log entry (0 = oldest, log_count-1 =
 * newest)
 */
const struct log_entry *
log_get_entry(uint16_t idx)
{
        if (idx >= log_count) {
                return NULL;
        }

        uint16_t phys_idx =
            (uint16_t)((log_head + LOG_ENTRIES - log_count + idx)
                       % LOG_ENTRIES);

        return &log_buffer[phys_idx];
}

const struct log_entry *
log_get_buffer(uint16_t *count)
{
        if (count != NULL) {
                *count = log_count;
        }

        return log_buffer;
}
