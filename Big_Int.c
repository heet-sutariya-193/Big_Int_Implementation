#include<stdio.h>
#include<stdlib.h>
#include <string.h>

#define BIT_SIZE 1024
#define CHUNK_SIZE 32
#define NUM_CHUNKS (BIT_SIZE/CHUNK_SIZE)

typedef struct bigint_tag
{
    unsigned int chunks[NUM_CHUNKS];
    int sign; // 1 for positive, -1 for negative. Zero's sign is 1.
} BigInt;

//prototypes of required functions;
void initBigInt(BigInt *b);
int all_zero(const BigInt *b); // Checks if magnitude is zero
int isValidDecimalString(char *str);
void setBigIntFromDecimal(BigInt *b, char *str);
int addBigInt(const BigInt *a, const BigInt *b, BigInt *result);
int subtractBigInt(const BigInt *a, const BigInt *b, BigInt *result);
int multiplyBigInt(const BigInt *a, const BigInt *b, BigInt *result);
void printBigIntDecimal(const BigInt *b);

// Helper prototypes
int compareAbsolute(const BigInt *a, const BigInt *b);
int _addAbsolute(const BigInt *a, const BigInt *b, BigInt *result_abs);
int _subtractAbsolute(const BigInt *a_abs, const BigInt *b_abs, BigInt *result_abs); // Assumes |a_abs| >= |b_abs|
int _multiplyAbsolute(const BigInt *a, const BigInt *b, BigInt *result_abs);


//main function
int main()
{
    BigInt num1, num2, result;
    char input_str[320]; // Increased size for sign + digits + newline + null
    char operator_char; // Renamed from operator to avoid conflict if any

    printf("Enter first number:\n");
    fgets(input_str, sizeof(input_str), stdin);
    input_str[strcspn(input_str, "\n")] = '\0';
    setBigIntFromDecimal(&num1, input_str);

    printf("Enter operation (+, -, *):\n");
    scanf(" %c", &operator_char); // Added space before %c to consume leftover newline
    while (getchar() != '\n'); // Clear rest of the input buffer

    printf("Enter second number:\n");
    fgets(input_str, sizeof(input_str), stdin);
    input_str[strcspn(input_str, "\n")] = '\0';
    setBigIntFromDecimal(&num2, input_str);

    int success = 1;
    switch (operator_char)
    {
        case '+':
            success = addBigInt(&num1, &num2, &result);
            if (success)
            {
                printf("Result of addition: ");
                printBigIntDecimal(&result);
            }
            break;

        case '-':
            success = subtractBigInt(&num1, &num2, &result);
            if (success)
            {
                printf("Result of subtraction: ");
                printBigIntDecimal(&result);
            }
            break;

        case '*':
            success = multiplyBigInt(&num1, &num2, &result);
            if (success)
            {
                printf("Result of multiplication: ");
                printBigIntDecimal(&result);
            }
            break;
        default:
            printf("Please select a valid operator.\n");
            success = 0; // Indicate failure for invalid operator
    }

    // Error messages are now printed by the respective functions.
    // if (!success && (operator_char == '+' || operator_char == '-' || operator_char == '*'))
    //     printf("Operation failed.\n"); // Generic message if needed, but specific ones are better.

    return 0;
}

//function to initialise BigInt to zero
void initBigInt(BigInt *b)
{
    for (int i = 0; i < NUM_CHUNKS; i++)
    {
        b->chunks[i] = 0;
    }
    b->sign = 1; // Default to positive (zero is also positive)
}

//an helper function to check if the Bigint magnitude is 0
int all_zero(const BigInt *b)
{
    for (int i = 0; i < NUM_CHUNKS; i++)
    {
        if (b->chunks[i] != 0)
            return 0; // Not all zero
    }
    return 1; // All zero
}

//to check whether the given string is valid (contains only digits, optionally a leading -)
int isValidDecimalString(char *str)
{
    if (str == NULL || str[0] == '\0') return 0;

    int i = 0;
    if (str[0] == '-')
    {
        if (str[1] == '\0') return 0; // String is just "-"
        i = 1;
    }
    
    if (str[i] == '\0') return 0; // String is empty after potential sign (e.g. "" or if str was just "-")


    for (; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return 0; // Invalid character
    }
    return 1;
}

