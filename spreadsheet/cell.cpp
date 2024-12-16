#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(Sheet& sheet)
    : sheet_{sheet}, impl_{std::make_unique<EmptyImpl>()} {
}

void Cell::Set(const std::string& text, Position pos) {
    if(GetText() == text) {
        return;
    }

    if(text[0] == '=') {
        CheckCyclicDependences(text,pos);
    }

    CacheInvalidate();

    for(auto cell_pos : GetReferencedCells()) {
        Cell* curr_cell = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
        if(curr_cell) {
            curr_cell->RemoveDependentCell(this);
        }
    }

    if(text[0] == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(sheet_, text.substr(1));
    } else if(text.size() == 0) {
        impl_ = std::make_unique<EmptyImpl>();
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }


    for(auto cell_pos : GetReferencedCells()) {
        sheet_.Resize(cell_pos);
        if(sheet_.GetUniqPtrCell(cell_pos).get() == nullptr) {
            sheet_.GetUniqPtrCell(cell_pos) = std::make_unique<Cell>(sheet_);
        }
        dynamic_cast<Cell*>(sheet_.GetUniqPtrCell(cell_pos).get())->AddDependentCell(this);
    }

}

void Cell::Clear() {
    Set("", {0,0});
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !impl_->GetReferencedCells().empty();
}

void Cell::AddDependentCell(Cell* cell) {
    dependent_cells_.insert(cell);
}

void Cell::RemoveDependentCell(Cell* cell) {
    dependent_cells_.erase(cell);
}

void Cell::CacheInvalidate() {
    impl_->ClearCache();
    for(Cell* cell : dependent_cells_) {
        cell->CacheInvalidate();
    }
}

void Cell::CheckCyclicDependences(const std::string& cell_text, Position pos) const {
    std::unique_ptr<FormulaInterface> form = ParseFormula(cell_text.substr(1));
    std::unordered_set<Position,PositionHasher> tmp_cells;
    tmp_cells.insert(pos);
    CheckCyclicDependences(form->GetReferencedCells(), tmp_cells);
}

void Cell::CheckCyclicDependences(const std::vector<Position>& poses, std::unordered_set<Position,PositionHasher>& tmp_cells) const {
    for (auto cell_pos : poses) {
        if(tmp_cells.count(cell_pos)) {
            throw CircularDependencyException("Circular Dependency");
        }
        tmp_cells.insert(cell_pos);
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
        if(cell != nullptr) {
            CheckCyclicDependences(cell->GetReferencedCells(), tmp_cells);
        }
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

void Cell::Impl::ClearCache() {
}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

Cell::TextImpl::TextImpl(std::string text) : text_{std::move(text)} {
}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if(text_[0] == '\'') {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(const SheetInterface& sheet,std::string formula) :
    sheet_{sheet}, formula_{std::move(ParseFormula(std::string(formula)))} {\
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    if(cache_ != std::nullopt) {
        return cache_.value();
    }

    FormulaInterface::Value eval = formula_->Evaluate(sheet_);

    if(std::holds_alternative<double>(eval)) {
        cache_  = std::get<double>(eval);
    }
    if(std::holds_alternative<FormulaError>(eval)) {
        cache_ =  std::get<FormulaError>(eval);
    }
    return cache_.value();
}

std::string Cell::FormulaImpl::GetText() const {
    return {'=' + formula_->GetExpression()};
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::ClearCache() {
    cache_ = std::nullopt;
}
