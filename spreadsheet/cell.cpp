#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
    sheet_(sheet) {
}
Cell::~Cell() = default;

void Cell::Set(std::string text) {

    if (text == this->GetText()) {
        return;
    }

    std::unique_ptr<Impl> impl_tmp;
        if (text.size() > 1 && text[0] == FORMULA_SIGN) {
             impl_tmp = std::make_unique<FormulaImpl>(std::move(text), sheet_);
             const auto tmp_ref_cells_in = impl_tmp->GetReferencedCells();
             if (!tmp_ref_cells_in.empty()) {
                 CheckCircularDependencyException(tmp_ref_cells_in);
             }
        }
        else if (!text.empty()) {
            impl_tmp = std::make_unique<TextImpl>(TextImpl(text));
        }
        else {
            impl_tmp = std::make_unique<EmptyImpl>(EmptyImpl());
        }
        impl_.reset();
        std::swap(impl_, impl_tmp);
        UpdateDependentCells();
        InvalidateCellsCache();
}

void Cell::Clear() {
    this->Set("");
}

Cell::Value Cell::GetValue() const {
    if ( impl_!= nullptr) {
        return impl_->GetValue();
    }
    return "";
    
}
std::string Cell::GetText() const {
    if (impl_ != nullptr) {
        return impl_->GetText();
    }
    return "";
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::InvalidateCellsCache() const{
    if (impl_->HasCache()) {
        impl_->InvalidateCache();
    }

    if (!dependent_cells_.empty()) {
        for (Cell* cell : dependent_cells_) {
            cell->InvalidateCellsCache();
        }
    }
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}

std::string Cell::Impl::GetText() const {
    return "";
}

std::vector<Position> Cell::Impl::GetReferencedCells() const { return {}; }

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}
bool Cell::Impl::HasCache() { return true; }
void Cell::Impl::InvalidateCache() {}

Cell::TextImpl::TextImpl(std::string text) : text_(text) {};
CellInterface::Value Cell::TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN) {
        return text_.substr(1, text_.length() - 1);
    }
    else {
        return text_;
    }
}
std::string Cell::TextImpl::GetText() const { return text_; }


Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) :
    cell_formula_(ParseFormula(text.substr(1, text.length() - 1))),
    sheet_(sheet){};

CellInterface::Value Cell::FormulaImpl::GetValue() const {

    if (!cache_.has_value()) {
        cache_ = cell_formula_->Evaluate(sheet_);
    }
    return std::visit([](auto& retval) {return Value(retval); }, cache_.value());
}

std::string Cell::FormulaImpl::GetText() const {
    std::string str = FORMULA_SIGN + cell_formula_->GetExpression();
    return str;
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return cell_formula_->GetReferencedCells();
}

void Cell::CheckCircularDependencyException(const std::vector<Position>& refcell) const {
    std::set<const Cell*> formula_cells;
    std::set<const Cell*> enter_cells;
    std::vector<const Cell*> bufer_cells;

    for (auto position : refcell) {
        formula_cells.insert(sheet_.GetCellPtr(position));
    }

    bufer_cells.push_back(this);

    while (!bufer_cells.empty()) {

        const Cell* ongoing = bufer_cells.back();

        bufer_cells.pop_back();
        enter_cells.insert(ongoing);

        if (formula_cells.find(ongoing) == formula_cells.end()) {

            for (const Cell* dependent : ongoing->dependent_cells_) {

                if (enter_cells.find(dependent) == enter_cells.end()) {
                    bufer_cells.push_back(dependent);
                }
            }
        }
        else {
            throw CircularDependencyException("circular dependency detected");
        }
    }
}

void Cell::UpdateDependentCells() {
    for (Cell* cell : formulas_cells_) {
        cell->dependent_cells_.erase(this);
    }

    formulas_cells_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
    
        Cell* refrenced = sheet_.GetCellPtr(pos);
    
        if (!refrenced) {
            sheet_.SetCell(pos, "");
            refrenced = sheet_.GetCellPtr(pos);
        }
    
        formulas_cells_.insert(refrenced);
        refrenced->dependent_cells_.insert(this);
    }
}

bool Cell::FormulaImpl::HasCache() { 
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() { 
    cache_.reset();
}
