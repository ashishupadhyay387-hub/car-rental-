/*
 * ============================================================================
 * Project: Terminal-Based Car Rental Management System
 * Author: MCA Student
 * Language: C (C99 Standard)
 * Compiler: GCC / Clang
 * Storage: Text-Based File Persistence (CRUD via cars.txt, customers.txt, bookings.txt)
 * Platform: macOS / Linux / Windows
 * ============================================================================
 * File Storage Format:
 *   - cars.txt:      carId|brand|model|year|type|fuelType|seats|pricePerDay|available
 *   - customers.txt: customerId|name|phone|email|password
 *   - bookings.txt:  bookingId|customerId|carId|days|actualDays|dailyRate|totalAmount|lateFee|status
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CARS_FILE "cars.txt"
#define CUSTOMERS_FILE "customers.txt"
#define BOOKINGS_FILE "bookings.txt"

#define LATE_FEE_PER_DAY 500.0f
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/* Structure representing a Car */
struct Car {
    int carId;              /* Unique ID, e.g. 101, 102 */
    char brand[30];         /* Manufacturer (Hyundai, Tata, etc.) */
    char model[30];         /* Model name (Creta, Nexon, etc.) */
    int year;               /* Manufacturing Year (2023, 2024) */
    char type[20];          /* SUV, Sedan, Hatchback */
    char fuelType[20];      /* Petrol, Diesel, EV */
    int seats;              /* Number of seats (5, 7) */
    float pricePerDay;      /* Rental charge per day in Rupees */
    int available;          /* 1 = Available, 0 = Currently Rented */
};

/* Structure representing a registered Customer */
struct Customer {
    int customerId;         /* Unique Customer ID, e.g. 501 */
    char name[50];          /* Full Name */
    char phone[15];         /* Contact Phone */
    char email[50];         /* Email Address */
    char password[30];      /* Account Password */
};

/* Structure representing a Rental Booking */
struct Booking {
    int bookingId;          /* Unique Booking ID, e.g. 1001 */
    int customerId;         /* Customer who rented */
    int carId;              /* Car rented */
    int days;               /* Planned rental days */
    int actualDays;         /* Actual days taken upon return */
    float dailyRate;        /* Locked-in rate at booking */
    float totalAmount;      /* Base amount = days * dailyRate */
    float lateFee;          /* Late return surcharge */
    int status;             /* 0 = Active, 1 = Completed/Returned, 2 = Cancelled */
};

/* ============================================================================
 * FUNCTION DECLARATIONS (PROTOTYPES)
 * ============================================================================ */

/* Utility Functions */
void clearInputBuffer(void);
void getString(char *dest, int maxLen);
void pauseScreen(void);
int strContainsIgnoreCase(const char *haystack, const char *needle);
void displayBanner(const char *title);

/* Text File CRUD & Database Helpers */
void initializeSampleCars(void);
int parseCarLine(const char *line, struct Car *outCar);
int parseCustomerLine(const char *line, struct Customer *outCust);
int parseBookingLine(const char *line, struct Booking *outBooking);

int isCarIdExists(int carId);
int isCustomerIdExists(int customerId);
int getNextBookingId(void);
int getCarById(int carId, struct Car *outCar);
int getCustomerById(int customerId, struct Customer *outCust);
int updateCarAvailability(int carId, int availability);
int updateCarRecord(struct Car updatedCar);

/* Customer Module Functions */
void customerMenu(void);
void customerRegister(void);
int customerLogin(void);
void customerDashboard(int customerId);
void viewAvailableCars(void);
void searchCarSubmenu(void);
void viewCarDetailsInteractive(void);
void bookCar(int customerId);
void viewMyBookings(int customerId);
void cancelBooking(int customerId);
void returnCar(int customerId);
void generateBillForCustomer(int customerId);
void printBillSlip(const struct Booking *b, const struct Customer *cust, const struct Car *car);

/* Admin Module Functions */
int adminLogin(void);
void adminDashboard(void);
void addCar(void);
void viewAllCars(int onlyAvailable);
void updateCar(void);
void deleteCar(void);
void viewAllCustomers(void);
void viewAllBookings(void);
void viewRevenueSummary(void);

/* ============================================================================
 * MAIN ENTRY POINT
 * ============================================================================ */
int main(void) {
    int choice = 0;

    /* Initialize sample cars in cars.txt if not existing */
    initializeSampleCars();

    while (1) {
        if (feof(stdin)) break;

        displayBanner("CAR RENTAL SYSTEM - MAIN MENU");
        printf("  1. Customer Portal\n");
        printf("  2. Administrator Portal\n");
        printf("  3. Exit Application\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice (1-3): ");

        int res = scanf("%d", &choice);
        if (res == EOF) break;
        if (res != 1) {
            printf("\n[!] Invalid input. Please enter a valid number.\n");
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                customerMenu();
                break;
            case 2:
                if (adminLogin()) {
                    adminDashboard();
                }
                break;
            case 3:
                displayBanner("THANK YOU");
                printf("Thank you for using Car Rental Management System.\n");
                printf("All changes successfully persisted to text files!\n");
                printf("Exiting application safely. Goodbye!\n");
                printf("==================================================\n\n");
                return 0;
            default:
                printf("\n[!] Invalid selection! Please choose 1, 2, or 3.\n");
                pauseScreen();
                break;
        }
    }

    return 0;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/* Flushes any remaining characters from input buffer up to newline or EOF */
void clearInputBuffer(void) {
    if (feof(stdin)) return;
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Discard */
    }
}

/* Safely reads a string with fgets and strips the trailing newline */
void getString(char *dest, int maxLen) {
    if (feof(stdin)) {
        dest[0] = '\0';
        return;
    }
    if (fgets(dest, maxLen, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && (dest[len - 1] == '\n' || dest[len - 1] == '\r')) {
            dest[len - 1] = '\0';
        }
        len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\r') {
            dest[len - 1] = '\0';
        }
    } else {
        dest[0] = '\0';
    }
}

/* Prompts user to press Enter before returning to menu */
void pauseScreen(void) {
    if (feof(stdin)) return;
    printf("\nPress [Enter] to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Consume until newline */
    }
}

