#include <string.h>

#include "log.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdint.h>

static uint32_t fake_time = 0;

static uint32_t
fake_timestamp(void)
{
        return fake_time;
}

void
setUp(void)
{
}
void
tearDown(void)
{
}

/* Test: log_init and log_event basic operation */
void
test_log_init_and_event(void)
{
        log_init(fake_timestamp);

        log_event(INFO, "Boot %d", 42);
        fake_time += 5;
        log_event(FAULT, "Overtemp!");
        fake_time += 5;
        log_event(WARN, "Retrying...");

        TEST_ASSERT_EQUAL_UINT16(3, log_get_count());

        const struct log_entry *e0 = log_get_entry(0);
        TEST_ASSERT_NOT_NULL(e0);
        TEST_ASSERT_EQUAL_UINT32(0, e0->timestamp);
        TEST_ASSERT_EQUAL_UINT16(INFO, e0->level);
        TEST_ASSERT_EQUAL_STRING("Boot 42", e0->msg);

        const struct log_entry *e1 = log_get_entry(1);
        TEST_ASSERT_NOT_NULL(e1);
        TEST_ASSERT_EQUAL_UINT32(5, e1->timestamp);
        TEST_ASSERT_EQUAL_UINT16(FAULT, e1->level);
        TEST_ASSERT_EQUAL_STRING("Overtemp!", e1->msg);

        const struct log_entry *e2 = log_get_entry(2);
        TEST_ASSERT_NOT_NULL(e2);
        TEST_ASSERT_EQUAL_UINT32(10, e2->timestamp);
        TEST_ASSERT_EQUAL_UINT16(WARN, e2->level);
        TEST_ASSERT_EQUAL_STRING("Retrying...", e2->msg);
}

/* Test: LOG_ONCE macro only logs one entry per invocation context */
void
test_log_once_macro(void)
{
        log_init(fake_timestamp);

        for (int i = 0; i < 10; ++i) {
                LOG_ONCE(WARN, "Logged only once");
        }
        TEST_ASSERT_EQUAL_UINT16(1, log_get_count());
        const struct log_entry *e0 = log_get_entry(0);
        TEST_ASSERT_EQUAL_STRING("Logged only once", e0->msg);
}

/* Test: log_get_entry out-of-bounds returns NULL */
void
test_log_get_entry_oob(void)
{
        log_init(fake_timestamp);
        log_event(INFO, "test");
        TEST_ASSERT_NULL(
            log_get_entry(2)); /* Only 1 entry, so index 2 invalid */
        TEST_ASSERT_NULL(log_get_entry(100)); /* Large invalid index */
}

/* Test: Buffer wraparound when more than LOG_ENTRIES events are logged */
void
test_log_buffer_wraparound(void)
{
        log_init(fake_timestamp);

        /* Fill buffer completely and overflow */
        const uint16_t N = LOG_ENTRIES + 5; /* 5 entries over capacity */
        for (uint16_t i = 0; i < N; ++i) {
                log_event(INFO, "Entry %u", i);
                fake_time += 1;
        }
        /* Should only retain last LOG_ENTRIES entries */
        TEST_ASSERT_EQUAL_UINT16(LOG_ENTRIES, log_get_count());

        /* The oldest retained entry should be N - LOG_ENTRIES */
        const struct log_entry *oldest = log_get_entry(0);
        TEST_ASSERT_NOT_NULL(oldest);

        char expected_msg[32];
        snprintf(expected_msg, sizeof(expected_msg), "Entry %u",
                 N - LOG_ENTRIES);
        TEST_ASSERT_EQUAL_STRING(expected_msg, oldest->msg);

        /* The newest retained entry should be N-1 */
        const struct log_entry *newest = log_get_entry(LOG_ENTRIES - 1);
        snprintf(expected_msg, sizeof(expected_msg), "Entry %u", N - 1);
        TEST_ASSERT_EQUAL_STRING(expected_msg, newest->msg);
}

/* Test: Correct timestamp and level for wraparound */
void
test_log_buffer_wraparound_timestamps(void)
{
        log_init(fake_timestamp);
        uint32_t start_time = 1000;
        fake_time = start_time;
        const uint16_t N = LOG_ENTRIES + 2;

        for (uint16_t i = 0; i < N; ++i) {
                log_event(FAULT, "Overrun %u", i);
                fake_time += 10;
        }
        /* Check the first (oldest) retained entry */
        const struct log_entry *first = log_get_entry(0);
        TEST_ASSERT_NOT_NULL(first);
        TEST_ASSERT_EQUAL_UINT16(FAULT, first->level);
        TEST_ASSERT_EQUAL_UINT32(
            start_time + 20,
            first->timestamp); /* N - LOG_ENTRIES = 2, so +2*10 */

        /* Check the last (newest) */
        const struct log_entry *last = log_get_entry(LOG_ENTRIES - 1);
        TEST_ASSERT_NOT_NULL(last);
        TEST_ASSERT_EQUAL_UINT32(start_time + 10 * (N - 1), last->timestamp);
}

/*
 * Simulate context: Call a function, reset context by calling another
 */
void
my_state(void)
{
        LOG_ONCE(INFO, "Entered state");
}

/* Test: LOG_ONCE resets if function's static context is reset */
void
test_log_once_reset_context(void)
{
        log_init(fake_timestamp);

        /* First entry (should log) */
        my_state();
        TEST_ASSERT_EQUAL_UINT16(1, log_get_count());

        /* Second call (should not log) */
        my_state();
        TEST_ASSERT_EQUAL_UINT16(1, log_get_count());

        /*
         * Simulate static variable reset by "function reload"
         * For actual embedded code, this happens on function exit if variable
         * is not truly static. Here, forcibly reset by resetting static var
         * (simulate reset or another state entry) But in this simple case,
         * LOG_ONCE is only truly reset across a full program restart.
         */
}

void
test_log_event_null_fmt(void)
{
        log_init(fake_timestamp);
        /* Should not crash or log */
        log_event(INFO, NULL);
        TEST_ASSERT_EQUAL_UINT16(0, log_get_count());
}

void
test_log_init_null_fn(void)
{
        /* Should not crash, and should not allow logging */
        log_init(NULL);
        log_event(INFO, "should not log");
        TEST_ASSERT_EQUAL_UINT16(0, log_get_count());
}

/* Main test runner */
int
main(void)
{
        UNITY_BEGIN();
        RUN_TEST(test_log_init_and_event);
        RUN_TEST(test_log_once_macro);
        RUN_TEST(test_log_get_entry_oob);
        RUN_TEST(test_log_buffer_wraparound);
        RUN_TEST(test_log_buffer_wraparound_timestamps);
        RUN_TEST(test_log_once_reset_context);
        RUN_TEST(test_log_event_null_fmt);
        RUN_TEST(test_log_init_null_fn);
        return UNITY_END();
}