/*Converts a decimal string to BigInt*/
void setBigIntFromDecimal(BigInt *b, char *str)
{
    initBigInt(b); // Initialize to 0, sign = 1

    if (!isValidDecimalString(str)) {
        printf("Error: Invalid input string '%s'. Setting to 0.\n", str);
        return; // b is already 0
    }

    int current_idx = 0;
    if (str[0] == '-') {
        b->sign = -1;
        current_idx = 1;
    } else {
        b->sign = 1;
    }
    
    // Skip leading zeros, e.g., "007" -> "7", "-000" -> "0"
    int first_digit_idx = current_idx;
    while (str[first_digit_idx] == '0' && str[first_digit_idx + 1] != '\0') {
        first_digit_idx++;
    }

    // Check if the number is effectively zero (e.g., "0", "00", "-0", "-000")
    // After skipping leading zeros, if current char is '0' and it's the last char.
    if (str[first_digit_idx] == '0' && str[first_digit_idx + 1] == '\0') {
        initBigInt(b); // Magnitude 0, sign 1
        return;
    }
    
    int num_len = strlen(str + first_digit_idx);
    if (num_len > 309) { // Max 309 digits for 1024 bits
        printf("Error: Input number '%s' too large (exceeds 1024 bits). Setting to 0.\n", str);
        initBigInt(b); 
        return;
    }
    if (num_len == 0) { // Should be caught by isValidDecimalString or zero check, but as safeguard
        initBigInt(b);
        return;
    }

    // Process digits from first_digit_idx
    for (int i = first_digit_idx; str[i] != '\0'; i++) {
        // Multiply b by 10 (in place)
        unsigned long long carry_mult = 0;
        for (int j = NUM_CHUNKS - 1; j >= 0; j--) {
            unsigned long long product = (unsigned long long)b->chunks[j] * 10 + carry_mult;
            b->chunks[j] = product & 0xFFFFFFFF;
            carry_mult = product >> 32;
        }
        if (carry_mult > 0) { // Overflow during multiplication by 10
            printf("Error: Overflow during string to BigInt conversion (multiply by 10) for '%s'. Setting to 0.\n", str);
            initBigInt(b);
            return;
        }

        int digit = str[i] - '0';
        // Add digit to b (in place)
        unsigned long long carry_add = digit;
        for (int j = NUM_CHUNKS - 1; j >= 0 && carry_add > 0; j--) {
            unsigned long long sum = (unsigned long long)b->chunks[j] + carry_add;
            b->chunks[j] = sum & 0xFFFFFFFF;
            carry_add = sum >> 32;
        }
        if (carry_add > 0) { // Overflow during addition of digit
             printf("Error: Overflow during string to BigInt conversion (add digit) for '%s'. Setting to 0.\n", str);
            initBigInt(b);
            return;
        }
    }
    
    // Final check: if the result is 0 (e.g. if input was "-0"), ensure sign is 1.
    if (all_zero(b)) {
        b->sign = 1;
    }
}

// Compares absolute values of a and b
// Returns: 0 if |a| == |b|, 1 if |a| > |b|, -1 if |a| < |b|
int compareAbsolute(const BigInt *a, const BigInt *b) {
    for (int i = 0; i < NUM_CHUNKS; i++) { // MSB to LSB
        if (a->chunks[i] > b->chunks[i]) return 1;
        if (a->chunks[i] < b->chunks[i]) return -1;
    }
    return 0; // Equal
}

// Adds absolute values: result_abs = |a| + |b|.
// Assumes result_abs is initialized (e.g. by initBigInt).
// Returns 1 on success, 0 on overflow.
int _addAbsolute(const BigInt *a, const BigInt *b, BigInt *result_abs) {
    unsigned long long carry = 0;
    for (int i = NUM_CHUNKS - 1; i >= 0; i--) { // LSB to MSB
        unsigned long long sum = (unsigned long long)a->chunks[i] + b->chunks[i] + carry;
        result_abs->chunks[i] = sum & 0xFFFFFFFF;
        carry = sum >> 32;
    }
    if (carry != 0) return 0; // Overflow
    return 1;
}