/* Case-insensitive substring match helper */
int strContainsIgnoreCase(const char *haystack, const char *needle) {
    char hLower[100];
    char nLower[100];
    size_t i;

    if (!haystack || !needle) return 0;
    if (strlen(needle) == 0) return 1;

    for (i = 0; haystack[i] && i < sizeof(hLower) - 1; i++) {
        hLower[i] = (char)tolower((unsigned char)haystack[i]);
    }
    hLower[i] = '\0';

    for (i = 0; needle[i] && i < sizeof(nLower) - 1; i++) {
        nLower[i] = (char)tolower((unsigned char)needle[i]);
    }
    nLower[i] = '\0';

    return (strstr(hLower, nLower) != NULL);
}

/* Prints a clean banner title */
void displayBanner(const char *title) {
    printf("\n==================================================\n");
    printf("  %s\n", title);
    printf("==================================================\n");
}

/* ============================================================================
 * TEXT FILE PARSING HELPERS
 * ============================================================================ */

/* Parses a line from cars.txt into a struct Car */
int parseCarLine(const char *line, struct Car *outCar) {
    int n = sscanf(line, "%d|%29[^|]|%29[^|]|%d|%19[^|]|%19[^|]|%d|%f|%d",
                   &outCar->carId,
                   outCar->brand,
                   outCar->model,
                   &outCar->year,
                   outCar->type,
                   outCar->fuelType,
                   &outCar->seats,
                   &outCar->pricePerDay,
                   &outCar->available);
    return (n == 9);
}

/* Parses a line from customers.txt into a struct Customer */
int parseCustomerLine(const char *line, struct Customer *outCust) {
    int n = sscanf(line, "%d|%49[^|]|%14[^|]|%49[^|]|%29[^\r\n]",
                   &outCust->customerId,
                   outCust->name,
                   outCust->phone,
                   outCust->email,
                   outCust->password);
    return (n == 5);
}

/* Parses a line from bookings.txt into a struct Booking */
int parseBookingLine(const char *line, struct Booking *outBooking) {
    int n = sscanf(line, "%d|%d|%d|%d|%d|%f|%f|%f|%d",
                   &outBooking->bookingId,
                   &outBooking->customerId,
                   &outBooking->carId,
                   &outBooking->days,
                   &outBooking->actualDays,
                   &outBooking->dailyRate,
                   &outBooking->totalAmount,
                   &outBooking->lateFee,
                   &outBooking->status);
    return (n == 9);
}

/* ============================================================================
 * TEXT FILE DATABASE OPERATIONS (CRUD)
 * ============================================================================ */

/* CREATE (Seed): Creates cars.txt with 5 sample cars if not existing */
void initializeSampleCars(void) {
    FILE *fp = fopen(CARS_FILE, "r");
    if (fp != NULL) {
        fclose(fp);
        return;
    }

    fp = fopen(CARS_FILE, "w");
    if (fp == NULL) {
        printf("[!] Warning: Could not create %s file.\n", CARS_FILE);
        return;
    }

    /* Seed initial sample cars into cars.txt */
    fprintf(fp, "101|Hyundai|Creta|2024|SUV|Petrol|5|2500.00|1\n");
    fprintf(fp, "102|Maruti|Swift|2023|Hatchback|Petrol|5|1500.00|1\n");
    fprintf(fp, "103|Tata|Nexon|2024|SUV|Diesel|5|2200.00|1\n");
    fprintf(fp, "104|Honda|City|2023|Sedan|Petrol|5|2000.00|1\n");
    fprintf(fp, "105|Toyota|Fortuner|2024|SUV|Diesel|7|4500.00|1\n");

    fclose(fp);
}

/* READ: Checks if Car ID exists in cars.txt */
int isCarIdExists(int carId) {
    FILE *fp = fopen(CARS_FILE, "r");
    if (fp == NULL) return 0;

    char line[256];
    struct Car c;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.carId == carId) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* READ: Checks if Customer ID exists in customers.txt */
int isCustomerIdExists(int customerId) {
    FILE *fp = fopen(CUSTOMERS_FILE, "r");
    if (fp == NULL) return 0;

    char line[256];
    struct Customer c;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCustomerLine(line, &c)) {
            if (c.customerId == customerId) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* READ: Computes next unique booking ID from bookings.txt */
int getNextBookingId(void) {
    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL) return 1001;

    char line[256];
    struct Booking b;
    int maxId = 1000;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            if (b.bookingId > maxId) {
                maxId = b.bookingId;
            }
        }
    }
    fclose(fp);
    return maxId + 1;
}

/* READ: Looks up a car by ID from cars.txt */
int getCarById(int carId, struct Car *outCar) {
    FILE *fp = fopen(CARS_FILE, "r");
    if (fp == NULL) return 0;

    char line[256];
    struct Car c;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.carId == carId) {
                if (outCar != NULL) *outCar = c;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* READ: Looks up a customer by ID from customers.txt */
int getCustomerById(int customerId, struct Customer *outCust) {
    FILE *fp = fopen(CUSTOMERS_FILE, "r");
    if (fp == NULL) return 0;

    char line[256];
    struct Customer c;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCustomerLine(line, &c)) {
            if (c.customerId == customerId) {
                if (outCust != NULL) *outCust = c;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* UPDATE: Updates car availability flag in cars.txt using temporary text file */
int updateCarAvailability(int carId, int availability) {
    FILE *fp = fopen(CARS_FILE, "r");
    FILE *tempFp = fopen("temp_cars.txt", "w");
    if (fp == NULL || tempFp == NULL) {
        if (fp) fclose(fp);
        if (tempFp) fclose(tempFp);
        return 0;
    }

    char line[256];
    struct Car c;
    int updated = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.carId == carId) {
                c.available = availability;
                updated = 1;
            }
            fprintf(tempFp, "%d|%s|%s|%d|%s|%s|%d|%.2f|%d\n",
                    c.carId, c.brand, c.model, c.year, c.type,
                    c.fuelType, c.seats, c.pricePerDay, c.available);
        }
    }

    fclose(fp);
    fclose(tempFp);

    if (updated) {
        remove(CARS_FILE);
        rename("temp_cars.txt", CARS_FILE);
        return 1;
    }

    remove("temp_cars.txt");
    return 0;
}

/* UPDATE: Modifies all fields of a car in cars.txt */
int updateCarRecord(struct Car updatedCar) {
    FILE *fp = fopen(CARS_FILE, "r");
    FILE *tempFp = fopen("temp_cars.txt", "w");
    if (fp == NULL || tempFp == NULL) {
        if (fp) fclose(fp);
        if (tempFp) fclose(tempFp);
        return 0;
    }

    char line[256];
    struct Car c;
    int updated = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.carId == updatedCar.carId) {
                c = updatedCar;
                updated = 1;
            }
            fprintf(tempFp, "%d|%s|%s|%d|%s|%s|%d|%.2f|%d\n",
                    c.carId, c.brand, c.model, c.year, c.type,
                    c.fuelType, c.seats, c.pricePerDay, c.available);
        }
    }

    fclose(fp);
    fclose(tempFp);

    if (updated) {
        remove(CARS_FILE);
        rename("temp_cars.txt", CARS_FILE);
        return 1;
    }

    remove("temp_cars.txt");
    return 0;
}

