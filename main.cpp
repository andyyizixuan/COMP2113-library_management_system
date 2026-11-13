#include <iostream>
#include "book.h"
#include "member.h"
#include "helper.h"

using namespace std;

void borrowBook();
void viewOverdue();
void returnBook();
void saveData();
void loadData();

int main(){
    int choice = -1;

    while (choice != 0){
        cout << "\n=================================================" << endl;
        cout << "    Welcome to the Library Management System :)   " << endl;
        cout << "==================================================" << endl;
        cout << "1. Add New Book(s)" << endl;
        cout << "2. Search for Book(s)" << endl;
        cout << "3. Member Registration" << endl;
        cout << "4. Borrow Book(s)" << endl;
        cout << "5. Return Book(s)" << endl;
        cout << "6. View Overdue Records" << endl;
        cout << "7. Save Data" << endl;
        cout << "8. Load Data" << endl;
        cout << "0. Exit" << endl;
        cout << "===================================================" << endl;
        cout << "Please enter your choice: "；

        if (!(cin >> choice)){
            cout << "Invalid input, please try again T_T" << endl;
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice){
            case 1:
                handleAddBook();
                break;
            case 2:
                handleSearchBook();
                break;
            case 3:
                registerMember();
                break;
            case 4:
                borrowBook();
                break;
            case 5:
                returnBook();
                break;
            case 6:
                viewOverdue();
                break;
            case 7:
                saveData();
                break;
            case 8:
                loadData();
                break;
            case 0:
                cout << "Thank you for using, goodbye! ^_^ " << endl;
                break;
            default:
                cout << "Invalid choice, please enter a number (0-8). >_<" << endl;
        }
    }

    return 0;
}