// Subtracts absolute values: result_abs = |a_abs| - |b_abs|.
// PRECONDITION: |a_abs| >= |b_abs|.
// Assumes result_abs is initialized.
// Returns 1 on success, 0 on error (e.g. precondition violation).
int _subtractAbsolute(const BigInt *a_abs, const BigInt *b_abs, BigInt *result_abs) {
    unsigned long long borrow = 0;
    for (int i = NUM_CHUNKS - 1; i >= 0; i--) { // LSB to MSB
        unsigned long long minuend_chunk = (unsigned long long)a_abs->chunks[i];
        unsigned long long subtrahend_chunk = (unsigned long long)b_abs->chunks[i] + borrow;

        if (minuend_chunk >= subtrahend_chunk) {
            result_abs->chunks[i] = minuend_chunk - subtrahend_chunk;
            borrow = 0;
        } else {
            result_abs->chunks[i] = (0x100000000ULL + minuend_chunk) - subtrahend_chunk;
            borrow = 1;
        }
    }
    if (borrow != 0) return 0; // Error: underflow, means |a_abs| < |b_abs|, precondition violated
    return 1;
}

//adding two BigInt numbers
int addBigInt(const BigInt *a, const BigInt *b, BigInt *result)
{
    initBigInt(result);
    int success;

    if (a->sign == b->sign) {
        // (+A) + (+B) = +(|A|+|B|) OR (-A) + (-B) = -(|A|+|B|)
        success = _addAbsolute(a, b, result);
        if (!success) {
            printf("Error: Addition overflow.\n");
            initBigInt(result); // Clear result on error
            return 0;
        }
        result->sign = a->sign;
    } else {
        // Signs differ: effectively subtraction. e.g. (+A) + (-B) = A - B
        int cmp = compareAbsolute(a, b);
        if (cmp == 0) {
            // A + (-A) = 0. Result is already 0, sign 1 by initBigInt.
            success = 1;
        } else if (cmp > 0) { // |a| > |b|
            success = _subtractAbsolute(a, b, result); // result = |a| - |b|
            if (!success) { // Should not happen if cmp > 0
                printf("Error: Internal error in subtraction logic (addBigInt).\n");
                initBigInt(result);
                return 0;
            }
            result->sign = a->sign; // Sign of the one with larger magnitude
        } else { // |b| > |a|
            success = _subtractAbsolute(b, a, result); // result = |b| - |a|
             if (!success) { // Should not happen if cmp < 0
                printf("Error: Internal error in subtraction logic (addBigInt).\n");
                initBigInt(result);
                return 0;
            }
            result->sign = b->sign; // Sign of the one with larger magnitude
        }
    }
    
    // Ensure 0 has positive sign
    if (all_zero(result)) {
        result->sign = 1;
    }
    return success;
}

int subtractBigInt(const BigInt *a, const BigInt *b, BigInt *result) 
{
    // Implement a - b as a + (-b)
    BigInt neg_b = *b; // Make a copy to change sign
    if (!all_zero(&neg_b)) { // Don't flip sign of 0
        neg_b.sign *= -1;
    }
    // The addBigInt function will print specific error message if any.
    return addBigInt(a, &neg_b, result);
}

