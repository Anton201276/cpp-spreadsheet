#include "sheet.h"
#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    CheckCellPosition(pos);

    if (pos.row >= static_cast<int>(table_.size())) {
        table_.resize(pos.row + 1);
    }
    if (pos.col >= static_cast<int>(table_[pos.row].size())) {
            table_[pos.row].resize(pos.col + 1);
    }

    if (table_[pos.row][pos.col] == nullptr) {
        table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    table_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckCellPosition(pos);

    if (pos.row < static_cast<int>(table_.size()) 
        && pos.col < static_cast<int>(table_[pos.row].size())) {
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {

    CheckCellPosition(pos);

    if (pos.row < static_cast<int>(table_.size()) 
        && pos.col < static_cast<int>(table_[pos.row].size())) {
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

Cell* Sheet::GetCellPtr(Position pos) {
    CheckCellPosition(pos);

    if (pos.row < static_cast<int>(table_.size()) 
        && pos.col < static_cast<int>(table_[pos.row].size())) {
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    CheckCellPosition(pos);

    if (pos.row < static_cast<int>(table_.size()) 
        && pos.col < static_cast<int>(table_[pos.row].size())) {
        table_[pos.row][pos.col]->Clear();

        if (!table_[pos.row][pos.col]->IsReferenced()) {
            table_[pos.row][pos.col].reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {

    int col_max = 0;
    int row_cnt = 0;
    for (int row = 0; row < static_cast<int>(table_.size()); ++row) {
        int pos_col = 0;
        for (int col = 0; col < static_cast<int>(table_[row].size()); ++col) {
            if (GetCell({row, col}) != nullptr) {
                pos_col = col + 1;
            }
        }

        if (pos_col > 0) {
            col_max = std::max(col_max, pos_col);
            row_cnt = row + 1;
        }
    }
    return {row_cnt, col_max};
}

void Sheet::PrintValues(std::ostream& output) const {

    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < size.cols; ++col) {
            if (!is_first) {
                output << '\t';
            }
            if (col < static_cast<int>(table_[row].size()) && GetCell({ row, col })) {
                
                if (std::holds_alternative<double>(table_[row][col]->GetValue())) {
                    output << std::get<double>(table_[row][col]->GetValue());
                }
                else if (std::holds_alternative<std::string>(table_[row][col]->GetValue())) {
                    output << std::get<std::string>(table_[row][col]->GetValue());
                }
                else {
                    output << std::get<FormulaError>(table_[row][col]->GetValue());
                }
            }
            is_first = false;
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {

    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < size.cols; ++col) {
            if (!is_first) {
                output << '\t';
            }
            if (col < static_cast<int>(table_[row].size()) && GetCell({row , col})) {
                output << table_[row][col]->GetText();
            }
            is_first = false;
        }
        output << "\n";
    }
}

void Sheet::CheckCellPosition(Position pos) const{
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
