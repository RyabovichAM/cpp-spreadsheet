#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Sheet::SetCell: out of range");
    }

    Resize(pos);
    if(cells_[pos.row][pos.col].get() == nullptr) {
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    dynamic_cast<Cell*>(cells_[pos.row][pos.col].get())->Set(std::move(text),pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Sheet::GetCell: out of range");
    }

    if(!(min_print_size_.rows - 1 < pos.row || 
        min_print_size_.cols - 1 < pos.col)) {
        return cells_.at(pos.row).at(pos.col).get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Sheet::GetCell: out of range");
    }

    if(!(min_print_size_.rows - 1 < pos.row || 
        min_print_size_.cols - 1 < pos.col)) {
        return cells_.at(pos.row).at(pos.col).get();
    }
    return nullptr;
}

std::unique_ptr<CellInterface>& Sheet::GetUniqPtrCell(Position pos) {
    return cells_[pos.row][pos.col];
}

void Sheet::ClearCell(Position pos) {
    if(pos.col < 0 || pos.row < 0 || pos.col >= Position::MAX_COLS || 
        pos.row >= Position::MAX_ROWS){
            throw InvalidPositionException("Sheet::ClearCell: out of range");
        }

    if(!(min_print_size_.rows - 1 < pos.row || 
        min_print_size_.cols - 1 < pos.col)) {

        dynamic_cast<Cell*>(cells_[pos.row][pos.col].get())->Clear();
        cells_[pos.row][pos.col].reset(nullptr);

        for(int row = min_print_size_.rows - 1;row >= 0; --row) {
            int brk = false;
            for(int col = min_print_size_.cols - 1;col >= 0; --col) {
                if(cells_[row][col].get() != nullptr) {
                    cells_.resize(row+1);
                    brk = true;
                    break;
                }
                if(row==0 && col == 0) {
                    cells_.resize(0);
                }
            }
            if(brk) {
                break;
            }
        }
        min_print_size_.rows = cells_.size();

        for(int col = min_print_size_.cols - 1;col >= 0; --col) {
            int brk = false;
            for(int row = min_print_size_.rows - 1;row >= 0; --row) {
                if(cells_[row][col].get() != nullptr) {
                    for(auto& r : cells_) {
                        r.resize(col+1);
                    }
                    brk = true;
                    break;
                }
            }       
            if(brk) {
                break;
            }
        }
        if(!cells_.empty()) {
            min_print_size_.cols = cells_[0].size();
        } else {
            min_print_size_.cols = 0;
        }

    }
}

Size Sheet::GetPrintableSize() const {
    return min_print_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int i = 0; i < min_print_size_.rows; ++i) {
        for(int k = 0; k < min_print_size_.cols; ++k) {
            if(cells_[i][k].get() != nullptr) {
                if(std::holds_alternative<std::string>(cells_[i][k]->GetValue())) {
                    output << std::get<std::string>(cells_[i][k]->GetValue());
                }
                if(std::holds_alternative<double>(cells_[i][k]->GetValue())) {
                    output << std::get<double>(cells_[i][k]->GetValue());
                }
                if(std::holds_alternative<FormulaError>(cells_[i][k]->GetValue())) {
                    output << std::get<FormulaError>(cells_[i][k]->GetValue());
                }
            }
            if(k != min_print_size_.cols-1) {
                output << '\t';
            }
        }
        output << '\n';
    }
    
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(int i = 0; i < min_print_size_.rows; ++i) {
        for(int k = 0; k < min_print_size_.cols; ++k) {
            if(cells_[i][k].get() != nullptr) {
                output << cells_[i][k]->GetText();
            }
            if(k != min_print_size_.cols-1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::Resize(Position pos) {
    if(cells_.size() < static_cast<size_t>(pos.row+1)) {
        size_t old_size = cells_.size();
        cells_.resize(pos.row+1);

        for(int i = old_size; i < static_cast<int>(cells_.size()); ++i) {
            cells_[i].resize(cells_[0].size());
        }
    }

    if(min_print_size_.rows <= pos.row) {
        min_print_size_.rows = pos.row + 1;
    }

    if(cells_[0].size() < static_cast<size_t>(pos.col + 1)) {
        for(auto& col : cells_) {
            col.resize(pos.col + 1);
        }
    }

    if(min_print_size_.cols <= pos.col) {
        min_print_size_.cols = pos.col + 1;
    }
}
