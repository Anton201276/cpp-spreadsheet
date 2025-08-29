#include "sheet.h"
#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }

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
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }

    if (pos.row < static_cast<int>(table_.size()) && pos.col < static_cast<int>(table_[pos.row].size())) {
        if (table_[pos.row][pos.col] == nullptr
            || table_[pos.row][pos.col]->GetText() == "") {
            return nullptr;
        }
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {

    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }

    if (pos.row < static_cast<int>(table_.size()) && pos.col < static_cast<int>(table_[pos.row].size())) {
        if (table_[pos.row][pos.col].get()->GetText().empty()) {
            table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        }
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

Cell* Sheet::GetCellPtr(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }

    if (pos.row < static_cast<int>(table_.size()) && pos.col < static_cast<int>(table_[pos.row].size())) {
        return table_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell address");
    }

    if (pos.row < static_cast<int>(table_.size()) && pos.col < static_cast<int>(table_[pos.row].size())) {
        table_[pos.row][pos.col]->Clear();
    }
}

Size Sheet::GetPrintableSize() const {

    int col_max = 0;
    int row_cnt = 0;
    for (int i = 0; i < static_cast<int>(table_.size()); ++i) {
        int pos_col = 0;
        for (int j = 0; j < static_cast<int>(table_[i].size()); ++j) {
            if (GetCell({i,j}) != nullptr) {
                pos_col = j + 1;
            }
        }

        if (pos_col > 0) {
            col_max = std::max(col_max, pos_col);
            row_cnt = i + 1;
        }
    }
    return { row_cnt,col_max };
}

void Sheet::PrintValues(std::ostream& output) const {

    Size size = GetPrintableSize();

    for (int i = 0; i < size.rows; ++i) {
        bool is_first = true;
        for (int j = 0; j < size.cols; ++j) {
            if (!is_first) {
                output << '\t';
            }
            if (j < static_cast<int>(table_[i].size()) && GetCell({i,j})) {
                
                if (std::holds_alternative<double>(table_[i][j]->GetValue())) {
                    output << std::get<double>(table_[i][j]->GetValue());
                }
                else if (std::holds_alternative<std::string>(table_[i][j]->GetValue())) {
                    output << std::get<std::string>(table_[i][j]->GetValue());
                }
                else {
                    output << std::get<FormulaError>(table_[i][j]->GetValue());
                }
                
            }
            is_first = false;
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {

    Size size = GetPrintableSize();

    for (int i = 0; i < size.rows; ++i) {
        bool is_first = true;
        for (int j = 0; j < size.cols; ++j) {
            if (!is_first) {
                output << '\t';
            }
            if (j < static_cast<int>(table_[i].size()) && GetCell({ i,j })) {
                output << table_[i][j]->GetText();
            }
            is_first = false;
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
