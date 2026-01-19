#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For tolower

#define TRAINS 20
#define CLASSES 5
#define SEATS_PER_CLASS 20
#define MAX_USERS 100 // Maximum number of users the system can handle

// Global arrays for class names and payment types
const char* classNames[CLASSES] = {
    "Sleeper", "Chair Car", "3rd AC", "2nd AC", "1st AC"
};

// --- Structs for Data ---
typedef struct {
    int seatNumber;
    char passengerName[50];
    int isReserved;
} Seat;

typedef struct {
    char className[20];
    int fare;
    Seat seats[SEATS_PER_CLASS];
} TrainClass;

typedef struct {
    char trainName[50];
    char route[200];
    TrainClass classes[CLASSES];
} Train;

typedef struct {
    char username[50];
    char password_hash[50]; // Stores a simple hash of the password
} User;

typedef enum {
    PAYMENT_CASH,
    PAYMENT_CARD,
    PAYMENT_UPI,
    PAYMENT_COUNT
} PaymentType;

const char* paymentTypeNames[] = {
    "Cash",
    "Card",
    "UPI"
};

// Global array for train routes (fixed data)
const char *trainRoutes[TRAINS][10] = {
    {"New Delhi", "Tughlakabad", "Agra Cantt", "Gwalior", "Jhansi", "Bhopal", NULL},
    {"Howrah", "Asansol", "Dhanbad", "Gaya", "Pt. Deen Dayal Upadhyaya", "Kanpur Central", "New Delhi", NULL},
    {"Mumbai CSMT", "Nasik Road", "Bhusaval", "Nagpur", "Raipur", "Bilaspur", "Howrah", NULL},
    {"Chennai", "Katpadi", "Bengaluru", "Mandya", "Mysuru", NULL},
    {"New Delhi", "Aligarh", "Kanpur", "Prayagraj", "Patna", NULL},
    {"Lucknow", "Bareilly", "Moradabad", "Ghaziabad", "New Delhi", NULL},
    {"Mumbai", "Surat", "Vadodara", "Ahmedabad", "Jaipur", NULL},
    {"Pune", "Solapur", "Wadi", "Guntakal", "Bengaluru", NULL},
    {"Guwahati", "New Jalpaiguri", "Katihar", "Patna", "Varanasi", NULL},
    {"Bhopal", "Itarsi", "Jabalpur", "Satna", "New Delhi", NULL},
    {"Delhi", "Rewari", "Jaipur", "Ajmer", "Udaipur", NULL},
    {"Bangalore", "Tumkur", "Chitradurga", "Hospet", "Ballari", "Raichur", NULL},
    {"Hyderabad", "Warangal", "Vijayawala", "Guntur", "Chennai", NULL},
    {"Kolkata", "Durgapur", "Asansol", "Dhanbad", "Gaya", "Varanasi", NULL},
    {"Ahmedabad", "Udaipur", "Ajmer", "Jaipur", "Delhi", NULL},
    {"Bhubaneswar", "Cuttack", "Sambalpur", "Raigarh", "Bilaspur", "Nagpur", NULL},
    {"Secunderabad", "Kazipet", "Nagpur", "Itarsi", "Bhopal", "Delhi", NULL},
    {"Thiruvananthapuram", "Ernakulam", "Coimbatore", "Salem", "Bangalore", NULL},
    {"Chandigarh", "Ambala", "Ludhiana", "Jalandhar", "Amritsar", NULL},
    {"Ranchi", "Bokaro", "Asansol", "Bardhaman", "Kolkata", NULL}
};

// Global array to store all registered users
User users[MAX_USERS];
int userCount = 0; // Current number of registered users

