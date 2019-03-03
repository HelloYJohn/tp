# tidb pingcap
  ## 1. 基本思路
      + 100G的大文件，1G内存肯定无法处理，把大文件hash成小文件（500M左右，具体为什么选，下面会介绍，文件个数大概200左右），不同的url不会分到相同的文件
      + 使用topk算法（最小堆，依据重复的个数） 求每个小文件的前100重复的URL（最后有多个相同的，取最先满足重复个数）放到一个线性容器中（大概2万左右）
      + 对这2万数据进行使用topk算法（依据重复的个数），取前一百即可
      
  ## 2. 文件介绍
      + CTopk.h topk算法实现
        + void Clear();  // 清除函数
        + void HeapAdjust(int nStart, int nLen); // 最小堆被破坏的调整函数
        + void BuildHeap(int nLen); // 简历最小堆
        + bool CompareGreater(const T &v1, const T &v2); // 通用比较函数
      + main.cpp 外存算法
        + size_t GetFileByteSize(std::string fileName） // 获取文件大小
        + size_t GetBucketNumber(const std::string& fileName, size_t bucketDataSize) // 桶数
        + void hash_write_file(std::string fileName, std::vector<std::string>& fileNames, size_t bucket_num, std::string base) // 通过hash分割文件
        + void read_hash_file(const std::vector<std::string>& fileNames, std::vector<std::pair<std::string, int>>& vec_pair, size_t bucketDataSize) 读取分割后的文件
        
  ## 3. 实现介绍
     + 利用GetFileByteSize获取文件大小，再根据可用内存，来分割多少个小文件
      
