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
        + int GetTopK(std::unordered_map<std::string, int>& unorder_map, int& nTop);
        + int GetTopK(std::vector<std::pair<std::string, int>>& pair_vt, int& nTop);
      + main.cpp 外存算法
        + size_t GetFileByteSize(std::string fileName） // 获取文件大小
        + size_t GetBucketNumber(const std::string& fileName, size_t bucketDataSize) // 桶数
        + void hash_write_file(std::string fileName, std::vector<std::string>& fileNames, size_t bucket_num, std::string base) // 通过hash分割文件
        + void read_hash_file(const std::vector<std::string>& fileNames, std::vector<std::pair<std::string, int>>& vec_pair, size_t bucketDataSize) 读取分割后的文件
        
  ## 3. 实现介绍
      + 利用GetFileByteSize获取文件大小，再根据可用内存，来分割多少个小文件，这里会有些特殊情况
        + 一个文件url数小于100， 这里topk的 int GetTopK(std::unordered_map<std::string, int>& unorder_map, int& nTop)函数 nTop是传引用，当文件中url小于100时，只取文件中的
        + 一个文件url全相同，且文件size大于设定的Filesize, 这时可以通过比较 parentFileSize 和 hash分割后 SubFileSize 是否相等来处理
        + 一般情况，hash分割后，文件size小于等于设定的Filesize，使用unorder_map记录次数，TopK取前100
        + 一次hash分割后FileSize仍然大于设定的Filesize, 我的处理方式继续hash分割, 直到遇到上面三种情况
      
  ## 4. 单元测试
      + 一般情况测试，hash分割后，文件size小于等于设定的Filesize
      + 一个文件url数小于100
      + 一次hash分割后FileSize仍然大于设定的Filesize
      + 一个文件url全部相同
      
  ## 5. 编译和执行
      + 使用Makefile 编译，使用-std=c++11
      + ./PingCAP URLFile [FileSize]
      + FileSize是根据内存设定的FileSize，单位时500M，默认5M
      
  ## 6. 一些思考
      + 虽然给了1G内存，但把文件大小设定成500M，主要的原因是我使用unorder_map, 它的内存是不能完全量化，如果使用自己控制的内存结构，文件大小可以提升很多，打造自己的内存分配器，减少外存使用，可以大大提高数据库的计算性能
   

      
