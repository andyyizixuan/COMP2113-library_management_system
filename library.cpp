#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include "book.h"
#include "member.h"
#include "helper.h"
#include "library.h"

using namespace std;

void borrowBook() {
    string memberID, bookID;
    
    cout << "\n--- Borrow Book ---" << endl;
    
    // 1. 查会员 (调 B 模块的 findMemberById)
    cout << "Enter Member ID: ";
    cin >> memberID;
    const Member* member = findMemberById(memberID);
    if (member == nullptr) {
        cout << "Error: Member not found." << endl;
        return;
    }

    // 2. 查书 (调 A 模块的 searchBook_ID)
    cout << "Enter Book ID: ";
    cin >> bookID;
    Book* book = searchBook_ID(bookID);
    if (book == nullptr) {
        cout << "Error: Book not found." << endl;
        return;
    }

    // 3. 查该会员已借几本 (遍历 borrowRecords)
    int borrowedCount = 0;
    for (size_t i = 0; i < borrowRecords.size(); ++i) {
        if (borrowRecords[i].memberID == memberID) {
            borrowedCount++;
        }
    }
    
    if (borrowedCount >= 5) {
        cout << "Error: Borrow limit reached. A member can only borrow up to 5 books." << endl;
        return;
    }

    // 4. 检查书是否有库存
    if (book->getAvailableNumber() > 0) {
        if (book->borrowCopy()) {
            BorrowRecord newRecord;
            newRecord.memberID = memberID;
            newRecord.bookID = bookID;
            newRecord.borrowTime = time(nullptr);
            
            borrowRecords.push_back(newRecord);
            cout << "Success: Book \"" << book->getTitle() << "\" borrowed successfully!" << endl;
        } else {
            cout << "Error: Failed to borrow the book (system error)." << endl;
        }
    } else {
        if (addReservation(bookID, memberID)) {
            cout << "Notice: No copies available. You have been added to the reservation queue." << endl;
        } else {
            cout << "Error: Failed to add reservation." << endl;
        }
    }
}


void viewOverdue() {
    cout << "\n--- Overdue Records ---" << endl;
    bool hasOverdue = false;

    // 遍历 borrowRecords
    for (size_t i = 0; i < borrowRecords.size(); ++i) {
        time_t borrowTime = borrowRecords[i].borrowTime;
        
        // 对每条调 E 的 isOverdue() / calculateFine()
        if (isOverdue(borrowTime)) {
            hasOverdue = true;
            double fine = calculateFine(borrowTime);
            long overdueDays = calcDayDiff(borrowTime, time(nullptr)) - 14; 
            cout << "Member ID: " << borrowRecords[i].memberID 
                 << " | Book ID: " << borrowRecords[i].bookID 
                 << " | Overdue Days: " << overdueDays
                 << " | Fine: " << fine << " HKD" << endl;
        }
    }

    if (!hasOverdue) {
        cout << "No overdue records found." << endl;
    }
    cout << "-----------------------" << endl;
}