/* ============================================================================
 * CUSTOMER MODULE IMPLEMENTATION
 * ============================================================================ */

void customerMenu(void) {
    int choice = 0;
    while (1) {
        if (feof(stdin)) return;

        displayBanner("CUSTOMER PORTAL");
        printf("  1. Register New Account\n");
        printf("  2. Login to Account\n");
        printf("  3. Back to Main Menu\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice (1-3): ");

        int res = scanf("%d", &choice);
        if (res == EOF) return;
        if (res != 1) {
            printf("\n[!] Invalid input! Please enter a number.\n");
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                customerRegister();
                break;
            case 2: {
                int loggedId = customerLogin();
                if (loggedId > 0) {
                    customerDashboard(loggedId);
                }
                break;
            }
            case 3:
                return;
            default:
                printf("\n[!] Invalid choice! Select between 1 and 3.\n");
                pauseScreen();
                break;
        }
    }
}

/* CREATE: Appends new customer to customers.txt */
void customerRegister(void) {
    struct Customer newCust;
    FILE *fp;

    displayBanner("CUSTOMER REGISTRATION");

    printf("Enter Desired Customer ID (numbers only): ");
    if (scanf("%d", &newCust.customerId) != 1 || newCust.customerId <= 0) {
        printf("[!] Error: Customer ID must be a positive integer.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    if (isCustomerIdExists(newCust.customerId)) {
        printf("[!] Error: Customer ID %d is already registered! Please choose another.\n", newCust.customerId);
        pauseScreen();
        return;
    }

    printf("Enter Full Name: ");
    getString(newCust.name, sizeof(newCust.name));
    if (strlen(newCust.name) == 0) {
        printf("[!] Error: Name cannot be empty.\n");
        pauseScreen();
        return;
    }

    printf("Enter Phone Number: ");
    getString(newCust.phone, sizeof(newCust.phone));
    if (strlen(newCust.phone) == 0) {
        printf("[!] Error: Phone number cannot be empty.\n");
        pauseScreen();
        return;
    }

    printf("Enter Email Address: ");
    getString(newCust.email, sizeof(newCust.email));
    if (strlen(newCust.email) == 0) {
        printf("[!] Error: Email cannot be empty.\n");
        pauseScreen();
        return;
    }

    printf("Enter Password: ");
    getString(newCust.password, sizeof(newCust.password));
    if (strlen(newCust.password) == 0) {
        printf("[!] Error: Password cannot be empty.\n");
        pauseScreen();
        return;
    }

    fp = fopen(CUSTOMERS_FILE, "a");
    if (fp == NULL) {
        printf("[!] Error: Unable to open %s for saving.\n", CUSTOMERS_FILE);
        pauseScreen();
        return;
    }

    /* Append structured text record */
    fprintf(fp, "%d|%s|%s|%s|%s\n",
            newCust.customerId, newCust.name, newCust.phone, newCust.email, newCust.password);
    fclose(fp);

    printf("\n[✓] Registration Successful! Saved in %s with Customer ID: %d\n", CUSTOMERS_FILE, newCust.customerId);
    pauseScreen();
}

/* READ: Authenticates against customers.txt */
int customerLogin(void) {
    int id;
    char pass[30];
    int attempts = 0;

    displayBanner("CUSTOMER LOGIN");

    while (attempts < 3) {
        if (feof(stdin)) return 0;

        printf("Customer ID: ");
        if (scanf("%d", &id) != 1) {
            printf("[!] Invalid ID format.\n");
            clearInputBuffer();
            attempts++;
            continue;
        }
        clearInputBuffer();

        printf("Password: ");
        getString(pass, sizeof(pass));

        FILE *fp = fopen(CUSTOMERS_FILE, "r");
        if (fp == NULL) {
            printf("[!] No customer accounts found in %s. Please register first!\n", CUSTOMERS_FILE);
            pauseScreen();
            return 0;
        }

        char line[256];
        struct Customer c;
        int found = 0;
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (parseCustomerLine(line, &c)) {
                if (c.customerId == id && strcmp(c.password, pass) == 0) {
                    found = 1;
                    break;
                }
            }
        }
        fclose(fp);

        if (found) {
            printf("\n[✓] Login successful! Welcome back, %s.\n", c.name);
            pauseScreen();
            return id;
        } else {
            attempts++;
            printf("\n[!] Invalid Customer ID or Password. Attempts remaining: %d\n\n", 3 - attempts);
        }
    }

    printf("[!] Too many failed attempts. Returning to menu.\n");
    pauseScreen();
    return 0;
}

/* Customer Dashboard */
void customerDashboard(int customerId) {
    int choice = 0;
    struct Customer currentCust;
    getCustomerById(customerId, &currentCust);

    while (1) {
        if (feof(stdin)) return;

        displayBanner("CUSTOMER DASHBOARD");
        printf("Logged in as: %s (ID: %d)\n", currentCust.name, customerId);
        printf("--------------------------------------------------\n");
        printf("  1. View Available Cars\n");
        printf("  2. Search Cars\n");
        printf("  3. View Car Details\n");
        printf("  4. Book a Car\n");
        printf("  5. My Bookings\n");
        printf("  6. Cancel Booking\n");
        printf("  7. Return Car\n");
        printf("  8. Generate / View Bill\n");
        printf("  9. Logout\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice (1-9): ");

        int res = scanf("%d", &choice);
        if (res == EOF) return;
        if (res != 1) {
            printf("\n[!] Invalid input! Please enter a number.\n");
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                viewAvailableCars();
                pauseScreen();
                break;
            case 2:
                searchCarSubmenu();
                break;
            case 3:
                viewCarDetailsInteractive();
                break;
            case 4:
                bookCar(customerId);
                break;
            case 5:
                viewMyBookings(customerId);
                pauseScreen();
                break;
            case 6:
                cancelBooking(customerId);
                break;
            case 7:
                returnCar(customerId);
                break;
            case 8:
                generateBillForCustomer(customerId);
                break;
            case 9:
                printf("\n[✓] Logged out successfully.\n");
                pauseScreen();
                return;
            default:
                printf("\n[!] Invalid choice! Select between 1 and 9.\n");
                pauseScreen();
                break;
        }
    }
}

