/*
 * @licence MIT
 *
 * @file: embedded_log.h
 *
 * @brief
 *   Lightweight, MISRA C-compliant RAM log buffer for embedded systems.
 *
 *   This module provides a minimal, robust API for event and fault logging
 *   on microcontrollers and safety-critical firmware. Logs are stored in a
 *   circular buffer in RAM and can be inspected with a debugger or dumped
 *   at runtime if desired.
 *
 *   Features:
 *   - Ring buffer for formatted string log entries
 *   - Levels: INFO, WARN, FAULT
 *   - Defensive: NULL pointer safe
 *   - One-shot logging macro (LOG_ONCE)
 *   - No dynamic memory, no heap, no OS dependency
 *   - Designed for integration as a Meson subproject
 *
 *   Typical use case: debugging or post-mortem analysis in systems without
 *   printf/serial output.
 */
#ifndef EMBEDDED_LOG_H
#define EMBEDDED_LOG_H

#include <stdint.h>

/**
 * @defgroup embedded_log_api Embedded Logging Facility
 * @brief Circular in-RAM log buffer for event and fault logging.
 *
 * @{
 */

#define LOG_MSG_LEN (48u)
#define LOG_ENTRIES (128u)

/**
 * @brief Log level enum.
 */
enum log_level {
        INFO = 0,
        WARN = 1,
        FAULT = 2
};

/**
 * @brief Log entry structure.
 */
struct log_entry {
        uint32_t timestamp;
        uint16_t level;
        char msg[LOG_MSG_LEN];
};

/**
 * @brief Initialize the log system.
 *
 * @param timestamp_fn  Pointer to function returning milliseconds.
 */
void log_init(uint32_t (*timestamp_fn)(void));

/**
 * @brief Add an event to the log.
 *
 * @param level     Log level (info, warn, fault).
 * @param fmt       printf-style format string.
 * @param ...       Arguments for format string.
 */
void log_event(enum log_level level, const char *fmt, ...);

/**
 * @brief Get the number of valid entries in the log.
 *
 * @return          Count of entries.
 */
uint16_t log_get_count(void);

/**
 * @brief Get a pointer to the idx-th oldest log entry.
 *
 * @param idx       Entry index (0 = oldest).
 * @return          Pointer to log_entry, or NULL if out of bounds.
 */
const struct log_entry *log_get_entry(uint16_t idx);

/**
 * @def LOG_ONCE
 * @brief Log a message at most once per code location per reset.
 *
 * This macro ensures that a log statement is only recorded once for the
 * lifetime of the program or until the enclosing function is re-entered with a
 * reset static context. It is useful for suppressing repeated logs from within
 * frequently called code paths, such as state machine tick functions.
 *
 * Example usage:
 * @code
 * void state_run(void)
 * {
 *     LOG_ONCE(WARN, "Waiting for module ready...");
 *     // ... rest of state logic ...
 *     // To re-enable the log, reset the function's static context as needed.
 * }
 * @endcode
 *
 * @param level     Log level (enum log_level).
 * @param fmt       printf-style format string.
 * @param ...       Arguments for format string.
 */
#define LOG_ONCE(level, fmt, ...)                                              \
        do {                                                                   \
                static uint16_t _logged = 0;                                   \
                if (!_logged) {                                                \
                        log_event(level, fmt, ##__VA_ARGS__);                  \
                        _logged = 1;                                           \
                }                                                              \
        } while (0)

/**
 * Close group: embedded_log_api
 * @}
 */

#endif /* EMBEDDED_LOG_H */
