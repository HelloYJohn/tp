//
//  main.cpp
//  PingCAP
//
//  Created by john-y on 2019/2/28.
//  Copyright © 2019 john-y. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include "CTopK.h"

using std::ifstream;
using std::ofstream;
using std::cout;
using std::endl;

#define TOP_K 100

// vector compare function
bool SortByPair(const std::pair<std::string, int> &v1, std::pair<std::string, int> &v2) {
    return v1.second > v2.second;
}

// get file size
size_t GetFileByteSize(std::string fileName) {
    ifstream ifstrm;
    ifstrm.open(fileName, ifstream::in);
    if (ifstrm.is_open()) {
        ifstrm.clear();
        ifstrm.seekg(0, std::ios::end);
        size_t ret = ifstrm.tellg();
        ifstrm.seekg(0, std::ios::beg);
        ifstrm.close();
        return ret;
    } else {
        throw fileName + std::string(", file is not existed");
    }
}

size_t GetBucketNumber(const std::string& fileName, size_t bucketDataSize) {
    size_t bucket_num = 0;
    size_t fileSize = GetFileByteSize(fileName);
    if (fileSize != -1) {
        bucket_num = fileSize / bucketDataSize + 1;
    }
    return bucket_num;
    
}

// split file by hash
void hash_write_file(std::string fileName, std::vector<std::string>& fileNames, size_t bucket_num, std::string base) {
    ifstream ifstrm;
    std::vector<std::ofstream> streams;
    ifstrm.open(fileName, ifstream::in);
    std::hash<std::string> hash_fn;
    if (bucket_num == 1) {
        fileNames.push_back(fileName);
        if (ifstrm.is_open()) {
            ifstrm.close();
        } else {
            throw fileName + std::string(", file is not existed");
        }
        
    } else {
        for (int i = 0; i < bucket_num; ++i) {
            std::string fileName_ = base +  "-" + std::to_string(i) + ".txt";
            // std::cout << "fileName_: " << fileName_ << std::endl;
            streams.emplace_back(std::ofstream{ fileName_ });
            fileNames.emplace_back(fileName_);
        }
        
        if (ifstrm.is_open()) {
            std::string str;
            while (std::getline(ifstrm, str, '\n')) {
                size_t hash = hash_fn(str);
                // std::cout << str << endl;
                streams[hash % bucket_num] << str << std::endl;
            }
            if (ifstrm.is_open()) {
                ifstrm.close();
            }
        } else {
            throw fileName + std::string(", file is not existed");
        }
        for (std::vector<std::ofstream>::iterator it = streams.begin();  it != streams.begin(); ++it) {
            if (it->is_open()) {
                it->close();
            }
        }
    }
}

// read hash small file
void read_hash_file(const std::vector<std::string>& fileNames, std::vector<std::pair<std::string, int>>& vec_pair, size_t bucketDataSize,
                   size_t parentFileSize) {
    std::unordered_map<std::string, int>::iterator unorder_map_it;
    ifstream ifstrm;
    for (int i = 0; i < fileNames.size(); ++i) {
        size_t bucket_num = 0;
        size_t fileSize = GetFileByteSize(fileNames[i]);
        std::unordered_map<std::string, int> unorder_map;
        bucket_num = GetBucketNumber(fileNames[i], bucketDataSize);
        // filesize < bucketDataSize or subFileSize == parentFileSize (url all is same)
        if (bucket_num == 1 || parentFileSize == fileSize) {
            /*
            if (parentFileSize == fileSize) {
                std::cout << "parentFileSize == fileSize" << std::endl;
            }
             */
            ifstrm.open(fileNames[i]);
            if (ifstrm.is_open()) {
                std::string str;
                while (std::getline(ifstrm, str, '\n')) {
                    if ((unorder_map_it = unorder_map.find(str)) != unorder_map.end()) {
                        unorder_map_it->second += 1;
                    } else {
                        unorder_map.insert({str, 1});
                    }
                }
            } else {
                throw fileNames[i] + std::string(", file is not existed");
            }
            ifstrm.close();
            int KNum = TOP_K;
            CTopK<std::unordered_map<std::string, int>::iterator> CTopK_unorder_map_it;
            CTopK_unorder_map_it.GetTopK(unorder_map, KNum);
            for (int i = 0; i < KNum; ++i) {
                vec_pair.push_back({CTopK_unorder_map_it.m_Data[i]->first, CTopK_unorder_map_it.m_Data[i]->second});
            }
            
        } else {
            std::vector<std::string> SubFileNames;
            // std::cout << "bucket_num: " << bucket_num << std::endl;
            hash_write_file(fileNames[i], SubFileNames, bucket_num, fileNames[i]);
            read_hash_file(SubFileNames, vec_pair, bucketDataSize, fileSize);
        }
    }
}

int main(int argc, const char * argv[]) {
    // bucket number
    size_t bucket_num = 0;
    int MNum = 5;
    if (argc == 2) {
    } else if (argc == 3) {
        MNum = std::atoi(argv[2]);
        if (MNum <= 0) {
            std::cout << "help: ./a.out filename [M]" << std::endl;
            std::cout << "M is a integer" << std::endl;
        }
    } else {
        std::cout << "help: ./a.out filename [% M]" << std::endl;
        return -1;
    }
    
    // deal bucket number
    const size_t bucketDataSize = 1024 * 1024 * MNum; // 5M
    // std::cout << "bucketDataSize: " << bucketDataSize << std::endl;
    try {
        bucket_num = GetBucketNumber(argv[1], bucketDataSize);
        size_t fileSize = GetFileByteSize(argv[1]);
        // std::cout << "bucket_num: " << bucket_num << endl;
        std::vector<std::string> fileNames;
        hash_write_file(argv[1], fileNames, bucket_num, "base");
        
        std::vector<std::pair<std::string, int>> vec_pair;
        // read hash file
        read_hash_file(fileNames, vec_pair, bucketDataSize, fileSize);
        
        int KNum = TOP_K;
        CTopK<std::vector<std::pair<std::string, int>>::iterator> CTopK_vt;
        std::vector<std::pair<std::string, int>>::iterator CTopK_vt_it;
        CTopK_vt.GetTopK(vec_pair, KNum);
        ofstream ofstream;
        ofstream.open("result");
        if (ofstream.is_open()) {
            for (int i = 0; i < KNum; i++) {
                // std::cout << CTopK_vt.m_Data[i]->second << " " << CTopK_vt.m_Data[i]->first << endl;
                ofstream << CTopK_vt.m_Data[i]->second << " " << CTopK_vt.m_Data[i]->first << endl;
            }
        }
        ofstream.close();
        std::cout << "success !" << std::endl;
    } catch (std::string e) {
        std::cout << e << std::endl;
        std::cout << "help: ./a.out filename" << std::endl;
    }
    return 0;
}
