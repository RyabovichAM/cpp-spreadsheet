#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(SheetInterface& sheet)
    : sheet_{sheet}, impl_{std::make_unique<EmptyImpl>()} {
}

void Cell::Set(std::string text) {
    if(text[0] == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(sheet_, std::move(text.substr(1)));
        return;
    }

    if(text.size() == 0) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }

    impl_ = std::make_unique<TextImpl>(std::move(text));
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
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