/* READ: Views Available Cars from cars.txt */
void viewAvailableCars(void) {
    FILE *fp = fopen(CARS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No car database found in %s.\n", CARS_FILE);
        return;
    }

    displayBanner("CURRENTLY AVAILABLE CARS");
    printf("%-6s %-12s %-12s %-10s %-8s %-6s %-12s\n",
           "ID", "Brand", "Model", "Type", "Fuel", "Seats", "Price/Day");
    printf("----------------------------------------------------------------------\n");

    char line[256];
    struct Car c;
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.available == 1) {
                printf("%-6d %-12s %-12s %-10s %-8s %-6d Rs. %-8.2f\n",
                       c.carId, c.brand, c.model, c.type, c.fuelType, c.seats, c.pricePerDay);
                count++;
            }
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("No cars are currently available for rent.\n");
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total available: %d\n", count);
}

/* READ: Multi-filter search from cars.txt */
void searchCarSubmenu(void) {
    int searchOption;
    char query[50];
    int queryId;

    displayBanner("SEARCH CAR INVENTORY");
    printf("  1. Search by Brand\n");
    printf("  2. Search by Model\n");
    printf("  3. Search by Car Type (SUV, Sedan, Hatchback)\n");
    printf("  4. Search by Car ID\n");
    printf("  5. Back\n");
    printf("--------------------------------------------------\n");
    printf("Enter choice (1-5): ");

    if (scanf("%d", &searchOption) != 1) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (searchOption == 5) return;

    FILE *fp = fopen(CARS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No car inventory found.\n");
        pauseScreen();
        return;
    }

    char line[256];
    struct Car c;
    int matches = 0;

    if (searchOption == 4) {
        printf("Enter Car ID: ");
        if (scanf("%d", &queryId) != 1) {
            clearInputBuffer();
            fclose(fp);
            return;
        }
        clearInputBuffer();

        printf("\n%-6s %-12s %-12s %-10s %-8s %-6s %-10s %-12s\n",
               "ID", "Brand", "Model", "Type", "Fuel", "Seats", "Price/Day", "Status");
        printf("--------------------------------------------------------------------------------\n");

        while (fgets(line, sizeof(line), fp) != NULL) {
            if (parseCarLine(line, &c)) {
                if (c.carId == queryId) {
                    printf("%-6d %-12s %-12s %-10s %-8s %-6d Rs. %-7.2f %s\n",
                           c.carId, c.brand, c.model, c.type, c.fuelType, c.seats,
                           c.pricePerDay, (c.available ? "Available" : "Rented"));
                    matches++;
                }
            }
        }
    } else {
        printf("Enter search keyword: ");
        getString(query, sizeof(query));

        printf("\n%-6s %-12s %-12s %-10s %-8s %-6s %-10s %-12s\n",
               "ID", "Brand", "Model", "Type", "Fuel", "Seats", "Price/Day", "Status");
        printf("--------------------------------------------------------------------------------\n");

        while (fgets(line, sizeof(line), fp) != NULL) {
            if (parseCarLine(line, &c)) {
                int match = 0;
                if (searchOption == 1 && strContainsIgnoreCase(c.brand, query)) match = 1;
                else if (searchOption == 2 && strContainsIgnoreCase(c.model, query)) match = 1;
                else if (searchOption == 3 && strContainsIgnoreCase(c.type, query)) match = 1;

                if (match) {
                    printf("%-6d %-12s %-12s %-10s %-8s %-6d Rs. %-7.2f %s\n",
                           c.carId, c.brand, c.model, c.type, c.fuelType, c.seats,
                           c.pricePerDay, (c.available ? "Available" : "Rented"));
                    matches++;
                }
            }
        }
    }

    fclose(fp);
    printf("--------------------------------------------------------------------------------\n");
    printf("Matching results found: %d\n", matches);
    pauseScreen();
}

