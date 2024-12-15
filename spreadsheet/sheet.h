#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class PositionHasher {
public:
    size_t operator()(const Position& pos) const  {
        return pos.row * 37 + pos.col;
    }
};

class Sheet : public SheetInterface {
public:
    using CellsMatrix = std::vector<std::vector<std::unique_ptr<CellInterface>>>;

    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void CheckCyclicDependences(Position) const;

private:
    CellsMatrix cells_;
    Size min_print_size_{0,0};

    void Resize(Position);
    void CheckCyclicDependences(Position, std::unordered_set<Position,PositionHasher>&) const;
};
