#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace chrono;

int binarySearchDouble(const vector<double>& arr, double target, double eps) {
    int left = 0, right = arr.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (fabs(arr[mid] - target) <= eps) return mid; 
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

template <typename K, typename V>
class HashTableOpenAddressing {
private:
    enum State { EMPTY, OCCUPIED, DELETED };
    struct Entry {
        K key;
        V value;
        State state;
        Entry() : state(EMPTY) {}
    };
    vector<Entry> table;
    int size;
    int count; 
    int deletedCount;
    int hashFunc(const string& key) const {
        int h = 0;
        for (char c : key) h = (h * 31 + c) % size;
        return h;
    }
    int hashFunc(int key) const {
        return key % size;
    }

    int hash(const K& key) const {
        return hashFunc(key);
    }
    void rehash() {
        vector<Entry> oldTable = table;
        int oldSize = size;
        size *= 2;
        table.clear();
        table.resize(size);
        count = 0;
        deletedCount = 0;
        for (int i = 0; i < oldSize; i++) {
            if (oldTable[i].state == OCCUPIED) {
                insert(oldTable[i].key, oldTable[i].value);
            }
        }
    }

public:
    HashTableOpenAddressing(int s = 10) : size(s), count(0), deletedCount(0) {
        table.resize(size);
    }
    void insert(const K& key, const V& value) {
        if ((count + deletedCount) * 2 >= size) {
            rehash();
        }
        int index = hash(key);
        int i = 0;
        int firstDeleted = -1; 
        while (true) {
            int pos = (index + i) % size;
            if (table[pos].state == EMPTY) {
                if (firstDeleted != -1) {
                    pos = firstDeleted;
                    deletedCount--;
                }
                table[pos].key = key;
                table[pos].value = value;
                table[pos].state = OCCUPIED;
                count++;
                return;
            }
            else if (table[pos].state == DELETED) {
                if (firstDeleted == -1) firstDeleted = pos;
            }
            else if (table[pos].state == OCCUPIED) {
                if (table[pos].key == key) {
                    table[pos].value = value;
                    return;
                }
            }
            i++;
        }
    }
    V* find(const K& key) {
        int index = hash(key);
        int i = 0;
        while (true) {
            int pos = (index + i) % size;
            if (table[pos].state == EMPTY) return nullptr;
            if (table[pos].state == OCCUPIED && table[pos].key == key) {
                return &table[pos].value;
            }
            i++;
            if (i == size) return nullptr;
        }
    }
    bool remove(const K& key) {
        int index = hash(key);
        int i = 0;
        while (true) {
            int pos = (index + i) % size;
            if (table[pos].state == EMPTY) return false;
            if (table[pos].state == OCCUPIED && table[pos].key == key) {
                table[pos].state = DELETED;
                count--;
                deletedCount++;
                return true;
            }
            i++;
            if (i == size) return false;
        }
    }

    int getSize() const { return count; }
};
int binarySearchString(const vector<string>& arr, const string& target) {
    int left = 0, right = arr.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}
string randomString(int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return s;
}

int main() {
    srand(time(0));

    cout << "Binary search for double with an error margin" << endl;
    vector<double> darr = { 1.0, 2.5, 3.7, 4.2, 5.9, 6.1, 7.8 };
    double target = 4.2001;
    double eps = 0.001;
    int idx = binarySearchDouble(darr, target, eps);
    if (idx != -1) {
        cout << "The element was found " << darr[idx] << " by index " << idx
            << " (error rate " << eps << ")" << endl;
    }
    else {
        cout << "The element was not found" << endl;
    }
    cout << endl;

    cout << "Hash table with linear probing (string -> int)" << endl;
    HashTableOpenAddressing<string, int> ht(5);
    ht.insert("apple", 10);
    ht.insert("banana", 20);
    ht.insert("cherry", 30);
    ht.insert("date", 40);
    ht.insert("elderberry", 50);
    int* val = ht.find("cherry");
    if (val) cout << "cherry: " << *val << endl;
    ht.remove("banana");
    val = ht.find("banana");
    if (!val) cout << "banana is delete" << endl;
    cout << endl;

    cout << "Comparison of binary search and a hash table" << endl;
    const int N = 100000;
    const int M = 10000;
    vector<string> arr;
    HashTableOpenAddressing<string, int> hashTable(N * 2);
    arr.reserve(N);
    for (int i = 0; i < N; i++) {
        string s = randomString(10);
        arr.push_back(s);
        hashTable.insert(s, i);
    }
    sort(arr.begin(), arr.end());

    vector<string> queries;
    queries.reserve(M);
    for (int i = 0; i < M; i++) {
        if (i % 2 == 0) {
            queries.push_back(arr[rand() % N]); // the key exists
        }
        else {
            queries.push_back(randomString(10)); // random key
        }
    }

    // Binary search is metering
    auto start = high_resolution_clock::now();
    volatile int sumBinary = 0;
    for (const string& q : queries) {
        int idx = binarySearchString(arr, q);
        sumBinary += idx;
    }
    auto end = high_resolution_clock::now();
    long long binaryTime = duration_cast<microseconds>(end - start).count();

    // Measuring the hash table search
    start = high_resolution_clock::now();
    volatile int sumHash = 0;
    for (const string& q : queries) {
        int* v = hashTable.find(q);
        if (v) sumHash += *v;
    }
    end = high_resolution_clock::now();
    long long hashTime = duration_cast<microseconds>(end - start).count();

    cout << "Number of requests: " << M << endl;
    cout << "Binary search (O(log n)): " << binaryTime << " msc" << endl;
    cout << "Hash table: "
        << hashTime << " msc" << endl;

    return 0;
}