// Multiplies absolute values: result_abs = |a| * |b|.
// Assumes result_abs is initialized (all chunks zeroed).
// Returns 1 on success, 0 on overflow.
int _multiplyAbsolute(const BigInt *a, const BigInt *b, BigInt *result_abs) {
    if (all_zero(a) || all_zero(b)) {
        // initBigInt(result_abs) was already called by multiplyBigInt, so it's 0.
        return 1;
    }

    // Standard grade-school multiplication
    // Iterate through chunks of 'a' (as multiplicand) from LSB representation
    for (int j_lpc = 0; j_lpc < NUM_CHUNKS; ++j_lpc) { // j_lpc is "Least-significant-chunk-based Position" for 'a'
        unsigned int a_chunk = a->chunks[NUM_CHUNKS - 1 - j_lpc];
        if (a_chunk == 0) continue; // Optimization

        unsigned long long carry = 0;
        // Iterate through chunks of 'b' (as multiplier) from LSB representation
        for (int i_lpc = 0; i_lpc < NUM_CHUNKS; ++i_lpc) { // i_lpc for 'b'
            int k_lpc = j_lpc + i_lpc; // LSB-based position in result_abs

            if (k_lpc >= NUM_CHUNKS) { // Product term would fall outside MSB of result array
                // If there's any non-zero value to write here, it's an overflow
                if (carry > 0 || (unsigned long long)a_chunk * b->chunks[NUM_CHUNKS - 1 - i_lpc] > 0) {
                    return 0; // Overflow
                }
                // If all remaining products and carry are zero, we can break (optimization)
                // Check if remaining b chunks are zero
                int b_remaining_zero = 1;
                for(int rem_idx = i_lpc; rem_idx < NUM_CHUNKS; ++rem_idx) {
                    if (b->chunks[NUM_CHUNKS - 1 - rem_idx] != 0) {
                        b_remaining_zero = 0;
                        break;
                    }
                }
                if (carry == 0 && b_remaining_zero) break; // Optimization: if carry is 0 and rest of b is 0 for this a_chunk
                // else continue to check for overflow from remaining terms (even if a_chunk * b_chunk is 0, carry might make it non-zero)
            }
            
            if (k_lpc < NUM_CHUNKS) { // Only process if target chunk is within bounds
                int k_msb = NUM_CHUNKS - 1 - k_lpc; // Convert LSB-based index to MSB-based index for our array

                unsigned long long product = (unsigned long long)a_chunk * b->chunks[NUM_CHUNKS - 1 - i_lpc];
                product += result_abs->chunks[k_msb]; // Add previous content of this result chunk
                product += carry;                     // Add carry from the previous (inner loop) iteration

                result_abs->chunks[k_msb] = product & 0xFFFFFFFF;
                carry = product >> 32;
            }
        }

        // After iterating all chunks of 'b' for a_chunk:
        // Propagate remaining carry to more significant chunks of result_abs
        int k_lpc_carry_prop = j_lpc + NUM_CHUNKS; // Position for this carry (one beyond last b_chunk product for this a_chunk)
        while (carry > 0) {
            if (k_lpc_carry_prop >= NUM_CHUNKS) {
                return 0; // Overflow: carry propagates out of MSB bound
            }
            int k_msb_carry_prop = NUM_CHUNKS - 1 - k_lpc_carry_prop;
            
            unsigned long long sum_carry = (unsigned long long)result_abs->chunks[k_msb_carry_prop] + carry;
            result_abs->chunks[k_msb_carry_prop] = sum_carry & 0xFFFFFFFF;
            carry = sum_carry >> 32;
            k_lpc_carry_prop++;
        }
    }
    return 1;
}


//funtion for the multiplication operation
int multiplyBigInt(const BigInt *a, const BigInt *b, BigInt *result) 
{
    initBigInt(result); // Initialize result to 0, sign = 1

    if (!_multiplyAbsolute(a, b, result)) {
        printf("Error: Multiplication overflow.\n");
        initBigInt(result); // Clear result on error
        return 0;
    }

    // Determine sign of the result
    if (all_zero(result)) { // If result is 0, sign is 1 (already set by initBigInt)
        result->sign = 1;
    } else if (a->sign == b->sign) {
        result->sign = 1; // (+)*(+) = (+) or (-)*(-) = (+)
    } else {
        result->sign = -1; // (+)*(-) = (-) or (-)*(+) = (-)
    }
    
    return 1;
}

//function which will print the results after operations or to show the Bigint type numbers
void printBigIntDecimal(const BigInt *b) 
{
    if (all_zero(b)) 
    {
        printf("0\n");
        return;
    }

    if (b->sign == -1)
    {
        printf("-");
    }

    char decimal_str[320] = {0}; // Buffer for decimal string representation
    BigInt temp = *b;            // Make a copy to modify (for division by 10)
    temp.sign = 1;               // Work with absolute value for printing magnitude
    int idx = 0;

    do 
    {
        unsigned int remainder_carry = 0; // This will be the digit (0-9)
        // Perform division by 10 on temp (absolute value)
        for (int i = 0; i < NUM_CHUNKS; i++) // From MSB to LSB
        {
            unsigned long long current_chunk_val = ((unsigned long long)remainder_carry << 32) + temp.chunks[i];
            temp.chunks[i] = current_chunk_val / 10;
            remainder_carry = current_chunk_val % 10;
        }
        decimal_str[idx++] = remainder_carry + '0'; // Store remainder as char
    } while (!all_zero(&temp));

    // Reverse the string of digits
    for (int i = 0; i < idx / 2; i++) 
    {
        char tmp_char = decimal_str[i];
        decimal_str[i] = decimal_str[idx - 1 - i];
        decimal_str[idx - 1 - i] = tmp_char;
    }
    // decimal_str[idx] is already '\0' due to array initialization with {0}

    printf("%s\n", decimal_str);
}