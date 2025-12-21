#include <inttypes.h>
#include <scu/alloc.h>
#include <scu/assert.h>
#include <scu/common.h>
#include <scu/hash-set.h>
#include <scu/io.h>
#include <scu/list.h>
#include <scu/string.h>
#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>

/** @brief Represents a scratchcard. */
typedef struct Card {

    /** @brief The number of winning numbers on the card. */
    int32_t winningNumbers;

} Card;

/**
 * @brief Parses a card from a line of text.
 *
 * The line must have the format "Card &lt;number&gt;: &lt;winning numbers&gt; |
 * &lt;scratched numbers&gt;", where &lt;number&gt; is the number of the card.
 * Although the card number is mostly ignored, it must still be present in the
 * line and be a positive decimal integer. The &lt;winning numbers&gt; and
 * &lt;scratched numbers&gt; must be lists of positive decimal integers, where
 * each number is surrounded by one or more spaces.
 *
 * An example for a valid line of text might be "Card 1: 41 48 83 86 17 | 83 86
 * 6 31 17 9 48 53".
 *
 * @warning The behavior is undefined if `line` is not a pointer to a
 * null-terminated byte string.
 *
 * @param[in]  line The line to parse the card from.
 * @param[out] card The parsed card.
 * @return `true` if a card was successfully parsed, otherwise `false`.
 */
static bool card_parse(const char* restrict line, Card* restrict card) {
    SCU_ASSERT(line != nullptr);
    SCU_ASSERT(card != nullptr);
    int32_t number;
    int64_t read;
    if (scu_sscanf(line, "Card %" SCNd32 ": %lln", &number, &read) != 1) {
        return false;
    }
    line += read;
    SCUHashSet* winningNumbers = scu_hash_set_new(
        SCU_SIZEOF(int32_t),
        scu_hash_int32,
        scu_equal_int32
    );
    while (scu_sscanf(line, "%" SCNd32 "%lln", &number, &read) == 1) {
        scu_hash_set_add(winningNumbers, number);
        line += read;
    }
    if (scu_sscanf(line, " | %lln", &read) != 0) {
        scu_hash_set_free(winningNumbers);
        return false;
    }
    line += read;
    card->winningNumbers = 0;
    while (scu_sscanf(line, "%" SCNd32 "%lln", &number, &read) == 1) {
        if (scu_hash_set_contains(winningNumbers, number)) {
            card->winningNumbers++;
        }
        line += read;
    }
    scu_hash_set_free(winningNumbers);
    return true;
}

/**
 * @brief Returns the total number of points won by a specified list of cards.
 *
 * @param[in] cards The list of cards to evaluate.
 * @return The total number of points won by the specified list of cards.
 */
static int32_t total_points(const Card* cards) {
    SCU_ASSERT(cards != nullptr);
    int32_t totalPoints = 0;
    const Card* card;
    SCU_LIST_FOREACH(card, cards) {
        totalPoints += (int32_t) pow(2, card->winningNumbers - 1);
    }
    return totalPoints;
}

/**
 * @brief Returns the total number of cards won by a specified list of cards.
 *
 * @param[in] cards The list of cards to evaluate.
 * @return The total number of cards won by the specified list of cards.
 */
static int32_t total_cards(const Card* cards) {
    SCU_ASSERT(cards != nullptr);
    int64_t count = scu_list_count(cards);
    int32_t* cardCounts = scu_list_new_with_capacity(
        SCU_SIZEOF(int32_t),
        count
    );
    for (int64_t i = 0; i < count; i++) {
        scu_list_add(cardCounts, (int32_t) { 1 });
    }
    for (int64_t i = 0; i < count; i++) {
        for (int64_t j = 0; j < cards[i].winningNumbers; j++) {
            cardCounts[i + 1 + j] += cardCounts[i];
        }
    }
    int32_t totalCards = 0;
    int32_t* cardCount;
    SCU_LIST_FOREACH(cardCount, cardCounts) {
        totalCards += *cardCount;
    }
    scu_list_free(cardCounts);
    return totalCards;
}

int main() {
    SCUError error = SCU_ERROR_NONE;
    char* line = nullptr;
    int64_t size = 0;
    Card* cards = scu_list_new(SCU_SIZEOF(Card));
    while ((error = scu_readln(&line, &size)) == SCU_ERROR_NONE) {
        // Replace the newline (if present) to simplify parsing and avoid an
        // ugly line break if an error occurs.
        int64_t newlineIndex = scu_str_index_of(line, '\n');
        if (newlineIndex != -1) {
            line[newlineIndex] = '\0';
        }
        Card card;
        if (!card_parse(line, &card)) {
            scu_fprintf(
                SCU_STDERR,
                "The line '%s' does not represent a valid card.\n",
                line
            );
            scu_list_free(cards);
            scu_free(line);
            return EXIT_FAILURE;
        }
        scu_list_add(cards, card);
    }
    scu_free(line);
    if (error != SCU_ERROR_END_OF_FILE) {
        scu_fprintf(
            SCU_STDERR,
            "An error occurred while reading the input file (code %d).\n",
            error
        );
        scu_list_free(cards);
        return EXIT_FAILURE;
    }
    int32_t totalPoints = total_points(cards);
    int32_t totalCards = total_cards(cards);
    scu_printf("The total number of points is %" PRId32 ".\n", totalPoints);
    scu_printf("The total number of cards is %" PRId32 ".\n", totalCards);
    scu_list_free(cards);
    return EXIT_SUCCESS;
}