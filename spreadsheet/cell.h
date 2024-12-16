#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>

class Sheet;
class PositionHasher {
public:
    size_t operator()(const Position& pos) const  {
        return pos.row * 37 + pos.col;
    }
};

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);

    void Set(const std::string& text, Position pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
    void CacheInvalidate();
    void CheckCyclicDependences(const std::string& cell_text, Position pos) const;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
        void ClearCache();

    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        virtual CellInterface::Value GetValue() const;
        virtual std::string GetText() const;

    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);
        virtual CellInterface::Value GetValue() const;
        virtual std::string GetText() const;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(const SheetInterface& sheet,std::string formula);
        virtual ~FormulaImpl() = default;
        virtual CellInterface::Value GetValue() const;
        virtual std::string GetText() const;
        virtual std::vector<Position> GetReferencedCells() const;
        void ClearCache();

    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<CellInterface::Value> cache_ = std::nullopt;
    };

private:
    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> dependent_cells_;

    void CheckCyclicDependences(const std::vector<Position>& poses, std::unordered_set<Position,PositionHasher>& tmp_cells) const;
    void AddDependentCell(Cell*);
    void RemoveDependentCell(Cell*);
};
