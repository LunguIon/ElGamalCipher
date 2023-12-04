#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

// Global variables
int public_key_part1, public_key_part2;
int prime_modulus, private_key;
int cipher_component1, cipher_component2;
FILE *cipher_file1, *cipher_file2;

// Function to calculate the greatest common divisor
int calculateGCD(int a, int b) {
    int quotient, remainder1, remainder2, temp;

    if (a > b) {
        remainder1 = a;
        remainder2 = b;
    } else {
        remainder1 = b;
        remainder2 = a;
    }

    while (remainder2 > 0) {
        quotient = remainder1 / remainder2;
        temp = remainder1 - quotient * remainder2;
        remainder1 = remainder2;
        remainder2 = temp;
    }

    return remainder1;
}

// Function for fast exponentiation
void fastExponentiation(int bit, int n, int* result, int* base) {
    if (bit == 1) {
        *result = (*result * (*base)) % n;
    }

    *base = (*base) * (*base) % n;
}

// Function to find T using modular exponentiation
int findT(int base, int exponent, int modulus) {
    int remainder;
    int result = 1;

    while (exponent > 0) {
        remainder = exponent % 2;
        fastExponentiation(remainder, modulus, &result, &base);
        exponent = exponent / 2;
    }

    return result;
}

// Primality test for a given number
int primalityTest(int base, int number) {
    int n = number - 1;
    int k = 0;
    int m, T;

    while (n % 2 == 0) {
        k++;
        n = n / 2;
    }

    m = n;
    T = findT(base, m, number);
    if (T == 1 || T == number - 1) {
        return 1;
    }

    for (int j = 0; j < k; j++) {
        T = findT(T, 2, number);
        if (T == 1) {
            return 0;
        }
        if (T == number - 1) {
            return 1;
        }
    }

    return 0;
}

// Find a primitive root of a prime number
int findPrimitiveRoot(int prime) {
    int flag;

    for (int root = 2; root < prime; root++) {
        flag = 1;
        for (int i = 1; i < prime; i++) {
            if (findT(root, i, prime) == 1 && i < prime - 1) {
                flag = 0;
            } else if (flag && findT(root, i, prime) == 1 && i == prime - 1) {
                return root;
            }
        }
    }
}

// Key generation for ElGamal
void elGamalKeyGeneration() {
    do {
        do {
            prime_modulus = rand() + 256;
        } while (prime_modulus % 2 == 0);

    } while (!primalityTest(2, prime_modulus));
    prime_modulus = 107;

    public_key_part1 = 2;
    do {
        private_key = rand() % (prime_modulus - 2) + 1; // 1 <= d <= prime_modulus-2
    } while (calculateGCD(private_key, prime_modulus) != 1);

    private_key = 67;
    public_key_part2 = findT(public_key_part1, private_key, prime_modulus);
}

// Encryption using ElGamal
void elGamalEncryption(int plaintext) {
    cipher_file1 = fopen("cipher1.txt", "a+");
    cipher_file2 = fopen("cipher2.txt", "a+");

    int random_value;
    do {
        random_value = rand() % (prime_modulus - 1) + 1; // 1 < r < prime_modulus
    } while (calculateGCD(random_value, prime_modulus) != 1);

    cipher_component1 = findT(public_key_part1, random_value, prime_modulus);
    cipher_component2 = findT(public_key_part2, random_value, prime_modulus) * plaintext % prime_modulus;

    fprintf(cipher_file1, "%d ", cipher_component1);
    fprintf(cipher_file2, "%d ", cipher_component2);

    fclose(cipher_file1);
    fclose(cipher_file2);
}

// Decryption using ElGamal
void elGamalDecryption(int component1, int component2) {
    FILE *result_file = fopen("result.txt", "a+");
    int deciphered_value = component2 * findT(component1, prime_modulus - 1 - private_key, prime_modulus) % prime_modulus;
    fprintf(result_file, "%c", deciphered_value);
    fclose(result_file);
}

int main() {
    FILE *result_file, *input_file;

    // Destroy contents of these files (from previous runs, if any)
    result_file = fopen("result.txt", "w+");
    fclose(result_file);

    cipher_file1 = fopen("cipher1.txt", "w+");
    fclose(cipher_file1);
    cipher_file2 = fopen("cipher2.txt", "w+");
    fclose(cipher_file2);

    elGamalKeyGeneration();

    input_file = fopen("plain.txt", "r+");
    if (input_file == NULL) {
        printf("Error opening Source File.\n");
        exit(1);
    }

    while (1) {
        char character = getc(input_file);
        if (character == EOF) {
            break; // M < prime_modulus
        }

        elGamalEncryption(toascii(character));
    }

    fclose(input_file);

    FILE *input_file1, *input_file2;
    input_file1 = fopen("cipher1.txt", "r");
    input_file2 = fopen("cipher2.txt", "r");

    int component1, component2;
    while (1) {
        int ret = fscanf(input_file1, "%d", &component1);
        fscanf(input_file2, "%d", &component2);
        if (ret == -1) {
            break;
        }

        elGamalDecryption(component1, component2);
    }

    fclose(input_file1);
    fclose(input_file2);

    return 0;
}