/* READ: Views specs of specific car */
void viewCarDetailsInteractive(void) {
    int carId;
    displayBanner("VIEW CAR SPECIFICATIONS");
    printf("Enter Car ID: ");
    if (scanf("%d", &carId) != 1) {
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    struct Car c;
    if (!getCarById(carId, &c)) {
        printf("\n[!] Car with ID %d not found in records.\n", carId);
        pauseScreen();
        return;
    }

    printf("\n==================================================\n");
    printf("          CAR SPECIFICATION SHEET                 \n");
    printf("==================================================\n");
    printf("  Car ID         : %d\n", c.carId);
    printf("  Brand          : %s\n", c.brand);
    printf("  Model          : %s\n", c.model);
    printf("  Year           : %d\n", c.year);
    printf("  Category       : %s\n", c.type);
    printf("  Fuel Type      : %s\n", c.fuelType);
    printf("  Seating        : %d Seater\n", c.seats);
    printf("  Daily Rate     : Rs. %.2f per day\n", c.pricePerDay);
    printf("  Current Status : %s\n", c.available ? "Available for Rent" : "Currently Booked / Rented");
    printf("==================================================\n");
    pauseScreen();
}

/* CREATE (Booking) & UPDATE (Car Availability in cars.txt) */
void bookCar(int customerId) {
    int carId;
    int rentalDays;
    struct Car car;
    struct Customer cust;

    getCustomerById(customerId, &cust);

    displayBanner("BOOK A CAR");
    viewAvailableCars();

    printf("\nEnter Car ID to book (or 0 to cancel): ");
    if (scanf("%d", &carId) != 1 || carId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (!getCarById(carId, &car)) {
        printf("[!] Car ID %d does not exist.\n", carId);
        pauseScreen();
        return;
    }

    /* Double-booking prevention check */
    if (car.available != 1) {
        printf("\n[!] Sorry, this car (%s %s) is currently UNAVAILABLE or already rented.\n",
               car.brand, car.model);
        pauseScreen();
        return;
    }

    printf("Enter Number of Rental Days: ");
    if (scanf("%d", &rentalDays) != 1 || rentalDays <= 0) {
        printf("[!] Invalid rental duration. Must be at least 1 day.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    float totalAmount = car.pricePerDay * (float)rentalDays;

    printf("\n==================================================\n");
    printf("  RENTAL SUMMARY FOR CONFIRMATION\n");
    printf("==================================================\n");
    printf("  Customer    : %s (ID: %d)\n", cust.name, cust.customerId);
    printf("  Car         : %s %s (%d)\n", car.brand, car.model, car.year);
    printf("  Duration    : %d day(s)\n", rentalDays);
    printf("  Rate / Day  : Rs. %.2f\n", car.pricePerDay);
    printf("  Total Est.  : Rs. %.2f\n", totalAmount);
    printf("==================================================\n");
    printf("Confirm this booking? (Y/N): ");

    char confirm = 'n';
    scanf(" %c", &confirm);
    clearInputBuffer();

    if (confirm != 'y' && confirm != 'Y') {
        printf("\n[!] Booking cancelled by user.\n");
        pauseScreen();
        return;
    }

    struct Booking newBooking;
    newBooking.bookingId = getNextBookingId();
    newBooking.customerId = customerId;
    newBooking.carId = carId;
    newBooking.days = rentalDays;
    newBooking.actualDays = rentalDays;
    newBooking.dailyRate = car.pricePerDay;
    newBooking.totalAmount = totalAmount;
    newBooking.lateFee = 0.0f;
    newBooking.status = 0; /* Active */

    /* Append to bookings.txt */
    FILE *fp = fopen(BOOKINGS_FILE, "a");
    if (fp == NULL) {
        printf("[!] Error saving booking to %s.\n", BOOKINGS_FILE);
        pauseScreen();
        return;
    }
    fprintf(fp, "%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%d\n",
            newBooking.bookingId, newBooking.customerId, newBooking.carId,
            newBooking.days, newBooking.actualDays, newBooking.dailyRate,
            newBooking.totalAmount, newBooking.lateFee, newBooking.status);
    fclose(fp);

    /* Update car status in cars.txt to 0 (Rented) */
    updateCarAvailability(carId, 0);

    displayBanner("BOOKING CONFIRMATION");
    printf("  Booking ID     : %d\n", newBooking.bookingId);
    printf("  Customer       : %s\n", cust.name);
    printf("  Car            : %s %s\n", car.brand, car.model);
    printf("  Rental Days    : %d\n", newBooking.days);
    printf("  Price / Day    : Rs. %.2f\n", newBooking.dailyRate);
    printf("  Total Amount   : Rs. %.2f\n", newBooking.totalAmount);
    printf("  Booking Status : CONFIRMED (Active)\n");
    printf("==================================================\n");
    printf("[✓] Car successfully rented! Saved to %s.\n", BOOKINGS_FILE);
    pauseScreen();
}

/* READ: Filter bookings for logged-in customer */
void viewMyBookings(int customerId) {
    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No bookings found in %s.\n", BOOKINGS_FILE);
        return;
    }

    displayBanner("MY BOOKING HISTORY");
    printf("%-10s %-8s %-20s %-6s %-12s %-12s\n",
           "Booking ID", "Car ID", "Car Name", "Days", "Total (Rs.)", "Status");
    printf("-----------------------------------------------------------------------------\n");

    char line[256];
    struct Booking b;
    struct Car c;
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            if (b.customerId == customerId) {
                char carName[40] = "Unknown";
                if (getCarById(b.carId, &c)) {
                    snprintf(carName, sizeof(carName), "%s %s", c.brand, c.model);
                }

                const char *statusStr = "Unknown";
                if (b.status == 0) statusStr = "ACTIVE";
                else if (b.status == 1) statusStr = "COMPLETED";
                else if (b.status == 2) statusStr = "CANCELLED";

                printf("%-10d %-8d %-20s %-6d %-12.2f %-12s\n",
                       b.bookingId, b.carId, carName, b.days, b.totalAmount + b.lateFee, statusStr);
                count++;
            }
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("You have not made any bookings yet.\n");
    }
    printf("-----------------------------------------------------------------------------\n");
    printf("Total records: %d\n", count);
}

/* UPDATE: Cancels an active booking in bookings.txt & restores car in cars.txt */
void cancelBooking(int customerId) {
    int bookingId;
    viewMyBookings(customerId);

    displayBanner("CANCEL A BOOKING");
    printf("Enter Booking ID to cancel (or 0 to exit): ");
    if (scanf("%d", &bookingId) != 1 || bookingId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    FILE *fp = fopen(BOOKINGS_FILE, "r");
    FILE *tempFp = fopen("temp_bookings.txt", "w");
    if (fp == NULL || tempFp == NULL) {
        if (fp) fclose(fp);
        if (tempFp) fclose(tempFp);
        printf("[!] No booking records found.\n");
        pauseScreen();
        return;
    }

    char line[256];
    struct Booking b;
    int found = 0;
    int carToRelease = -1;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            if (b.bookingId == bookingId && b.customerId == customerId) {
                found = 1;
                if (b.status != 0) {
                    printf("[!] This booking cannot be cancelled because it is already %s.\n",
                           b.status == 1 ? "COMPLETED" : "CANCELLED");
                    fprintf(tempFp, "%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%d\n",
                            b.bookingId, b.customerId, b.carId, b.days,
                            b.actualDays, b.dailyRate, b.totalAmount, b.lateFee, b.status);
                    continue;
                }

                printf("\nAre you sure you want to cancel Booking #%d? (Y/N): ", bookingId);
                char confirm = 'n';
                scanf(" %c", &confirm);
                clearInputBuffer();

                if (confirm == 'y' || confirm == 'Y') {
                    b.status = 2; /* Cancelled */
                    carToRelease = b.carId;
                    printf("\n[✓] Booking #%d has been cancelled successfully.\n", bookingId);
                } else {
                    printf("\n[!] Cancellation aborted.\n");
                }
            }
            fprintf(tempFp, "%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%d\n",
                    b.bookingId, b.customerId, b.carId, b.days,
                    b.actualDays, b.dailyRate, b.totalAmount, b.lateFee, b.status);
        }
    }

    fclose(fp);
    fclose(tempFp);

    remove(BOOKINGS_FILE);
    rename("temp_bookings.txt", BOOKINGS_FILE);

    if (carToRelease != -1) {
        updateCarAvailability(carToRelease, 1);
        printf("[✓] Car ID #%d is now marked as Available in %s.\n", carToRelease, CARS_FILE);
    } else if (!found) {
        printf("[!] Booking ID %d not found under your account.\n", bookingId);
    }
    pauseScreen();
}

/* UPDATE: Returns car, applies late fees in bookings.txt, and marks car available in cars.txt */
void returnCar(int customerId) {
    int bookingId;
    viewMyBookings(customerId);

    displayBanner("RETURN RENTED CAR");
    printf("Enter Booking ID to return (or 0 to exit): ");
    if (scanf("%d", &bookingId) != 1 || bookingId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    FILE *fp = fopen(BOOKINGS_FILE, "r");
    FILE *tempFp = fopen("temp_bookings.txt", "w");
    if (fp == NULL || tempFp == NULL) {
        if (fp) fclose(fp);
        if (tempFp) fclose(tempFp);
        printf("[!] No bookings found.\n");
        pauseScreen();
        return;
    }

    char line[256];
    struct Booking b;
    struct Booking completedBooking;
    int found = 0;
    int returnedSuccess = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            if (b.bookingId == bookingId && b.customerId == customerId) {
                found = 1;
                if (b.status != 0) {
                    printf("[!] Booking #%d is not active (Already %s).\n",
                           bookingId, b.status == 1 ? "Returned" : "Cancelled");
                } else {
                    int actualDays;
                    printf("Originally booked rental days : %d\n", b.days);
                    printf("Enter ACTUAL days the car was kept: ");
                    if (scanf("%d", &actualDays) != 1 || actualDays <= 0) {
                        printf("[!] Invalid days entered.\n");
                        clearInputBuffer();
                    } else {
                        clearInputBuffer();
                        b.actualDays = actualDays;
                        b.lateFee = 0.0f;

                        if (actualDays > b.days) {
                            int overdue = actualDays - b.days;
                            b.lateFee = (float)overdue * LATE_FEE_PER_DAY;
                            printf("\n[!] Vehicle returned %d day(s) late.\n", overdue);
                            printf("    Late fee penalty applied: %d x Rs. %.2f = Rs. %.2f\n",
                                   overdue, LATE_FEE_PER_DAY, b.lateFee);
                        }

                        b.status = 1; /* Completed */
                        completedBooking = b;
                        returnedSuccess = 1;
                    }
                }
            }
            fprintf(tempFp, "%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%d\n",
                    b.bookingId, b.customerId, b.carId, b.days,
                    b.actualDays, b.dailyRate, b.totalAmount, b.lateFee, b.status);
        }
    }

    fclose(fp);
    fclose(tempFp);

    remove(BOOKINGS_FILE);
    rename("temp_bookings.txt", BOOKINGS_FILE);

    if (returnedSuccess) {
        updateCarAvailability(completedBooking.carId, 1);
        printf("\n[✓] Car successfully returned and marked Available in %s!\n", CARS_FILE);

        struct Customer cust;
        struct Car car;
        getCustomerById(completedBooking.customerId, &cust);
        getCarById(completedBooking.carId, &car);
        printBillSlip(&completedBooking, &cust, &car);
    } else if (!found) {
        printf("[!] Booking ID %d not found under your account.\n", bookingId);
    }
    pauseScreen();
}