// --- Function Prototypes ---
void showMenu();
void flushInput();
void hashPassword(const char *password, char *hashed_password);
int login();
int signup();
void showLoginSignupMenu();
void initializeTrains(Train trains[], int totalTrains);
int selectPaymentType();
int getClassIndex();
void selectTrain(Train trains[], int *trainIndex);
int findSeatIndex(int seatNumber);
int validateRoute(int trainIndex, char *from, char *to);
void reserveSeat(Train trains[]);
void cancelReservation(Train trains[]);
void displayReservedSeats(Train trains[]);
void displaySeatChart(Train trains[]);

// --- Functions for Data Persistence ---
void saveData(Train trains[]);
void loadData(Train trains[]);


// --- Utility Functions ---

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// A very simple XOR-based hash for demonstration. NOT for production!
// For real applications, use secure hashing libraries (e.g., bcrypt, Argon2).
void hashPassword(const char *password, char *hashed_password) {
    int i;
    for (i = 0; password[i] != '\0' && i < 49; i++) {
        hashed_password[i] = password[i] ^ 0xAA; // XOR with a constant byte
    }
    hashed_password[i] = '\0';
}

// Function to handle user signup (now supports multiple users)
int signup() {
    char newUsername[50];
    char newPassword[50];
    char hashed_newPassword[50];

    printf("\n==== Train Reservation System Signup ====\n");

    if (userCount >= MAX_USERS) {
        printf("Maximum number of users reached. Cannot create new account.\n");
        return 0;
    }

    printf("Enter new username: ");
    if (fgets(newUsername, sizeof(newUsername), stdin) == NULL) {
        return 0;
    }
    newUsername[strcspn(newUsername, "\n")] = '\0';

    // Check if username already exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(newUsername, users[i].username) == 0) {
            printf("Error: Username '%s' already taken. Please choose another.\n", newUsername);
            return 0;
        }
    }

    printf("Enter new password: ");
    if (fgets(newPassword, sizeof(newPassword), stdin) == NULL) {
        return 0;
    }
    newPassword[strcspn(newPassword, "\n")] = '\0';

    // Store new user
    strcpy(users[userCount].username, newUsername);
    hashPassword(newPassword, users[userCount].password_hash);
    userCount++;

    printf("Account for '%s' created successfully!\n", newUsername);
    return 1;
}

