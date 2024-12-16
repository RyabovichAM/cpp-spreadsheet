#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    using CellsMatrix = std::vector<std::vector<std::unique_ptr<CellInterface>>>;

    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    std::unique_ptr<CellInterface>& GetUniqPtrCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    void Resize(Position);

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
private:
    CellsMatrix cells_;
    Size min_print_size_{0,0};
};
