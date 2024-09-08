#pragma once
#include <string>
class ApplyMsg {
public:
    bool CommandValid{};
    std::string Command;
    int CommandIndex{};
    bool SnapshotValid{};
    std::string Snapshot;
    int SnapshotTerm{};
    int SnapshotIndex{};

public:
    // 两个valid最开始要赋予false！！
    ApplyMsg()
        : Command(){};
};
