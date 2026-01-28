# 🏦 Bank Management System — Système de Gestion Bancaire (C)

[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)]()

> Console-based banking application in **C** simulating core banking operations: account management, deposits, withdrawals, transfers, and transaction history tracking.

---

## Table of Contents
- [Overview](#overview)
- [Features](#features)
  - [Account Management](#account-management)
  - [Financial Operations](#financial-operations)
  - [Transaction History](#transaction-history)
- [Project Structure](#project-structure)
- [Data Structures](#data-structures)
- [Installation & Usage](#installation--usage)
  - [Compilation](#compilation)
  - [Running the Program](#running-the-program)
  - [Recommended Workflow](#recommended-workflow)
- [Security Features](#security-features)
- [Technical Implementation](#technical-implementation)
- [Author](#author)
- [Conclusion](#conclusion)

---

## Overview

This program is a **console-based application** that manages essential banking operations in an educational context.

**Key capabilities:**
- Complete **account management** system
- **Financial operations** (deposits, withdrawals, transfers)
- **Transaction history** tracking and consultation
- **Data persistence** through file handling
- Input validation and security measures

The system operates **without external databases** and relies solely on the C programming language and file I/O operations.

---

## Features

### Account Management
- Create new bank accounts with unique account numbers
- Set initial balance and 4-digit PIN during account creation
- View account details (balance, account number)
- PIN-protected access to sensitive operations
- Account information stored persistently

### Financial Operations

**Deposits:**
- Add funds to any account
- Positive amount validation
- Immediate balance update
- Transaction recorded in history

**Withdrawals:**
- PIN authentication required
- Positive amount validation
- Sufficient balance verification
- Balance updated after successful withdrawal
- Transaction logged

**Transfers:**
- Transfer funds between accounts
- PIN authentication for source account
- Validates both source and destination accounts exist
- Ensures sufficient balance in source account
- Updates both accounts simultaneously
- Records transaction for both accounts

### Transaction History
- View complete transaction history for any account
- Shows transaction type (Deposit/Withdrawal/Transfer)
- Displays amount and destination account (for transfers)
- Chronological listing of all operations
- Helps track account activity

---

## Project Structure

```
bank-management-system/
├── bank_system.c        # Main source code
└── accounts.txt         # Data persistence file
```

> The `accounts.txt` file is created automatically on first run.

---

## Data Structures

The program uses a structured approach to store account information:

```c
struct Account {
    int accountNumber;      // Unique identifier
    char pin[5];           // 4-digit PIN (string for leading zeros)
    float balance;         // Current account balance
    // Transaction history stored separately
}
```

### File Format
Data is stored in a simple text-based format in `accounts.txt`:
- Each line represents one account
- Fields are space-separated for easy parsing
- Transaction history maintained in program memory during runtime

---

## Installation & Usage

### Compilation

Compile the program using GCC or any C compiler:

```bash
gcc bank_system.c -o bank_system
```

Or with additional flags:

```bash
gcc -Wall -Wextra bank_system.c -o bank_system
```

### Running the Program

Execute the compiled binary:

```bash
./bank_system
```

The main menu will appear with the following options:

```
=== Bank Management System ===
1. Create Account
2. Deposit
3. Withdraw
4. Transfer
5. Check Balance
6. View Transaction History
7. Exit
```

### Recommended Workflow

1. **Create an Account**
   - Enter desired account number
   - Set a 4-digit PIN
   - Specify initial balance

2. **Perform Operations**
   - Deposit funds to add money
   - Withdraw with PIN authentication
   - Transfer between accounts securely

3. **Monitor Activity**
   - Check balance anytime
   - View transaction history for audit trail

---

## Security Features

- **PIN Protection**: All sensitive operations require 4-digit PIN verification
- **Input Validation**: 
  - Positive amounts enforced for all transactions
  - Account existence verified before operations
  - Duplicate account numbers prevented
- **Balance Verification**: Withdrawals and transfers check for sufficient funds
- **Data Persistence**: Account data saved to file, loaded on startup
- **Transaction Logging**: Complete audit trail of all operations

---

## Technical Implementation

### Key Programming Concepts Demonstrated:

1. **Structures**: Used to organize account data efficiently
2. **File I/O**: 
   - Read/write operations for data persistence
   - Text-based storage for portability
3. **Input Validation**: Robust checking of user inputs
4. **Menu-Driven Interface**: User-friendly console navigation
5. **Array Management**: Dynamic handling of multiple accounts
6. **String Operations**: PIN comparison and account searching
7. **Error Handling**: Graceful handling of invalid operations

### Core Functions:
- `createAccount()` - Account registration with validation
- `deposit()` - Add funds to account
- `withdraw()` - Remove funds with authentication
- `transfer()` - Move funds between accounts
- `checkBalance()` - Display account balance
- `viewHistory()` - Show transaction records
- `saveAccounts()` - Persist data to file
- `loadAccounts()` - Load data from file

---

## Author

**Hamza Bordo**

📚 Project Type: Academic Programming Project  
💻 Language: C  
🎓 Institution: Faculté des Sciences Ben M'Sik  
📅 Year: 2024-2025

---

## Conclusion

This project demonstrates:

- **Strong command of C programming fundamentals**
- **Effective use of data structures** (structs, arrays)
- **File handling** for data persistence
- **Input validation and error handling**
- **Implementation of realistic banking business rules**
- **Clean code organization** and logical flow

The system provides a solid foundation for understanding core programming concepts and can be extended with features such as:
- Multiple user types (admin/customer)
- Interest calculation
- Account statements export
- Enhanced security (password encryption)
- Database integration
- Graphical user interface

🏦 **Bank Management System**  
*Academic Project — C Programming*