/* Print Standard Invoice / Bill */
void printBillSlip(const struct Booking *b, const struct Customer *cust, const struct Car *car) {
    float grandTotal = b->totalAmount + b->lateFee;

    printf("\n==================================================\n");
    printf("                  RENTAL BILL                     \n");
    printf("==================================================\n");
    printf("  Invoice / Booking ID : %d\n", b->bookingId);
    printf("  Customer ID          : %d\n", cust->customerId);
    printf("  Customer Name        : %s\n", cust->name);
    printf("  Phone Number         : %s\n", cust->phone);
    printf("--------------------------------------------------\n");
    printf("  Car ID               : %d\n", car->carId);
    printf("  Vehicle              : %s %s (%s)\n", car->brand, car->model, car->type);
    printf("  Planned Rental Days  : %d\n", b->days);
    printf("  Actual Rental Days   : %d\n", b->actualDays);
    printf("  Daily Rental Rate    : Rs. %.2f\n", b->dailyRate);
    printf("--------------------------------------------------\n");
    printf("  Base Rental Amount   : Rs. %.2f\n", b->totalAmount);
    printf("  Late Charges         : Rs. %.2f\n", b->lateFee);
    printf("--------------------------------------------------\n");
    printf("  FINAL TOTAL AMOUNT   : Rs. %.2f\n", grandTotal);
    printf("  Status               : %s\n",
           b->status == 1 ? "PAID & COMPLETED" : (b->status == 0 ? "ACTIVE (UNRETURNED)" : "CANCELLED"));
    printf("==================================================\n");
}

/* Customer requests bill for any of their bookings */
void generateBillForCustomer(int customerId) {
    int bookingId;
    viewMyBookings(customerId);

    displayBanner("GENERATE RENTAL BILL");
    printf("Enter Booking ID to generate bill (or 0 to exit): ");
    if (scanf("%d", &bookingId) != 1 || bookingId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No bookings found in %s.\n", BOOKINGS_FILE);
        pauseScreen();
        return;
    }

    char line[256];
    struct Booking b;
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            if (b.bookingId == bookingId && b.customerId == customerId) {
                found = 1;
                struct Customer cust;
                struct Car car;
                getCustomerById(b.customerId, &cust);
                getCarById(b.carId, &car);
                printBillSlip(&b, &cust, &car);
                break;
            }
        }
    }
    fclose(fp);

    if (!found) {
        printf("[!] Booking ID %d not found under your account.\n", bookingId);
    }
    pauseScreen();
}

/* ============================================================================
 * ADMIN MODULE IMPLEMENTATION
 * ============================================================================ */

/* Admin Login Verification */
int adminLogin(void) {
    char user[30];
    char pass[30];

    displayBanner("ADMINISTRATOR LOGIN");
    printf("Enter Admin Username: ");
    getString(user, sizeof(user));

    printf("Enter Admin Password: ");
    getString(pass, sizeof(pass));

    if (strcmp(user, ADMIN_USERNAME) == 0 && strcmp(pass, ADMIN_PASSWORD) == 0) {
        printf("\n[✓] Admin Authentication Successful! Welcome, Administrator.\n");
        pauseScreen();
        return 1;
    }

    printf("\n[!] Access Denied: Invalid Admin Credentials.\n");
    pauseScreen();
    return 0;
}

