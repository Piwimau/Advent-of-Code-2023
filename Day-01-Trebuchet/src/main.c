#include <inttypes.h>
#include <scu/alloc.h>
#include <scu/assert.h>
#include <scu/common.h>
#include <scu/io.h>
#include <scu/string.h>
#include <stdint.h>
#include <stdlib.h>

struct Digit {

    /** @brief The token of the digit, e.g., "one" for the digit one. */
    const char* token;

    /** @brief The length of the token, e.g., 3 for the digit one. */
    int64_t length;

    /** @brief The value of the digit, e.g., 1 for the digit one. */
    int32_t value;

};

/** @brief Represents a digit. */
typedef struct Digit Digit;

/** @brief The digits to find in the second part of the puzzle. */
static const Digit DIGITS[] = {
    { .token = "one", .length = SCU_SIZEOF("one") - 1, .value = 1 },
    { .token = "two", .length = SCU_SIZEOF("two") - 1, .value = 2 },
    { .token = "three", .length = SCU_SIZEOF("three") - 1, .value = 3 },
    { .token = "four", .length = SCU_SIZEOF("four") - 1, .value = 4 },
    { .token = "five", .length = SCU_SIZEOF("five") - 1, .value = 5 },
    { .token = "six", .length = SCU_SIZEOF("six") - 1, .value = 6 },
    { .token = "seven", .length = SCU_SIZEOF("seven") - 1, .value = 7 },
    { .token = "eight", .length = SCU_SIZEOF("eight") - 1, .value = 8 },
    { .token = "nine", .length = SCU_SIZEOF("nine") - 1, .value = 9 }
};

/**
 * @brief Finds the numeric and alphanumeric calibration value in a
 * null-terminated byte string.
 *
 * @note The numeric calibration value is formed by combining the first and last
 * numeric digit (1 to 9) found in `s`. The alphanumeric calibration value is
 * formed by combining the first and last numeric or alphanumeric digit (1 to 9
 * and "one" to "nine") found in `s`.
 *
 * @warning The behavior is undefined if `s` is not a pointer to a
 * null-terminated byte string.
 *
 * @param[in]  s                     The null-terminated byte string to search
 *                                   for calibration values.
 * @param[out] calibrationValue      The numeric calibration value, or -1 on
 *                                   failure.
 * @param[out] alnumCalibrationValue The alphanumeric calibration value, or -1
 *                                   on failure.
 * @return `true` if the two calibration values were found, otherwise `false`.
 */
static inline bool find_calibration_values(
    const char* restrict s,
    int32_t* restrict calibrationValue,
    int32_t* restrict alnumCalibrationValue
) {
    SCU_ASSERT(s != nullptr);
    SCU_ASSERT(calibrationValue != nullptr);
    SCU_ASSERT(alnumCalibrationValue != nullptr);
    int32_t firstDigit = -1;
    int32_t lastDigit = -1;
    int32_t firstAlnumDigit = -1;
    int32_t lastAlnumDigit = -1;
    for (int64_t i = 0; s[i] != '\0'; i++) {
        if ((s[i] >= '1') && (s[i] <= '9')) {
            int32_t digit = s[i] - '0';
            if (firstDigit == -1) {
                firstDigit = digit;
            }
            if (firstAlnumDigit == -1) {
                firstAlnumDigit = digit;
            }
            lastDigit = digit;
            lastAlnumDigit = digit;
        }
        else {
            const Digit* digit;
            SCU_FOREACH(digit, DIGITS) {
                if (scu_strncmp(s + i, digit->token, digit->length) == 0) {
                    if (firstAlnumDigit == -1) {
                        firstAlnumDigit = digit->value;
                    }
                    lastAlnumDigit = digit->value;
                    break;
                }
            }
        }
    }
    if (firstDigit == -1) {
        *calibrationValue = -1;
        *alnumCalibrationValue = -1;
        return false;
    }
    *calibrationValue = (firstDigit * 10) + lastDigit;
    *alnumCalibrationValue = (firstAlnumDigit * 10) + lastAlnumDigit;
    return true;
}

int main() {
    SCUError error = SCU_ERROR_NONE;
    char* line = nullptr;
    int64_t size = 0;
    int32_t sumOfCalibrationValues = 0;
    int32_t sumOfAlnumCalibrationValues = 0;
    while ((error = scu_readln(&line, &size)) == SCU_ERROR_NONE) {
        int32_t calibrationValue = -1;
        int32_t alnumCalibrationValue = -1;
        bool foundCalibrationValues = find_calibration_values(
            line,
            &calibrationValue,
            &alnumCalibrationValue
        );
        if (!foundCalibrationValues) {
            // Replace the newline (if present) to avoid an ugly line break.
            int64_t newlineIndex = scu_str_index_of(line, '\n');
            if (newlineIndex != -1) {
                line[newlineIndex] = '\0';
            }
            scu_fprintf(
                SCU_STDERR,
                "The line '%s' does not contain both calibration values.\n",
                line
            );
            scu_free(line);
            return EXIT_FAILURE;
        }
        sumOfCalibrationValues += calibrationValue;
        sumOfAlnumCalibrationValues += alnumCalibrationValue;
    }
    scu_free(line);
    if (error != SCU_ERROR_END_OF_FILE) {
        scu_fprintf(
            SCU_STDERR,
            "An error occurred while reading the input file (code %d).\n",
            error
        );
        return EXIT_FAILURE;
    }
    scu_printf(
        "The sum of the numeric calibration values is %" PRId32 ".\n",
        sumOfCalibrationValues
    );
    scu_printf(
        "The sum of the alphanumeric calibration values is %" PRId32 ".\n",
        sumOfAlnumCalibrationValues
    );
    return EXIT_SUCCESS;
}