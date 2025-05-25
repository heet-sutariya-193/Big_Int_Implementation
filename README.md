# Big_Int_Implementation

##  Problem Statement

Standard integer data types in C (like `int`, `long`, `long long`) have limited precision and cannot handle extremely large numbers (hundreds of digits long). This project addresses the need for performing arithmetic operations (addition, subtraction, multiplication) on such **very large integers**, commonly known as **Big Integers**, which exceed 64-bit or 128-bit boundaries.

This system allows for accurate computations of numbers up to **1024 bits**, handling signs, carry propagation, and overflow with custom logic.

---

##  Concepts Used

- **Bit manipulation and chunk-based representation**: A 1024-bit number is divided into 32-bit chunks using an array.
- **Structs in C**: `BigInt` is a structured data type encapsulating sign and magnitude.
- **Manual base conversion**: Decimal string to base-2^32 chunk array and back.
- **Grade-school multiplication logic**: Implemented chunk-wise with carry management.
- **Dynamic string parsing and validation**: Input strings are validated and cleaned before conversion.
- **Overflow detection and error handling** in arithmetic operations.

---

##  Why Is It Important?

- **Scientific computing** often involves calculations beyond standard integer sizes.
- **Cryptographic algorithms** (RSA, ECC) rely on large number arithmetic.
- **Blockchain** and **financial systems** deal with arbitrarily large values.
- Helps in **understanding internal arithmetic logic** that underpins languages like Python, which natively support big integers.

This project serves as an educational foundation to build more advanced arbitrary-precision arithmetic systems.

---

##  Input and Output Format

### ➤ Input

The program asks for three inputs from the user:
1. First integer (as a decimal string, can be negative).
2. Operator (`+`, `-`, `*`).
3. Second integer (as a decimal string, can be negative).

**Example:**
```
Enter first number:
999999999999999999999999999999
Enter operation (+, -, *):
*
Enter second number:
100000000000000000000000000000
```

### ➤ Output

Displays the result of the arithmetic operation in decimal form:

```
Result of multiplication:
99999999999999999999999999999900000000000000000000000000000
```

If the input is invalid or an overflow occurs:

```
Error: Input number '...' too large (exceeds 1024 bits). Setting to 0.
```

---

##  How to Use

###  Compilation

Make sure you have `gcc` or any C compiler installed.

```bash
gcc Big_Int.c -o BigInt
```

###  Running the Program

```bash
./BigInt
```

You will be prompted to enter two numbers and an operation (`+`, `-`, `*`).

---

##  Features

- Handles up to **309-digit numbers** (1024 bits).
- Performs **addition**, **subtraction**, and **multiplication** with proper sign handling.
- Detects **invalid inputs** and **overflows** gracefully.
- Shows the **exact decimal result**.

---