// Function to handle user login (now authenticates against multiple users)
int login() {
    char username[50];
    char password[50];
    char hashed_password_input[50];

    if (userCount == 0) {
        printf("\nNo accounts found. Please sign up first.\n");
        return 0;
    }

    printf("\n==== Train Reservation System Login ====\n");

    for (int attempts = 3; attempts > 0; attempts--) {
        printf("Username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) return 0;
        username[strcspn(username, "\n")] = '\0';

        printf("Password: ");
        if (fgets(password, sizeof(password), stdin) == NULL) return 0;
        password[strcspn(password, "\n")] = '\0';

        hashPassword(password, hashed_password_input); // Hash the input password for comparison

        int loggedIn = 0;
        for (int i = 0; i < userCount; i++) {
            if (strcmp(username, users[i].username) == 0 && strcmp(hashed_password_input, users[i].password_hash) == 0) {
                printf("Login successful! Welcome, %s.\n", username);
                loggedIn = 1;
                break;
            }
        }

        if (loggedIn) {
            return 1;
        } else {
            printf("Invalid credentials. Attempts left: %d\n", attempts - 1);
        }
    }
    printf("Too many failed attempts. Exiting...\n");
    return 0;
}

// Displays the login/signup menu
void showLoginSignupMenu() {
    printf("\n--- Welcome to Train Reservation System ---\n");
    printf("1. Login\n");
    printf("2. Sign Up\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}

// Initializes train data, including class names, fares, and initial seat availability.
// This function is called only if no saved train data is found.
void initializeTrains(Train trains[], int totalTrains) {
    for (int i = 0; i < totalTrains; i++) {
        // Class names and fares are static
        strcpy(trains[i].classes[0].className, "Sleeper");
        trains[i].classes[0].fare = 200;

        strcpy(trains[i].classes[1].className, "Chair Car");
        trains[i].classes[1].fare = 350;

        strcpy(trains[i].classes[2].className, "3rd AC");
        trains[i].classes[2].fare = 500;

        strcpy(trains[i].classes[3].className, "2nd AC");
        trains[i].classes[3].fare = 750;

        strcpy(trains[i].classes[4].className, "1st AC");
        trains[i].classes[4].fare = 1000;

        // All seats are initially unreserved
        for (int c = 0; c < CLASSES; c++) {
            for (int s = 0; s < SEATS_PER_CLASS; s++) {
                trains[i].classes[c].seats[s].seatNumber = s + 1;
                trains[i].classes[c].seats[s].isReserved = 0;
                trains[i].classes[c].seats[s].passengerName[0] = '\0';
            }
        }
    }
}

// Displays the main menu options to the user.
void showMenu() {
    printf("\n--- Train Reservation System ---\n");
    printf("1. Reserve Seat\n");
    printf("2. Cancel Reservation\n");
    printf("3. Display Seat Chart\n");
    printf("4. Display Reserved Seats Only\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

// Allows the user to select a payment method.
int selectPaymentType() {
    printf("Select Payment Method:\n");
    for (int i = 0; i < PAYMENT_COUNT; i++) {
        printf("%d. %s\n", i + 1, paymentTypeNames[i]);
    }
    printf("Enter choice: ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        flushInput();
        return -1;
    }
    flushInput();
    if (choice < 1 || choice > PAYMENT_COUNT) {
        return -1;
    }
    return choice - 1;
}

// Gets the user's class selection and returns its zero-based index.
int getClassIndex() {
    printf("Select Class:\n");
    for (int i = 0; i < CLASSES; i++) {
        printf("%d. %s\n", i + 1, classNames[i]);
    }
    printf("Enter class number: ");
    int c;
    if (scanf("%d", &c) != 1 || c < 1 || c > CLASSES) {
        flushInput();
        printf("Invalid class selection.\n");
        return -1;
    }
    flushInput();
    return c - 1;
}

// Prompts the user to select a train and stores its index.
void selectTrain(Train trains[], int *trainIndex) {
    printf("Select Train:\n");
    for (int i = 0; i < TRAINS; i++) {
        printf("%2d. %-25s (%s)\n", i + 1, trains[i].trainName, trains[i].route);
    }
    printf("Enter train number: ");
    int selectedTrainNum;
    if (scanf("%d", &selectedTrainNum) != 1 || selectedTrainNum < 1 || selectedTrainNum > TRAINS) {
        printf("Invalid train selection.\n");
        flushInput();
        *trainIndex = -1;
        return;
    }
    flushInput();
    *trainIndex = selectedTrainNum - 1;
}

// Validates a given seat number and returns its zero-based index.
int findSeatIndex(int seatNumber) {
    if (seatNumber < 1 || seatNumber > SEATS_PER_CLASS)
        return -1;
    return seatNumber - 1;
}

// Checks if boarding and destination stations are valid and in the correct order for a given train.
int validateRoute(int trainIndex, char *from, char *to) {
    int fromIndex = -1, toIndex = -1;
    char lowerFrom[50], lowerTo[50];

    for (int i = 0; from[i]; i++) lowerFrom[i] = tolower(from[i]);
    lowerFrom[strlen(from)] = '\0';
    for (int i = 0; to[i]; i++) lowerTo[i] = tolower(to[i]);
    lowerTo[strlen(to)] = '\0';

    for (int i = 0; i < 10 && trainRoutes[trainIndex][i] != NULL; i++) {
        char currentStationLower[50];
        for (int j = 0; trainRoutes[trainIndex][i][j]; j++) currentStationLower[j] = tolower(trainRoutes[trainIndex][i][j]);
        currentStationLower[strlen(trainRoutes[trainIndex][i])] = '\0';

        if (strcmp(currentStationLower, lowerFrom) == 0) {
            fromIndex = i;
        }
        if (strcmp(currentStationLower, lowerTo) == 0) {
            toIndex = i;
        }
    }
    if (fromIndex == -1 || toIndex == -1) {
        return 0;
    }
    return fromIndex < toIndex;
}

// Handles the seat reservation process, including multiple seat bookings and payment.
void reserveSeat(Train trains[]) {
    int trainIndex;
    selectTrain(trains, &trainIndex);
    if (trainIndex == -1) return;

    printf("\nStations for %s:\n", trains[trainIndex].trainName);
    for (int i = 0; trainRoutes[trainIndex][i] != NULL; i++) {
        printf("    - %s\n", trainRoutes[trainIndex][i]);
    }

    char from[50], to[50];
    printf("\nEnter boarding station: ");
    if (fgets(from, sizeof(from), stdin) == NULL) return;
    from[strcspn(from, "\n")] = '\0';

    printf("Enter destination station: ");
    if (fgets(to, sizeof(to), stdin) == NULL) return;
    to[strcspn(to, "\n")] = '\0';

    if (!validateRoute(trainIndex, from, to)) {
        printf("Invalid route for this train or stations are in incorrect order. Reservation cancelled.\n");
        return;
    }

    int classIndex = getClassIndex();
    if (classIndex == -1) {
        printf("Invalid class choice.\n");
        return;
    }

    int numSeats;
    printf("Enter number of seats to reserve (1-%d): ", SEATS_PER_CLASS);
    if (scanf("%d", &numSeats) != 1 || numSeats < 1 || numSeats > SEATS_PER_CLASS) {
        flushInput();
        printf("Invalid number of seats.\n");
        return;
    }
    flushInput();

    int selectedSeatIndices[SEATS_PER_CLASS];
    int currentReserved = 0;

    for (int i = 0; i < numSeats; i++) {
        int seatNum;
        printf("Enter seat number #%d (1-%d): ", i + 1, SEATS_PER_CLASS);
        if (scanf("%d", &seatNum) != 1) {
            flushInput();
            printf("Invalid input. Try again.\n");
            i--;
            continue;
        }
        flushInput();

        int seatIndex = findSeatIndex(seatNum);
        if (seatIndex == -1) {
            printf("Seat number out of range. Try again.\n");
            i--;
            continue;
        }

        Seat *seat = &trains[trainIndex].classes[classIndex].seats[seatIndex];
        if (seat->isReserved) {
            printf("Seat %d already reserved. Choose another.\n", seatNum);
            i--;
            continue;
        }

        printf("Enter passenger name for seat %d: ", seatNum);
        fgets(seat->passengerName, sizeof(seat->passengerName), stdin);
        seat->passengerName[strcspn(seat->passengerName, "\n")] = '\0';

        seat->isReserved = 1;
        selectedSeatIndices[currentReserved++] = seatIndex;
    }

    if (currentReserved > 0) {
        int totalFare = currentReserved * trains[trainIndex].classes[classIndex].fare;
        printf("Total Fare for %d seat(s): Rs.%d\n", currentReserved, totalFare);

        int paymentMethod = selectPaymentType();
        if (paymentMethod == -1) {
            printf("Payment failed or cancelled. Rolling back reservations.\n");
            for (int i = 0; i < currentReserved; i++) {
                int seatIndexToRollback = selectedSeatIndices[i];
                Seat *seat = &trains[trainIndex].classes[classIndex].seats[seatIndexToRollback];
                seat->isReserved = 0;
                seat->passengerName[0] = '\0';
            }
            return;
        }

        printf("Payment Method: %s\n", paymentTypeNames[paymentMethod]);
        printf("Reservation successful for %d seat(s) on %s in %s class.\n",
               currentReserved, trains[trainIndex].trainName, trains[trainIndex].classes[classIndex].className);
        saveData(trains); // Save data after successful reservation
    } else {
        printf("No seats reserved.\n");
    }
}

// Handles the cancellation of an existing reservation.
void cancelReservation(Train trains[]) {
    int trainIndex;
    selectTrain(trains, &trainIndex);
    if (trainIndex == -1) return;

    int classIndex = getClassIndex();
    if (classIndex == -1) return;

    int seatNum;
    printf("Enter seat number to cancel (1-%d): ", SEATS_PER_CLASS);
    if (scanf("%d", &seatNum) != 1) {
        flushInput();
        printf("Invalid seat number input.\n");
        return;
    }
    flushInput();

    int seatIndex = findSeatIndex(seatNum);
    if (seatIndex == -1) {
        printf("Invalid seat number.\n");
        return;
    }

    Seat *seat = &trains[trainIndex].classes[classIndex].seats[seatIndex];
    if (!seat->isReserved) {
        printf("Seat is not reserved.\n");
        return;
    }

    seat->isReserved = 0;
    seat->passengerName[0] = '\0';
    printf("Reservation cancelled for seat %d in %s class on train %s.\n", seatNum,
           trains[trainIndex].classes[classIndex].className, trains[trainIndex].trainName);
    saveData(trains); // Save data after successful cancellation
}

// Displays only the reserved seats for a selected train and class, including passenger names.
void displayReservedSeats(Train trains[]) {
    int trainIndex;
    selectTrain(trains, &trainIndex);
    if (trainIndex == -1) return;

    printf("\n--- Reserved Seats for %s (%s) ---\n", trains[trainIndex].trainName, trains[trainIndex].route);
    for (int c = 0; c < CLASSES; c++) {
        printf("  %s Class:\n", trains[trainIndex].classes[c].className);
        int reservedFound = 0;
        for (int s = 0; s < SEATS_PER_CLASS; s++) {
            Seat seat = trains[trainIndex].classes[c].seats[s];
            if (seat.isReserved) {
                if (!reservedFound) {
                    printf("    Reserved Seats:\n");
                    reservedFound = 1;
                }
                printf("      Seat %2d: %s\n", seat.seatNumber, seat.passengerName);
            }
        }
        if (!reservedFound) {
            printf("    No reserved seats in this class.\n");
        }
    }
}

// Displays a visual representation of the seat chart for a selected train.
void displaySeatChart(Train trains[]) {
    int trainIndex;
    selectTrain(trains, &trainIndex);
    if (trainIndex == -1) return;

    printf("\n--- Seat Chart for %s (%s) ---\n", trains[trainIndex].trainName, trains[trainIndex].route);

    for (int c = 0; c < CLASSES; c++) {
        printf("\n%s Class:\n", trains[trainIndex].classes[c].className);
        for (int s = 0; s < SEATS_PER_CLASS; s++) {
            Seat seat = trains[trainIndex].classes[c].seats[s];
            if (seat.isReserved) {
                printf("[ X ] "); // 'X' for reserved seats
            } else {
                printf("[%-2d] ", seat.seatNumber); // Seat number for available seats
            }
            if ((s + 1) % 5 == 0) { // Print 5 seats per row for better visualization
                printf("\n");
            }
        }
        printf("\n");
    }
}

// --- Data Persistence Functions ---

// Saves train and user data to files
void saveData(Train trains[]) {
    FILE *train_fp = fopen("train_data.txt", "w");
    FILE *user_fp = fopen("user_data.txt", "w");

    if (train_fp == NULL) {
        perror("Error opening train_data.txt for writing");
        return;
    }
    if (user_fp == NULL) {
        perror("Error opening user_data.txt for writing");
        fclose(train_fp);
        return;
    }

    // Save user data
    fprintf(user_fp, "%d\n", userCount); // First, save the number of users
    for (int i = 0; i < userCount; i++) {
        fprintf(user_fp, "%s\n", users[i].username);
        fprintf(user_fp, "%s\n", users[i].password_hash);
    }

    // Save train data
    for (int i = 0; i < TRAINS; i++) {
        fprintf(train_fp, "%s|%s\n", trains[i].trainName, trains[i].route);
        for (int c = 0; c < CLASSES; c++) {
            fprintf(train_fp, "%s|%d\n", trains[i].classes[c].className, trains[i].classes[c].fare);
            for (int s = 0; s < SEATS_PER_CLASS; s++) {
                fprintf(train_fp, "%d,%d,%s\n",
                        trains[i].classes[c].seats[s].seatNumber,
                        trains[i].classes[c].seats[s].isReserved,
                        trains[i].classes[c].seats[s].passengerName);
            }
        }
    }

    fclose(train_fp);
    fclose(user_fp);
    printf("Data saved successfully!\n");
}

// Loads train and user data from files
void loadData(Train trains[]) {
    FILE *train_fp = fopen("train_data.txt", "r");
    FILE *user_fp = fopen("user_data.txt", "r");

    if (train_fp == NULL || user_fp == NULL) {
        printf("No saved data found. Initializing new data.\n");
        initializeTrains(trains, TRAINS); // Initialize trains with default empty state
        userCount = 0; // Ensure userCount is 0 if no user data file found
        if (train_fp) fclose(train_fp); // Close if one was opened but not the other
        if (user_fp) fclose(user_fp);
        return;
    }

    // Load user data
    if (fscanf(user_fp, "%d\n", &userCount) != 1) { // Read the number of users first
        printf("Error reading user count. User data might be corrupted. Resetting user data.\n");
        userCount = 0; // Reset user count if read fails
    } else {
        if (userCount > MAX_USERS) { // Prevent buffer overflow if file has too many users
            printf("Warning: Too many users in file, limiting to MAX_USERS (%d).\n", MAX_USERS);
            userCount = MAX_USERS;
        }
        for (int i = 0; i < userCount; i++) {
            if (fgets(users[i].username, sizeof(users[i].username), user_fp) != NULL) {
                users[i].username[strcspn(users[i].username, "\n")] = '\0';
            } else {
                printf("Error reading username for user %d. User data might be corrupted. Stopping user load.\n", i);
                userCount = i; // Adjust user count to reflect loaded users
                break;
            }
            if (fgets(users[i].password_hash, sizeof(users[i].password_hash), user_fp) != NULL) {
                users[i].password_hash[strcspn(users[i].password_hash, "\n")] = '\0';
            } else {
                printf("Error reading password hash for user %d. User data might be corrupted. Stopping user load.\n", i);
                userCount = i; // Adjust user count
                break;
            }
        }
    }


    // Load train data (similar logic as before)
    char line[300];

    for (int i = 0; i < TRAINS; i++) {
        if (fgets(line, sizeof(line), train_fp) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            char *token = strtok(line, "|");
            if (token != NULL) {
                strcpy(trains[i].trainName, token);
                token = strtok(NULL, "|");
                if (token != NULL) {
                    strcpy(trains[i].route, token);
                }
            }
        } else {
            printf("Error reading train name/route from file. Data might be corrupted. Initializing remaining trains.\n");
            // If reading fails, initialize the rest of the trains to default
            for(int j = i; j < TRAINS; j++) {
                initializeTrains(&trains[j], 1); // Initialize just this train with empty seats
            }
            break;
        }

        for (int c = 0; c < CLASSES; c++) {
            if (fgets(line, sizeof(line), train_fp) != NULL) {
                line[strcspn(line, "\n")] = '\0';
                char *token = strtok(line, "|");
                if (token != NULL) {
                    strcpy(trains[i].classes[c].className, token);
                    token = strtok(NULL, "|");
                    if (token != NULL) {
                        trains[i].classes[c].fare = atoi(token);
                    }
                }
            } else {
                printf("Error reading class name/fare for train %d. Data might be corrupted.\n", i);
                break;
            }

            for (int s = 0; s < SEATS_PER_CLASS; s++) {
                if (fgets(line, sizeof(line), train_fp) != NULL) {
                    line[strcspn(line, "\n")] = '\0';
                    char *token = strtok(line, ",");
                    if (token != NULL) {
                        trains[i].classes[c].seats[s].seatNumber = atoi(token);
                        token = strtok(NULL, ",");
                        if (token != NULL) {
                            trains[i].classes[c].seats[s].isReserved = atoi(token);
                            token = strtok(NULL, ",");
                            if (token != NULL) {
                                strcpy(trains[i].classes[c].seats[s].passengerName, token);
                            } else {
                                trains[i].classes[c].seats[s].passengerName[0] = '\0';
                            }
                        } else {
                            trains[i].classes[c].seats[s].passengerName[0] = '\0';
                        }
                    }
                } else {
                    printf("Error reading seat data for train %d, class %d. Data might be corrupted.\n", i, c);
                    break;
                }
            }
        }
    }

    fclose(train_fp);
    fclose(user_fp);
    printf("Data loaded successfully!\n");
}


// --- Main Function ---

int main() {
    // Initialize static train properties (names, routes).
    // Seat reservations and class fares/names will be loaded or default.
    Train trains[TRAINS] = {
        {"Shatabdi Express", "New Delhi to Bhopal"},
        {"Rajdhani Express", "Howrah to New Delhi"},
        {"Duronto Express", "Mumbai CSMT to Howrah"},
        {"Kaveri Express", "Chennai to Mysuru"},
        {"Magadh Express", "New Delhi to Patna"},
        {"Avadh Express", "Lucknow to New Delhi"},
        {"Aravali Express", "Mumbai to Jaipur"},
        {"Gol Gumbaz Express", "Pune to Bengaluru"},
        {"Kamrup Express", "Guwahati to Varanasi"},
        {"Vindhyachal Express", "Bhopal to New Delhi"},
        {"Chetak Express", "Delhi to Udaipur"},
        {"Basava Express", "Bangalore to Raichur"},
        {"Charminar Express", "Hyderabad to Chennai"},
        {"Kolkata Express", "Kolkata to Varanasi"},
        {"Saurashtra Express", "Ahmedabad to Delhi"},
        {"Kalinga Utkal Express", "Bhubaneswar to Nagpur"},
        {"Secunderabad Express", "Secunderabad to Delhi"},
        {"Kerala Express", "Thiruvananthapuram to Bangalore"},
        {"Punjab Mail", "Chandigarh to Amritsar"},
        {"Ranchi Express", "Ranchi to Kolkata"}
    };

    // Load data at the start of the program
    loadData(trains);

    // Authentication loop
    int auth_successful = 0;
    while (!auth_successful) {
        int choice;
        showLoginSignupMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            flushInput();
            continue;
        }
        flushInput();

        switch (choice) {
            case 1: // Login
                auth_successful = login();
                break;
            case 2: // Sign Up
                if (signup()) { // If signup was successful (new user added)
                   saveData(trains); // Save user data (and train data, though train data is unchanged)
                }
                break;
            case 3: // Exit from pre-login menu
                printf("Exiting Train Reservation System. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please enter 1, 2, or 3.\n");
                break;
        }
    }

    // Main application loop (after successful login)
    int choice;
    do {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            flushInput();
            continue;
        }
        flushInput();

        switch (choice) {
            case 1:
                reserveSeat(trains);
                break;
            case 2:
                cancelReservation(trains);
                break;
            case 3:
                displaySeatChart(trains);
                break;
            case 4:
                displayReservedSeats(trains);
                break;
            case 5:
                printf("Exiting Train Reservation System. Bye!\n");
                saveData(trains); // Save all data before exiting the main loop
                break;
            default:
                printf("Invalid choice. Please enter a number between 1 and 5.\n");
        }
    } while (choice != 5);

    return 0;
}