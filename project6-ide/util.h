//
// STARTER CODE: util.h
//
// util.h stores struct and the functions for freeTree,
// buildFrequencyMap, buildEncodingTree
// buildEncodingMap, encode, decode, compress, and decompress
//
// used c++ beautifier
//

#pragma once 
#include <iostream>

#include <fstream>
#include <functional>  // std::greater
#include <map>
#include <queue>  // std::priority_queue
#include <string>
#include <unordered_map>
#include <vector>  // std::vector

#include "bitstream.h"
#include "hashmap.h"
#include "mymap.h"
#include "util.h"

struct HuffmanNode {
  int character;
  int count;
  HuffmanNode* zero;
  HuffmanNode* one;
};

class prioritize  // you could also use a struct
{
 public:
  bool operator()(const HuffmanNode* p1, const HuffmanNode* p2) const {
    return p1->count > p2->count;  // HuffmanNode* will be different
  }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
  if (node == nullptr) {
    return;
  }
  freeTree(node->zero);  // post-order recursion
  freeTree(node->one);
  delete node;
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap& map) {
  if (isFile == true) {
    ifstream inFS(filename);
    char c;
    inFS.get(c);

    while (!inFS.eof()) {
      if (map.containsKey(
              c)) {  // checks if theres a key and puts into the hashmap
        map.put(c, map.get(c) + 1);
      } else {
        map.put(c, 1);
      }
      inFS.get(c);
    }
    map.put(PSEUDO_EOF, 1);
  } else {
    stringstream inFS(filename);
    char c;
    inFS.get(c);
    while (!inFS.eof()) {
      if (map.containsKey(c)) {
        map.put(c, map.get(c) + 1);
      } else {
        map.put(c, 1);
      }
      inFS.get(c);
    }
    map.put(PSEUDO_EOF, 1);
  }
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap& map) {
  priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;
  vector<int> mKey = map.keys();

  for (auto e : mKey) {
    HuffmanNode* temp = new HuffmanNode();
    temp->character = e;
    temp->count = map.get(e);
    temp->zero = nullptr;
    temp->one = nullptr;
    pq.push(temp);
  }

  while (pq.size() > 1) {
    HuffmanNode* firstPop = pq.top();
    pq.pop();
    HuffmanNode* secondPop = pq.top();
    pq.pop();
    HuffmanNode* temp = new HuffmanNode();
    temp->character = NOT_A_CHAR;
    temp->count = (firstPop->count + secondPop->count);
    temp->zero = firstPop;
    temp->one = secondPop;
    pq.push(temp);
  }

  return pq.top();
}

void recursiveBuildEncodingMap(HuffmanNode* tree, string BEM,
                               mymap<int, string>& tempMap) {
  if (tree == nullptr) {
    return;
  } else if (tree->character == NOT_A_CHAR) {
    recursiveBuildEncodingMap(tree->zero, BEM + "0",
                              tempMap);  // recursive call to each node
    recursiveBuildEncodingMap(tree->one, BEM + "1", tempMap);
  } else if (tree->character != NOT_A_CHAR) {
    tempMap.put(tree->character, BEM);
  }
}

//
// *This function builds the encoding map from an encoding tree.
//
mymap<int, string> buildEncodingMap(HuffmanNode* tree) {
  mymap<int, string> encodingMap;

  string str = "";
  recursiveBuildEncodingMap(tree, str, encodingMap);
  return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, mymap<int, string>& encodingMap,
              ofbitstream& output, int& size, bool makeFile) {
  string outputString;
  char stringChar;

  if (makeFile == true) {
    while (input.get(stringChar)) {
      if (encodingMap.contains(stringChar)) {  // checks if char is in the mymap
        outputString += encodingMap.get(stringChar);
      }
    }
    outputString += encodingMap.get(PSEUDO_EOF);
    for (char e : outputString) {
      if (e == '0') {
        output.writeBit(0);
      } else if (e == '1') {
        output.writeBit(1);
      }
    }
  } else {
    while (input.get(stringChar)) {
      outputString += encodingMap.get(stringChar);
    }
    outputString += encodingMap.get(PSEUDO_EOF);
  }
  size = outputString.size();
  return outputString;
}

// int recursiveDecode (ifbitstream &input, HuffmanNode* temp) {
//   while (!input.eof()) {
//     int bit = input.readBit();

//     if (temp -> character != NOT_A_CHAR) {
//       return temp -> character;
//     }
//     else {
//       if (bit == 1) {
//         return recursiveDecode(input, temp -> one);
//       }
//       else if (bit == 0){
//         return recursiveDecode(input, temp -> zero);
//       }
//     }
//   }
//   return -1;
// }

//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream& input, HuffmanNode* encodingTree, ofstream& output) {
  string outputDecode;
  HuffmanNode* temp = encodingTree;

  while (!input.eof()) {
    int bit = input.readBit();
    if (bit == 1) {  // goes right
      encodingTree = encodingTree->one;
    } else if (bit == 0) {  // goes left
      encodingTree = encodingTree->zero;
    }
    if (encodingTree->character != NOT_A_CHAR) {
      if (encodingTree->character == PSEUDO_EOF) {
        break;
      } else {
        output.put(char(encodingTree->character));
        outputDecode += char(encodingTree->character);
        encodingTree = temp;
      }
    }
  }

  return outputDecode;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
  int size = 0;
  hashmap frequencyMap;
  HuffmanNode* encodingTree;
  mymap<int, string> encodingMap;

  buildFrequencyMap(filename, true, frequencyMap);
  encodingTree = buildEncodingTree(frequencyMap);  // calls buildEncodingTree
  encodingMap = buildEncodingMap(encodingTree);    // calls buildEncodingMap

  ofbitstream output(filename + ".huf");
  ifstream input(filename);
  output << frequencyMap;  // add the frequency map to the file
  string codeStr =
      encode(input, encodingMap, output, size, true);  // build encode

  freeTree(encodingTree);
  return codeStr;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
  ifbitstream input(filename);
  hashmap frequencyMap;
  input >> frequencyMap;
  HuffmanNode* encodingTree;
  encodingTree = buildEncodingTree(frequencyMap);  // calls buildEncodingTree

  size_t pos = filename.find(".txt.huf");
  string outfile = filename.substr(0, pos);
  ofstream output(outfile + "_unc.txt");
  string decodeStr = decode(input, encodingTree, output);  // calls decode

  freeTree(encodingTree);
  return decodeStr;
}