/* Admin Dashboard */
void adminDashboard(void) {
    int choice = 0;
    while (1) {
        if (feof(stdin)) return;

        displayBanner("ADMINISTRATOR DASHBOARD");
        printf("  1. Add New Car (Create)\n");
        printf("  2. View All Cars (Read)\n");
        printf("  3. Search Cars (Read/Filter)\n");
        printf("  4. Update Car Information (Update)\n");
        printf("  5. Delete Car (Delete)\n");
        printf("  6. View Registered Customers\n");
        printf("  7. View All Rental Bookings\n");
        printf("  8. View Revenue Analytics\n");
        printf("  9. Logout\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice (1-9): ");

        int res = scanf("%d", &choice);
        if (res == EOF) return;
        if (res != 1) {
            printf("\n[!] Invalid input! Please enter a number.\n");
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                addCar();
                break;
            case 2:
                viewAllCars(0);
                pauseScreen();
                break;
            case 3:
                searchCarSubmenu();
                break;
            case 4:
                updateCar();
                break;
            case 5:
                deleteCar();
                break;
            case 6:
                viewAllCustomers();
                pauseScreen();
                break;
            case 7:
                viewAllBookings();
                pauseScreen();
                break;
            case 8:
                viewRevenueSummary();
                pauseScreen();
                break;
            case 9:
                printf("\n[✓] Admin logged out successfully.\n");
                pauseScreen();
                return;
            default:
                printf("\n[!] Invalid choice! Select between 1 and 9.\n");
                pauseScreen();
                break;
        }
    }
}

/* CREATE (Car): Appends a new car record to cars.txt */
void addCar(void) {
    struct Car newCar;
    displayBanner("ADD NEW CAR TO INVENTORY (CREATE)");

    printf("Enter New Car ID: ");
    if (scanf("%d", &newCar.carId) != 1 || newCar.carId <= 0) {
        printf("[!] Invalid Car ID.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    if (isCarIdExists(newCar.carId)) {
        printf("[!] Error: Car ID %d already exists! Duplicate IDs are not allowed.\n", newCar.carId);
        pauseScreen();
        return;
    }

    printf("Enter Brand (e.g. Hyundai, Tata): ");
    getString(newCar.brand, sizeof(newCar.brand));

    printf("Enter Model (e.g. Creta, Nexon): ");
    getString(newCar.model, sizeof(newCar.model));

    printf("Enter Manufacturing Year: ");
    if (scanf("%d", &newCar.year) != 1 || newCar.year < 1990 || newCar.year > 2030) {
        printf("[!] Invalid Year.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    printf("Enter Car Type (SUV / Sedan / Hatchback): ");
    getString(newCar.type, sizeof(newCar.type));

    printf("Enter Fuel Type (Petrol / Diesel / EV): ");
    getString(newCar.fuelType, sizeof(newCar.fuelType));

    printf("Enter Number of Seats (e.g. 5, 7): ");
    if (scanf("%d", &newCar.seats) != 1 || newCar.seats <= 0) {
        printf("[!] Invalid seats.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    printf("Enter Daily Rental Price (in Rupees): ");
    if (scanf("%f", &newCar.pricePerDay) != 1 || newCar.pricePerDay <= 0.0f) {
        printf("[!] Invalid price.\n");
        clearInputBuffer();
        pauseScreen();
        return;
    }
    clearInputBuffer();

    newCar.available = 1;

    FILE *fp = fopen(CARS_FILE, "a");
    if (fp == NULL) {
        printf("[!] Error opening %s for appending.\n", CARS_FILE);
        pauseScreen();
        return;
    }

    /* Append formatted line */
    fprintf(fp, "%d|%s|%s|%d|%s|%s|%d|%.2f|%d\n",
            newCar.carId, newCar.brand, newCar.model, newCar.year,
            newCar.type, newCar.fuelType, newCar.seats, newCar.pricePerDay, newCar.available);
    fclose(fp);

    printf("\n[✓] Car #%d (%s %s) appended to %s as Available!\n",
           newCar.carId, newCar.brand, newCar.model, CARS_FILE);
    pauseScreen();
}

/* READ (Cars): Views all cars from cars.txt */
void viewAllCars(int onlyAvailable) {
    FILE *fp = fopen(CARS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No car inventory records found in %s.\n", CARS_FILE);
        return;
    }

    displayBanner(onlyAvailable ? "AVAILABLE CARS" : "COMPLETE CAR INVENTORY");
    printf("%-6s %-12s %-12s %-6s %-10s %-8s %-6s %-10s %-12s\n",
           "ID", "Brand", "Model", "Year", "Type", "Fuel", "Seats", "Rate/Day", "Status");
    printf("----------------------------------------------------------------------------------------\n");

    char line[256];
    struct Car c;
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (!onlyAvailable || c.available == 1) {
                printf("%-6d %-12s %-12s %-6d %-10s %-8s %-6d Rs.%-7.2f %s\n",
                       c.carId, c.brand, c.model, c.year, c.type, c.fuelType,
                       c.seats, c.pricePerDay, (c.available ? "Available" : "Rented"));
                count++;
            }
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("No records found in inventory.\n");
    }
    printf("----------------------------------------------------------------------------------------\n");
    printf("Total cars listed: %d\n", count);
}

/* UPDATE (Car): Updates an existing car in cars.txt */
void updateCar(void) {
    int carId;
    viewAllCars(0);

    displayBanner("UPDATE CAR RECORD (UPDATE)");
    printf("Enter Car ID to update (or 0 to cancel): ");
    if (scanf("%d", &carId) != 1 || carId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    struct Car car;
    if (!getCarById(carId, &car)) {
        printf("[!] Car ID %d not found in %s.\n", carId, CARS_FILE);
        pauseScreen();
        return;
    }

    printf("\nUpdating Car: %s %s (ID: %d)\n", car.brand, car.model, car.carId);
    printf("Press [Enter] on text fields to keep current values.\n\n");

    char buffer[50];

    printf("Brand [%s]: ", car.brand);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strncpy(car.brand, buffer, sizeof(car.brand) - 1);

    printf("Model [%s]: ", car.model);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strncpy(car.model, buffer, sizeof(car.model) - 1);

    printf("Car Type [%s]: ", car.type);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strncpy(car.type, buffer, sizeof(car.type) - 1);

    printf("Fuel Type [%s]: ", car.fuelType);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strncpy(car.fuelType, buffer, sizeof(car.fuelType) - 1);

    printf("Price Per Day [Rs. %.2f]: ", car.pricePerDay);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        float newPrice = (float)atof(buffer);
        if (newPrice > 0.0f) car.pricePerDay = newPrice;
    }

    printf("Availability (1 = Available, 0 = Rented) [%d]: ", car.available);
    getString(buffer, sizeof(buffer));
    if (strlen(buffer) > 0) {
        int newAvail = atoi(buffer);
        if (newAvail == 0 || newAvail == 1) car.available = newAvail;
    }

    if (updateCarRecord(car)) {
        printf("\n[✓] Car record for ID %d updated successfully in %s!\n", car.carId, CARS_FILE);
    } else {
        printf("\n[!] Failed to update car record in file.\n");
    }
    pauseScreen();
}

/* DELETE (Car): Safely removes a car line from cars.txt */
void deleteCar(void) {
    int carId;
    viewAllCars(0);

    displayBanner("DELETE CAR FROM INVENTORY (DELETE)");
    printf("Enter Car ID to delete (or 0 to cancel): ");
    if (scanf("%d", &carId) != 1 || carId == 0) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    struct Car target;
    if (!getCarById(carId, &target)) {
        printf("[!] Car ID %d not found in %s.\n", carId, CARS_FILE);
        pauseScreen();
        return;
    }

    /* Safety Check: Ensure car is not currently rented */
    if (target.available == 0) {
        printf("\n[!] SECURITY ALERT: Car #%d is currently RENTED by an active customer.\n", carId);
        printf("    You cannot delete an actively rented vehicle. Must be returned first.\n");
        pauseScreen();
        return;
    }

    printf("\nAre you sure you want to permanently delete Car #%d (%s %s)? (Y/N): ",
           target.carId, target.brand, target.model);
    char confirm = 'n';
    scanf(" %c", &confirm);
    clearInputBuffer();

    if (confirm != 'y' && confirm != 'Y') {
        printf("[!] Deletion cancelled.\n");
        pauseScreen();
        return;
    }

    /* Rewrite cars.txt excluding this car line */
    FILE *fp = fopen(CARS_FILE, "r");
    FILE *tempFp = fopen("temp_cars.txt", "w");

    if (fp == NULL || tempFp == NULL) {
        printf("[!] File error during deletion.\n");
        if (fp) fclose(fp);
        if (tempFp) fclose(tempFp);
        pauseScreen();
        return;
    }

    char line[256];
    struct Car c;
    int deleted = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCarLine(line, &c)) {
            if (c.carId == carId) {
                deleted = 1;
            } else {
                fprintf(tempFp, "%s", line);
            }
        }
    }
    fclose(fp);
    fclose(tempFp);

    if (deleted) {
        remove(CARS_FILE);
        rename("temp_cars.txt", CARS_FILE);
        printf("\n[✓] Car #%d was permanently deleted from %s.\n", carId, CARS_FILE);
    } else {
        remove("temp_cars.txt");
        printf("\n[!] Car could not be deleted.\n");
    }
    pauseScreen();
}

/* READ: Views registered customers from customers.txt */
void viewAllCustomers(void) {
    FILE *fp = fopen(CUSTOMERS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No registered customer records found in %s.\n", CUSTOMERS_FILE);
        return;
    }

    displayBanner("REGISTERED CUSTOMERS");
    printf("%-8s %-22s %-16s %-25s\n",
           "Cust ID", "Full Name", "Phone", "Email");
    printf("-----------------------------------------------------------------------------\n");

    char line[256];
    struct Customer c;
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseCustomerLine(line, &c)) {
            printf("%-8d %-22s %-16s %-25s\n",
                   c.customerId, c.name, c.phone, c.email);
            count++;
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("No customers registered yet.\n");
    }
    printf("-----------------------------------------------------------------------------\n");
    printf("Total customers registered: %d\n", count);
}

/* READ: Views all bookings from bookings.txt */
void viewAllBookings(void) {
    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No bookings found in %s.\n", BOOKINGS_FILE);
        return;
    }

    displayBanner("ALL RENTAL BOOKINGS");
    printf("%-8s %-8s %-8s %-6s %-6s %-12s %-10s %-12s\n",
           "Book ID", "Cust ID", "Car ID", "Plan", "Act", "Base (Rs.)", "Late Fee", "Status");
    printf("------------------------------------------------------------------------------------\n");

    char line[256];
    struct Booking b;
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            const char *statusStr = "Unknown";
            if (b.status == 0) statusStr = "ACTIVE";
            else if (b.status == 1) statusStr = "COMPLETED";
            else if (b.status == 2) statusStr = "CANCELLED";

            printf("%-8d %-8d %-8d %-6d %-6d %-12.2f %-10.2f %-12s\n",
                   b.bookingId, b.customerId, b.carId, b.days, b.actualDays,
                   b.totalAmount, b.lateFee, statusStr);
            count++;
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("No bookings made yet in the system.\n");
    }
    printf("------------------------------------------------------------------------------------\n");
    printf("Total bookings: %d\n", count);
}

