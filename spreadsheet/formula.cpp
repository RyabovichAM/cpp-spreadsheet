#include "formula.h"
#include "cell.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try : ast_{ParseFormulaAST(std::move(expression))} {
        for (const auto cell : ast_.GetCells()) {
            referenced_cells_.push_back(cell);
        }
        auto uniq_end = std::unique(referenced_cells_.begin(), referenced_cells_.end());
        referenced_cells_.erase(uniq_end,referenced_cells_.end());
    } catch(std::exception &) {
        throw FormulaException("ParseFormula error");
    }

    FormulaInterface::Value Evaluate(const SheetInterface& sheet) const override {
        FormulaInterface::Value result;

        auto Exec = [&sheet](const Position pos) {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }
            double result;

            auto cell = sheet.GetCell(pos);

            if (cell == nullptr) {
                return 0.0;
            }
            CellInterface::Value val = cell->GetValue();

            if (std::holds_alternative<double>(val)) {
                result = std::get<double>(val);
            }

            if (std::holds_alternative<FormulaError>(val)) {
                throw std::get<FormulaError>(val);
            }

            if (std::holds_alternative<std::string>(val)) {
                if(std::get<std::string>(val).empty()) {
                    return 0.0;
                }
                try {
                    result = std::stod(std::get<std::string>(val));
                } catch (std::invalid_argument& err) {
                    throw FormulaError(FormulaError::Category::Value);
                }
            }

            return result;
        };

        try {
            result = ast_.Execute(Exec);
        } catch (FormulaError& err) {
            result = err;
        }
        return result;
    }
    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const {
        return referenced_cells_;
    }

private:
    FormulaAST ast_;
    std::vector<Position> referenced_cells_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
