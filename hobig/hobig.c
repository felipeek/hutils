typedef int bool;
typedef unsigned char u8;
typedef unsigned long long int u64;
typedef long long int s64;
#define true 1
#define false 0

#include <assert.h>
#include "../light_array.h"
#include "table.h"

#define printf(...) printf(__VA_ARGS__); fflush(stdout)

typedef struct {
    int  negative;
    u64* value;    // dynamic light array
} HoBigInt;

HoBigInt 
hobig_int_new(s64 v) {
    HoBigInt result = { 0, array_new(u64) };
    if(v < 0) {
        result.negative = 1;
        array_push(result.value, -v);
    } else {
        array_push(result.value, v);
    }
    return result;
}

void
hobig_free(HoBigInt v) {
    if(v.value) array_free(v.value);
}

static void 
print_number(unsigned char* num, int length) {
    bool seen_number = false;
    for(int i = length-1; i >= 0; --i) {
        if(num[i] == 0 && !seen_number)
            continue;
        else
            seen_number = true;
        printf("%d", num[i]);
    }
    if(!seen_number) printf("0");
}

// a will be the destination
static void 
big_dec_sum(u8* dst, u8* src, int length) {
    u8 carry = 0;
    for(int i = 0; i < length; ++i) {
        u8 a = dst[i];
        u8 b = src[i];
        dst[i] = mult_table[a][b][carry][0]; // this is the result
        carry = mult_table[a][b][carry][1];
    }
}

void 
hobig_int_print(HoBigInt n) {
    if(!n.value || array_length(n.value) == 0){
        printf("0");
        return;
    }

    if(n.negative) {
        printf("-");
    }
    size_t length = (array_length(n.value) + 1) * 64;

    u8* result = calloc(length, 1); // The result is at least the size of the binary number + 1
    u8* buffer = calloc(length, 1); // Buffer that will hold powers of 2 in decimal
    buffer[0] = 1;

    for(int k = 0; k < array_length(n.value); k++) {
        u64 v = n.value[k];
        for(int i = 0; i < sizeof(*n.value) * 8; ++i) {
            int bit = (v >> i) & 1;
            if(bit) {
                big_dec_sum(result, buffer, length);
            }
            big_dec_sum(buffer, buffer, length);
        }
    }

    print_number(result, length);

    free(result);
    free(buffer);
}

static void 
big_gen_pow2(int p, u8* buffer, int length) {
    if(length == 0) return;
    buffer[0] = 1;
    for(int i = 0; i < p; ++i) {
        u8 carry = 0;
        for(int i = 0; i < length-1; ++i) {
            u8 a = buffer[i];
            u8 b = buffer[i];
            buffer[i] = mult_table[a][b][carry][0]; // this is the result
            carry = mult_table[a][b][carry][1];
        }
    }
}

HoBigInt
hobig_int_copy(HoBigInt v) {
    HoBigInt result = {0};
    result.negative = v.negative;
    result.value = array_copy(v.value);
    return result;
}

static void
multiply_by_pow2(HoBigInt* n, int power) {
    int word_size_bytes = sizeof(*n->value);
    int word_size_bits = word_size_bytes * 8;
    int word_shift_count = power / word_size_bits;
    int shift_amount = (power % word_size_bits);

    u64 s = 0;
    for(int i = 0; i < array_length(n->value); ++i) {
        u64 d = n->value[i] >> (word_size_bits - shift_amount);
        n->value[i] <<= shift_amount;
        n->value[i] |= s;
        s = d;
    }
    if(s) {
        // grow array
        array_push(n->value, s);
    }

    if(word_shift_count) {
        // insert zeros at the beggining
        array_allocate(n->value, word_shift_count);
        memcpy(n->value + array_length(n->value), n->value, array_length(n->value) * word_size_bytes);
        memset(n->value, 0, word_shift_count * word_size_bytes);
        array_length(n->value) += word_shift_count;
    }
}

// Compares two numbers considering sign
// Return value:
//  1 -> left is bigger
//  0 -> they are equal
// -1 -> right is bigger
int
hobig_int_compare_signed(HoBigInt* left, HoBigInt* right) {
    // Check the sign first
    if(left->negative != right->negative) {
        if(left->negative) return -1;
        return 1;
    }

    // If both are negative, the biggest absolute will be
    // the lower value.
    int negative = (left->negative && right->negative) ? -1 : 1;

    size_t llen = array_length(left->value);
    size_t rlen = array_length(right->value);

    if(llen > rlen) return 1 * negative;
    if(llen < rlen) return -1 * negative;

    for(int i = llen - 1;; --i) {
        if(left->value[i] > right->value[i]) {
            return 1 * negative;
        } else if (left->value[i] < right->value[i]) {
            return -1 * negative;
        }
        if(i == 0) break;
    }

    return 0;
}