/* READ & AGGREGATE: Computes revenue from bookings.txt */
void viewRevenueSummary(void) {
    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL) {
        printf("[!] No bookings found in %s to compute revenue.\n", BOOKINGS_FILE);
        return;
    }

    displayBanner("FINANCIAL REVENUE & ANALYTICS");

    char line[256];
    struct Booking b;
    int totalBookings = 0;
    int activeBookings = 0;
    int completedBookings = 0;
    int cancelledBookings = 0;

    float realizedRevenue = 0.0f;
    float pendingRevenue = 0.0f;
    float totalLateFees = 0.0f;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseBookingLine(line, &b)) {
            totalBookings++;
            if (b.status == 0) {
                activeBookings++;
                pendingRevenue += b.totalAmount;
            } else if (b.status == 1) {
                completedBookings++;
                realizedRevenue += b.totalAmount;
                totalLateFees += b.lateFee;
            } else if (b.status == 2) {
                cancelledBookings++;
            }
        }
    }
    fclose(fp);

    float grandRealized = realizedRevenue + totalLateFees;

    printf("  Total Bookings Processed    : %d\n", totalBookings);
    printf("  Completed / Returned Rentals: %d\n", completedBookings);
    printf("  Active Ongoing Rentals      : %d\n", activeBookings);
    printf("  Cancelled Bookings          : %d\n", cancelledBookings);
    printf("--------------------------------------------------\n");
    printf("  Base Rent Collected         : Rs. %.2f\n", realizedRevenue);
    printf("  Late Return Charges Accrued : Rs. %.2f\n", totalLateFees);
    printf("  TOTAL REALIZED REVENUE      : Rs. %.2f\n", grandRealized);
    printf("--------------------------------------------------\n");
    printf("  Pending / Active Pipeline   : Rs. %.2f\n", pendingRevenue);
    printf("==================================================\n");
}
