# 🚗 Terminal-Based Car Rental Management System in C (Text File CRUD)

A complete, beginner-friendly, and modular console-based application written in standard **C (C99)**.

---

## 📌 Project Overview
* **Language:** Pure C (no external libraries or GUI dependencies)
* **Storage Engine:** Human-readable pipe-delimited text files (`cars.txt`, `customers.txt`, `bookings.txt`)
* **Target Audience:** MCA / B.Tech / BCA Students
* **Platform:** macOS, Linux, Windows

---

## 📄 File Storage & CRUD Formats

All data is stored directly in plain `.txt` files on your Desktop. You can open and view them anytime in **TextEdit, Notepad, or VS Code**:

### 1. `cars.txt`
```text
carId|brand|model|year|type|fuelType|seats|pricePerDay|available
101|Hyundai|Creta|2024|SUV|Petrol|5|2500.00|1
102|Maruti|Swift|2023|Hatchback|Petrol|5|1500.00|1
103|Tata|Nexon|2024|SUV|Diesel|5|2200.00|1
104|Honda|City|2023|Sedan|Petrol|5|2000.00|1
105|Toyota|Fortuner|2024|SUV|Diesel|7|4500.00|1
```
* `available`: `1` = Available for rent, `0` = Currently rented.

### 2. `customers.txt`
```text
customerId|name|phone|email|password
501|Nikhil Sharma|9876543210|nikhil@test.com|pass123
```

### 3. `bookings.txt`
```text
bookingId|customerId|carId|days|actualDays|dailyRate|totalAmount|lateFee|status
1001|501|101|3|5|2500.00|7500.00|1000.00|1
```
* `status`: `0` = Active, `1` = Completed/Returned, `2` = Cancelled.

---

## 🛠️ How CRUD Operations Work in C

| Operation | C Implementation | Explanation |
| :--- | :--- | :--- |
| **CREATE (Insert)** | `fopen("cars.txt", "a")` + `fprintf(...)` | Appends a new formatted record line to the end of the file. |
| **READ (Select)** | `fopen("cars.txt", "r")` + `fgets(...)` + `sscanf("%d\|%[^\|]\|...", ...)` | Reads line-by-line and parses fields using pipe `\|` delimiters. |
| **UPDATE (Modify)** | Read original + write to `temp_cars.txt` + `remove()` + `rename()` | Copies all lines to a temporary file while replacing the target record, then swaps files atomically. |
| **DELETE (Remove)** | Read original + write non-matching to `temp_cars.txt` + `rename()` | Skips the deleted record when writing to the temp file. |

---

## 🔑 Default Credentials

### Administrator
* **Username:** `admin`
* **Password:** `admin123`

---

## 🚀 How to Compile and Run

Open Terminal on macOS and execute:

```bash
cd /Users/nikhil/Desktop/CarRentalSystem

# Compile using GCC with strict standard C99 flags:
gcc -Wall -Wextra -std=c99 car_rental.c -o car_rental

# Run the program:
./car_rental
```

---

## 🎓 Viva Voce & Oral Exam Questions and Answers

### 1. Why did you use text files (`.txt`) with pipe delimiters (`|`)?
* **Answer:** Text files are human-readable, transparent, and can be inspected or edited using any text editor. Using pipe delimiters (`|`) allows customer names and car models to contain spaces (e.g., "Nikhil Sharma", "Fortuner Legender") without breaking field parsing.

### 2. How does `sscanf()` parse delimited text files?
* **Answer:** We use the scanset format specifier `%[^|]`, which instructs `sscanf()` to read all characters until it encounters the pipe symbol (`|`).

### 3. How do you update or delete records in text files?
* **Answer:** Text records have variable lengths, so we cannot overwrite them in-place like fixed-size binary records. Instead, we:
  1. Open the original file in read mode (`"r"`).
  2. Open a temporary file in write mode (`"w"`).
  3. Loop through lines with `fgets()`, modify/skip the target record, and write to the temp file with `fprintf()`.
  4. Close both files, remove the original with `remove()`, and rename the temporary file using `rename()`.

### 4. How do you prevent double booking?
* **Answer:** Before creating a booking, the system checks `car.available == 1`. When confirmed, the booking is recorded, and the car's availability in `cars.txt` is updated to `0`.

### 5. How are late fees calculated upon return?
* **Answer:** The customer enters their actual rental days. If `actualDays > plannedDays`, the overdue days are computed (`overdue = actualDays - plannedDays`) and multiplied by a late charge of ₹500/day.