// Compares the absolute value of two numbers, ignoring the sign
// Return value:
//  1 -> left is bigger
//  0 -> they are equal
// -1 -> right is bigger
int
hobig_int_compare_absolute(HoBigInt* left, HoBigInt* right) {
    if(left->value == right->value)
        return 0;

    size_t llen = array_length(left->value);
    size_t rlen = array_length(right->value);

    if(llen > rlen) return 1;
    if(llen < rlen) return -1;


    for(int i = llen - 1;; --i) {
        if(left->value[i] > right->value[i]) {
            return 1;
        } else if (left->value[i] < right->value[i]) {
            return -1;
        }
        if(i == 0) break;
    }

    return 0;
}

void
hobig_int_sub(HoBigInt* dst, HoBigInt* src);

void 
hobig_int_add(HoBigInt* dst, HoBigInt* src) {
    // Check to see if a subtraction is preferred
    if(dst->negative != src->negative) {
        // Subtract instead
        if(dst->negative) {
            // -x + y => -x -(-y)
            src->negative = 1;
            hobig_int_sub(dst, src);
            src->negative = 0;
        } else {
            // x + (-y) => x - y
            src->negative = 0;
            hobig_int_sub(dst, src);
            src->negative = 1;
        }
        return;
    }

    HoBigInt s = {0};
    int free_source = 0;
    if(dst == src) {
        s = hobig_int_copy(*src);
        src = &s;
        free_source = 1;
    }

    // destination is at least the size of src or bigger
    if(array_length(dst->value) < array_length(src->value)) {
        size_t count = array_length(src->value) - array_length(dst->value);
        array_allocate(dst->value, count);
        memset(dst->value + array_length(dst->value), 0, count * sizeof(*dst->value));
        array_length(dst->value) = array_length(src->value);
    }
    u64 carry = 0;
    for(int i = 0; i < array_length(src->value); ++i) {
        u64 sum = dst->value[i] + src->value[i] + carry;
        if(sum < src->value[i]) {
            carry = 1;
        } else {
            carry = 0;
        }
        dst->value[i] = sum;
    }
    if(carry) {
        if(array_length(src->value) == array_length(dst->value)) {
            // grow destination
            array_push(dst->value, 1);
        } else {
            // destination is bigger, so sum 1 to it
            if(dst->value[array_length(src->value)] != (u64)-1) {
                dst->value[array_length(src->value)] += 1;
            } else {
                HoBigInt big_one = hobig_int_new(1);
                hobig_int_add(dst, &big_one);
            }
        }
    }

    if(free_source) {
        hobig_free(*src);
    }
}

void
hobig_int_sub(HoBigInt* dst, HoBigInt* src) {
    int comparison = hobig_int_compare_absolute(dst, src);

    int dst_sign = dst->negative;
    int src_sign = src->negative;

    if(dst->negative != src->negative) {
        assert(comparison != 0);
        // if different sign and dst > src perform an absolute sum
        dst->negative = 0;
        src->negative = 0;
        
        hobig_int_add(dst, src);
        // final sign is going to be the destination sign, since its absolute
        // value is bigger.
        dst->negative = dst_sign;

        // restore src
        src->negative = src_sign;
        return;
    }

    switch(comparison) {
        case 0: {
            // Result is 0
            dst->negative = 0;
            *dst->value = 0;
            array_length(dst->value) = 1;
        } break;
        case 1: {
            // dst > src
            u64 borrow = 0;
            for(int i = 0; i < array_length(src->value); ++i) {
                u64 start = dst->value[i];
                dst->value[i] -= borrow;
                dst->value[i] -= (src->value[i]);
                if(dst->value[i] > start) {
                    borrow = 1;
                } else {
                    borrow = 0;
                }
            }
            if(borrow) {
                dst->value[array_length(src->value)] -= 1;
            }
        } break;
        case -1: {
            // dst < src
            dst->negative = (dst->negative) ? 0 : 1;
            u64 borrow = 0;
            for(int i = 0; i < array_length(src->value); ++i) {
                u64 start = src->value[i];
                dst->value[i] = src->value[i] - borrow - dst->value[i];
                if(dst->value[i] > start) {
                    borrow = 1;
                } else {
                    borrow = 0;
                }
            }
            assert(borrow == 0);
        } break;
        default: assert(0); break;
    }
    // Reduce the array size of destination if it is the case
    size_t dst_length = array_length(dst->value);
    size_t reduction = 0;
    for(size_t i = dst_length - 1;;--i) {
        if(dst->value[i] == 0)
            reduction++;
        else
            break;
        if(i == 0) break;
    }
    array_length(dst->value) -= reduction;
}

void 
hobig_int_mul_pow10(HoBigInt* start, int p) {
    if(p == 0) {
        return;
    }
    // 8x + 2x
    for(int i = 0; i < p; ++i) {
        HoBigInt copy2 = hobig_int_copy(*start);

        multiply_by_pow2(start, 3);     // multiply by 8
        multiply_by_pow2(&copy2, 1);    // multiply by 2

        hobig_int_add(start, &copy2);   // sum the result

        array_free(copy2.value);        // free temporary
    }
}

