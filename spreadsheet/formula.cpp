#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include <set>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
    catch (...) {
        throw FormulaException("formula is syntactically incorrect");
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
			
            std::function<double(Position)> args = [&sheet, this](Position pos) {
                if (!pos.IsValid()) {
                    throw FormulaError(FormulaError::Category::Ref);
                }
                return GetCellDoubleValue(sheet.GetCell(pos));
                };
			
            return ast_.Execute(args);
        }
        catch (const FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream os;
        ast_.PrintFormula(os);
        return os.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        
        std::set<Position> s_cells;
        for (const auto& cell : ast_.GetCells()) {

            if (cell.IsValid()) {
                s_cells.insert(cell);
            }
            else {
                continue;
            }
        }
        std::vector<Position> cells(s_cells.begin(), s_cells.end());
        return cells;
    }
	
private:	
	double GetCellDoubleValue(const CellInterface* cell) const {
		if (cell) {                  
            if (std::holds_alternative<double>(cell->GetValue())) {
                return std::get<double>(cell->GetValue());
                
            } 
			else if (std::holds_alternative<std::string>(cell->GetValue())) {
                std::string value = std::get<std::string>(cell->GetValue());
                if (value != "") {
                    std::istringstream input(value);
                    double num = 0.0;
                    input >> num;
                    if (input.eof() && input) {
                        return num;
                    } 
					else {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                } 
				else {
                    return 0.0;
                }     
            } 
			else {
                throw FormulaError(std::get<FormulaError>(cell->GetValue()));
            }
        } 
		else {
			return 0.0;
		}
	}

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}