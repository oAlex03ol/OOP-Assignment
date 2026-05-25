#pragma once
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <stdexcept>

namespace fs = std::filesystem;

enum class NodeType { File, Folder };

class FSNode {
    friend class FolderNode;

protected:
    fs::path path;
    NodeType type;

    virtual void updatePath(const fs::path& newPath) {
        path = newPath;
    }

public:
    FSNode(const fs::path& p, NodeType t) : path(p), type(t) {}
    virtual ~FSNode() = default;

    std::string getName() const { return path.filename().string(); }
    fs::path getPath() const { return path; }
    NodeType getType() const { return type; }

    void renameTo(const std::string& newName) {
        if (newName.empty() || newName == "." || newName == ".." ||
            newName.find('/') != std::string::npos ||
            newName.find('\\') != std::string::npos) {
            throw std::runtime_error("Invalid file or folder name.");
        }

        fs::path newPath = path.parent_path() / newName;
        if (newPath == path) return;
        if (fs::exists(newPath)) {
            throw std::runtime_error("An item with the same name already exists.");
        }

        fs::rename(path, newPath);
        updatePath(newPath);
    }

    // 新增虛擬函式：取得原始位元組大小
    virtual uintmax_t getSize() const {
        if (type == NodeType::Folder) return 0; // Folder 會在子類別中覆寫，這裡預設為 0
        try {
            return fs::file_size(path);
        } catch (...) {
            return 0;
        }
    }

    // 改寫字串轉換邏輯，讓它直接讀取虛擬函式 getSize() 計算出來的結果
    std::string getSizeString() const {
        uintmax_t size = getSize();
        if (size == 0 && type == NodeType::Folder) return "0 B"; // 空資料夾顯示 0 B
        
        if (size < 1024) return std::to_string(size) + " B";
        if (size < 1024 * 1024) return std::to_string(size / 1024) + " KB";
        return std::to_string(size / (1024 * 1024)) + " MB";
    }

    std::string getTimeString() const {
        try {
            auto ftime = fs::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&ctime), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        } catch (...) {
            return "Unknown";
        }
    }
};

class FolderNode : public FSNode {
private:
    std::vector<std::shared_ptr<FSNode>> children;

protected:
    void updatePath(const fs::path& newPath) override {
        path = newPath;
        for (const auto& child : children) {
            child->updatePath(path / child->getName());
        }
    }

public:
    FolderNode(const fs::path& p) : FSNode(p, NodeType::Folder) {}

    // 實作 DFS 遞迴：計算所有子節點（檔案與子資料夾）的總大小
    uintmax_t getSize() const override {
        uintmax_t totalSize = 0;
        for (const auto& child : children) {
            totalSize += child->getSize(); 
        }
        return totalSize;
    }

    std::shared_ptr<FolderNode> createChildFolder(const std::string& name) {
        fs::path childPath = path / name;
        fs::create_directories(childPath); 
        auto child = std::make_shared<FolderNode>(childPath);
        children.push_back(child);
        return child;
    }

    std::shared_ptr<FSNode> createChildFile(const std::string& name, uintmax_t size = 0) {
        fs::path childPath = path / name;
        fs::create_directories(childPath.parent_path()); 
        std::ofstream file(childPath, std::ios::binary | std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Unable to create file.");
        }
        if (size > 0) {
            file.seekp(static_cast<std::streamoff>(size - 1));
            file.put('\0');
            if (!file) {
                throw std::runtime_error("Unable to set file size.");
            }
        }
        file.close();
        auto child = std::make_shared<FSNode>(childPath, NodeType::File);
        children.push_back(child);
        return child;
    }

    const std::vector<std::shared_ptr<FSNode>>& getChildren() const { return children; }
};
