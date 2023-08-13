#include <bits/stdc++.h>
using namespace std;

FILE *openFile(string fileName, string mode) {
  FILE *fp = fopen(fileName.c_str(), mode.c_str());
  if (fp == NULL) {
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  return fp;
}

struct Transaction {
  int id;
  int amount;
  char customerName[4];
  int itemCategory;

  bool input(FILE *inFile) {
    if (!inFile) {
      return false;
    }
    return fscanf(inFile, "%d %d %s %d", &id, &amount, customerName,
                  &itemCategory) == 4;
  }

  void output(FILE *outFile) {
    if (!outFile) {
      return;
    }
    fprintf(outFile, "%d %d %s %d\n", id, amount, customerName, itemCategory);
  }
};

string randStr(int len) {
  string res;
  for (int i = 0; i < len; i++) {
    char randChar = 'a' + (rand() % 26);
    res.push_back(randChar);
  }
  return res;
}

void createInput(int numRecords) {
  FILE *outFile = openFile("input.txt", "w");
  for (int i = 0; i < numRecords; i++) {
    Transaction trans;
    trans.id = i + 1;
    trans.amount = (rand() % 60000) + 1;
    strcpy(trans.customerName, randStr(3).c_str());
    trans.itemCategory = (rand() % 1500) + 1;
    trans.output(outFile);
  }
  fclose(outFile);
}

struct Bucket {
  int localDepth;
  vector<Transaction> records;

  Bucket(int depth = 0) { localDepth = depth; }
};

const int MAX_BUCKETS = 1000000, MAX_BITS = 16, BUCKET_SIZE = 3;
Bucket *buckets[MAX_BUCKETS], *tempBucket;
int globalDepth = 1;

void initDirectory() {
  int dirSize = 1 << globalDepth;
  Bucket *bucket = new Bucket();
  for (int i = 0; i < dirSize; i++) {
    buckets[i] = bucket;
  }
  tempBucket = buckets[MAX_BUCKETS - 1] = new Bucket();
}

int calculateHash(int n) { return n >> (MAX_BITS - globalDepth); }

void insertRecord(Transaction trans) {
  int bucketIdx = calculateHash(trans.id);
  if (buckets[bucketIdx]->records.size() < BUCKET_SIZE) {
    buckets[bucketIdx]->records.push_back(trans);
  } else {
    buckets[bucketIdx]->localDepth++;
    if (buckets[bucketIdx]->localDepth > globalDepth) {
      globalDepth++;
      int dirSize = 1 << globalDepth;
      for (int i = dirSize - 1; i >= 0; i--) {
        buckets[i] = buckets[i >> 1];
      }
      bucketIdx <<= 1;
    }
    int pairIdx =
        bucketIdx ^ (1 << (globalDepth - buckets[bucketIdx]->localDepth));
    buckets[pairIdx] = new Bucket(buckets[bucketIdx]->localDepth);
    tempBucket->records = buckets[bucketIdx]->records;
    buckets[bucketIdx]->records.clear();
    int maxi = 1 << (globalDepth - buckets[bucketIdx]->localDepth);
    int offset = pairIdx & (((1 << MAX_BITS) - 1) ^ (maxi - 1));
    for (int i = 0; i < maxi; i++) {
      buckets[i + offset] = buckets[pairIdx];
    }
    for (int i = 0; i < tempBucket->records.size(); i++) {
      insertRecord(tempBucket->records[i]);
    }
    insertRecord(trans);
  }
}

void readAndInsertRecords() {
  FILE *inFile = openFile("input.txt", "r");
  Transaction trans;
  while (trans.input(inFile)) {
    insertRecord(trans);
  }
  fclose(inFile);
}

void visualizeBuckets() {
  FILE *outFile = openFile("output.txt", "w");
  for (int i = 0; i < MAX_BUCKETS && buckets[i] != NULL; i++) {
    string bucketAddr;
    for (int j = 0, x = i; j < globalDepth; j++) {
      bucketAddr.push_back(x % 2 + '0');
      x >>= 1;
    }
    reverse(bucketAddr.begin(), bucketAddr.end());
    fprintf(outFile, "%s\n", bucketAddr.c_str());
    for (int j = 0; j < buckets[i]->records.size(); j++) {
      buckets[i]->records[j].output(outFile);
    }
    fprintf(outFile, "\n");
  }
  fclose(outFile);
}

int main() {
   int numRecords;
   cin >> numRecords;
   createInput(numRecords);
  initDirectory();
  readAndInsertRecords();
  visualizeBuckets();
  return 0;
}
