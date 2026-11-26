//
// Created by 27565 on 2026/11/4.
//

#include <iostream>
#include <string>
#include <vector>
#include "book.h"
using namespace std;

// 定义书柜
std::vector<std::unique_ptr<Book>> Booklist;

// 构造函数
Book::Book(const std::string& id, const std::string& title,
           const std::string& author, const int total_number, int available_number)
    : id(id), title(title), author(author),
      total_number(total_number), available_number(available_number) {}
// 借书
bool Book::borrowCopy() {
    if (available_number > 0) {
        available_number--;
        return true;
    }
    else {
        return false;
    }
}
// 还书
bool Book::returnCopy() {
    if (available_number < total_number) {
        available_number++;
        return true;
    }
    else {
        return false;
    }
}
std::string Book::getId() const {
    return id;
}

std::string Book::getTitle() const {
    return title;
}

std::string Book::getAuthor() const {
    return author;
}

int Book::getTotalNumber() const {
    return total_number;
}

int Book::getAvailableNumber() const {
    return available_number;
}

// =============================================================
// 全局函数
// 加书
bool addBook(const std::string& id, const std::string& title,
             const std::string& author, const int total_number) {
    //新增了判断是否输入为空
    if (id.empty() || title.empty() || author.empty()) {
        std::cout << "Book information cannot be empty." << std::endl;
        return false;
    }
    //新增了判断非法输入totalnumber<0
    if (total_number <= 0) {
        std::cout << "Total number must be positive." << std::endl;
        return false;
    }
    if (searchBook_ID(id) != nullptr) {
        std::cout << "The book already exists." << std::endl;
        return false;
    }
    else {
        Booklist.push_back(std::unique_ptr<Book>(
            new Book(id, title, author, total_number, total_number)));
        std::cout << "Book added successfully." << std::endl;
        return true;
    }
}
// 用id找书
Book* searchBook_ID(const std::string& id) {
    for (std::size_t i = 0; i < Booklist.size(); i++) { // 用 size_t 消除警告
        Book *current = Booklist[i].get();
        if (current->getId() == id) {
            return current;
        }
    }
    return nullptr;
}
// 用标题找书，支持模糊查找，会返回一堆书名
std::vector<Book*> searchBook_title(const std::string& title) {
    std::vector<Book*> result;
    for (std::size_t i = 0; i < Booklist.size(); i++) {
        Book *current = Booklist[i].get();
        if (current->getTitle().find(title) != std::string::npos) {
            result.push_back(current);
        }
    }
    return result;
}
// 从文件里加载书库存
bool addBookFromFile(const std::string& id, const std::string& title,
                     const std::string& author, const int total_number, int available_number) {
    //新增了判断空输入
    if (id.empty() || title.empty() || author.empty() ||
        total_number <= 0 || available_number < 0 || available_number > total_number) {
        return false;
    }
    if (searchBook_ID(id) == nullptr) {
        Booklist.push_back(std::unique_ptr<Book>(new Book(id, title, author, total_number, available_number)));
        return true;
    }
    else {
        return false;
    }
}
// 交互：加书
void handleAddBook() {
    std::string id, title, author;
    int total_number = 0;
    cout << "Book ID: ";
    cin >> id;
    cin.ignore(10000, '\n'); // 解决回车残留
    cout << "Title: ";
    getline(cin, title); // 防止书名有空格读取错误
    cout << "Author: ";
    getline(cin, author);
    cout << "Total number: ";
    if (!(cin >> total_number)) {
        cout << "Invalid input, please try again T_T" << endl;
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    } // 处理错误输入避免死循环
    cin.ignore(10000, '\n');
    addBook(id, title, author, total_number);
}
// 交互：查书
void handleSearchBook() {
    int choice;
    std::cout << "Book Searching" << std::endl;
    std::cout << "1. Use ID" << std::endl;
    std::cout << "2. Use title" << std::endl;
    std::cout << "Please enter 1 or 2......" << std::endl;
    if (!(std::cin >> choice)) {
        std::cout << "Invalid input, please try again T_T" << std::endl;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return;
    } // 避免错误输入
    if (choice == 1) {
        std::string id;
        std::cout << "Please enter the ID: ";
        std::cin >> id;

        Book *b = searchBook_ID(id);
        if (b != nullptr) {
            std::cout << "Book already found." << std::endl;
            std::cout << "ID: " << b->getId() << std::endl;
            std::cout << "Title: " << b->getTitle() << std::endl;
            std::cout << "Author: " << b->getAuthor() << std::endl;
            std::cout << "Total: " << b->getTotalNumber() << std::endl;
            std::cout << "Available: " << b->getAvailableNumber() << std::endl;
        }
        else {
            std::cout << "Book not found." << std::endl;
        }
    }
    else if (choice == 2) {
        std::string title;
        std::cout << "Please enter the title keyword: ";
        std::cin >> title;

        std::vector<Book*> result = searchBook_title(title);
        if (result.empty() == false) {
            std::cout << result.size() << " books already found." << std::endl;
            for (std::size_t i = 0; i < result.size(); i++) {
                Book *current = result[i];
                std::cout << "Book " << i + 1 << std::endl;
                std::cout << "ID: " << current->getId() << std::endl;
                std::cout << "Title: " << current->getTitle() << std::endl;
                std::cout << "Author: " << current->getAuthor() << std::endl;
                std::cout << "Available: " << current->getAvailableNumber() << std::endl;
            }
        }
        else {
            std::cout << "Book not found." << std::endl;
        }
    }
    else {
        std::cout << "Invalid choice, please enter 1 or 2." << std::endl;
    }
}