// Multiplication works by multiplying the powers of 2 i.e.: for 777
// 777 * x => 512 * x + 256 * x + 8 * x + 1 * x
void 
hobig_int_mul(HoBigInt* dst, HoBigInt* src) {
    int free_source = 0;
    HoBigInt s = {0};
    if(dst == src) {
        s = hobig_int_copy(*src);
        src = &s;
        free_source = 1;
    }

    dst->negative = (dst->negative ^ src->negative);

    int word_size = sizeof(u64) * 8;
    HoBigInt dst_copy = hobig_int_copy(*dst);
    memset(dst->value, 0, array_length(dst->value) * sizeof(*dst->value));
    
    for(int k = 0; k < array_length(src->value); k++) {
        u64 v = src->value[k];
        for(int i = 0; i < word_size; ++i) {
            int bit = (v >> i) & 1;
            if(bit) {
                hobig_int_add(dst, &dst_copy);
            }
            multiply_by_pow2(&dst_copy, 1);
        }
    }
    hobig_free(dst_copy);

    if(free_source) {
        hobig_free(*src);
    }
}

HoBigInt 
hobig_new_dec(const char* number, unsigned int* error) {
    HoBigInt result = {0};
    if(error) *error = 0;

    int len = strlen(number);
    if(len == 0) {
        if(error) *error = 1;
        return result;
    }

    int sign = 0;
    int index = 0;
    if(number[index] == '-') {
        sign = 1;
        index++;
    }

    int first = number[len - 1] - 0x30;
    if(first < 0 || first > 9) {
        if(error) *error = 1;
        return result;
    }
    result.value = array_new(u64);
    array_push(result.value, first);

    // All digits to be used in multiplication
    HoBigInt digits[10] = { 0 };
    for(int i = 1; i < 10; ++i) {
        digits[i] = hobig_int_new(i);
    }

    // Powers of ten that will be used for every step
    HoBigInt powers_of_ten = hobig_int_new(1);

    for(int i = len - 2; i >= index; --i) {
        int n = number[i] - 0x30;
        if(n < 0 || n > 9) {
            // Not a decimal number
            if(error) *error = 1;
            break;
        }

        // Calculate n * 10^power
        // Start at 10^1

        // Generate the power of 10
        hobig_int_mul_pow10(&powers_of_ten, 1);

        // When the digit is 0, we still advance the powers of 10
        // but we do not attempt to sum up anything to the number
        if(n == 0) continue;

        // Grab a copy to be used to multiply by the digit value
        HoBigInt pow10val = hobig_int_copy(powers_of_ten);

        // Multiply by the digit value n
        hobig_int_mul(&pow10val, &digits[n]);

        // Sum it back to the final result
        hobig_int_add(&result, &pow10val);

        // Free temporary
        hobig_free(pow10val);
    }

    // Free digits
    for(int i = 1; i < 10; ++i) {
        hobig_free(digits[i]);
    }
    // Free suppor power of 10
    hobig_free(powers_of_ten);

    if(error && *error) {
        hobig_free(result);
    }

    result.negative = sign; // do it only now, to leave the sums positive

    return result;
}

typedef struct {
    HoBigInt quotient;
    HoBigInt remainder;
} HoBigInt_DivResult;

HoBigInt_DivResult
hobig_int_div(HoBigInt* dividend, HoBigInt* divisor) {
    HoBigInt_DivResult result = { 0 };

    if(*divisor->value == 0) {
        // Division by 0
        assert(0);
    }

    int comparison = hobig_int_compare_absolute(dividend, divisor);

    if(comparison == -1) {
        // Dividend is smaller than divisor, quotient = 0 and remainder = dividend
        result.quotient = hobig_int_new(0);
        result.remainder = hobig_int_copy(*dividend);
    } else if(comparison == 0) {
        // Both numbers are equal, quotient is 1 and remainder is 0
        result.quotient = hobig_int_new(1);
        result.remainder = hobig_int_new(0);
    } else {
        // Perform long division since dividend > divisor
        // 100101010 | 111101
        HoBigInt remainder = hobig_int_new(0);
        HoBigInt quotient = hobig_int_new(0);
        HoBigInt one = hobig_int_new(1);

        for(int k = array_length(dividend->value) - 1 ;; --k) {
            u64 v = dividend->value[k];
            for(int i = sizeof(*dividend->value) * 8 - 1; i >= 0; --i) {
                multiply_by_pow2(&quotient, 1);
                multiply_by_pow2(&remainder, 1);
                int bit = (v >> i) & 1;
                if(bit) {
                    hobig_int_add(&remainder, &one);
                }
                int comparison = hobig_int_compare_absolute(&remainder, divisor);
                if(comparison == 1) {
                    // Ready to divide, quotient receives one
                    // and divisor is subtracted from remainder 
                    hobig_int_add(&quotient, &one);
                    hobig_int_sub(&remainder, divisor);
                } else if(comparison == 0) {
                    // Division is 1 and remainder 0
                    *remainder.value = 0;
                    array_length(remainder.value) = 1;
                    hobig_int_add(&quotient, &one);
                } else {
                    // Still not ready to divide
                    // Put a zero in the quotient
                }
            }
            if(k == 0) break;
        }
        result.quotient = quotient;
        result.remainder = remainder;
        hobig_free(one);
    }

    return result;
}