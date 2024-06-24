#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>  // For strlen and strstr
#include <thread>
#include <mutex>

// es zamanli erisim icin 
std::mutex outputMutex;

// Function to process a range of files
void processFiles(int startIndex, int endIndex, const std::string& folderPath, const std::string& subText) {
    for (int i = startIndex; i <= endIndex; ++i) {
        std::string fileName = folderPath + "Lord" + std::to_string(i) + ".txt";
        std::ifstream inputFile(fileName);

        if (!inputFile) {
            std::lock_guard<std::mutex> guard(outputMutex);
            std::cerr << "Failed to open the file: " << fileName << std::endl;
            continue;
        }

        std::vector<std::string> lines;
        std::string line;

        while (std::getline(inputFile, line)) {
            lines.push_back(line);
        }

        inputFile.close();
//performens test
        for (size_t j = 0; j < lines.size(); ++j) {
            const char* foundPos = strstr(lines[j].c_str(), subText.c_str());
            if (foundPos) {
                volatile size_t dummy = 0;
                for (volatile size_t l = 0; l < 100000; ++l) {
                    dummy += l;
                }

                size_t position = foundPos - lines[j].c_str();
                std::lock_guard<std::mutex> guard(outputMutex);
                std::cout << "Substring found in file " << fileName
                          << " at line " << (j + 1) << ", position " << position << std::endl;
            }
        }
    }
}

int main() {
    const std::string folderPath = "test/"; // Folder path
    const std::string subText = "had"; // Substring to search for
    const int fileCount = 800; // Number of files
    int threadCount = std::thread::hardware_concurrency();

    if (threadCount == 0) {
        threadCount = 16; // Default to 8 threads if hardware_concurrency returns 0
    }

    std::vector<std::thread> threads;
    int filesPerThread = fileCount / threadCount;//Dosyaları eşit olarak iş parçacıklarına dağıtmak için
    // iş parçacığı başına dosya sayısı hesaplanır
    int remainingFiles = fileCount % threadCount;//Artan dosya sayısını takip eder ve iş parçacıklarına dağıtır

    int startIndex = 1;
  for (int i = 0; i < threadCount; ++i) {
        int endIndex = startIndex + filesPerThread - 1;
        if (remainingFiles > 0) {
            endIndex++;
            remainingFiles--;
        }
//Her iş parçacığı belirli bir dosya aralığını işlemek üzere başlatılır ve threads vektörüne eklenir
  
        std::cout << "Starting thread for files " << startIndex << " to " << endIndex << std::endl;
        threads.emplace_back(processFiles, startIndex, endIndex, folderPath, subText);
        startIndex = endIndex + 1;
    }
// hepsi tamamlana kadar bekler
    for (std::thread& t : threads) {
        t.join();
    }

    std::cout << "All threads completed." << std::endl;
    return 0;
}
