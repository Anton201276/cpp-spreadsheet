#pragma once
#include <optional>
#include <stack>
#include <set>
#include <vector>

#include "common.h"
#include "formula.h"

class Sheet;

class Cell : public CellInterface {
public:
    //enum class GraphSign {
    //    GS_WT,
    //    GS_RS,
    //    GS_RD
    //};

    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCellsCache() const;


private:
    //void SetCellPtrIn() const;
    void CheckCircularDependencyException(const std::vector<Position>& refcell) const;
    void UpdateDependentCells();

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;            // указатель на ячейку - пустую, текстовую, формульную
    std::set<Cell*> formulas_cells_;       // контейнер для хранения указателей на ячейки для формулы
    std::set<Cell*> dependent_cells_;      // контейнер для хранения указателей на ячейки с другими формулами
    Sheet& sheet_;                          // ссылка на лист таблицы
    //enum GraphSign cell_gs_;                     // хранение цвета ячейки для алгоритма поиска циклов в формуле  

    class Impl {
       public:
           virtual ~Impl() = default;
           virtual Value GetValue() const = 0;
           virtual std::string GetText() const;
           virtual std::vector<Position> GetReferencedCells() const;
           virtual bool HasCache();
           virtual void InvalidateCache();
    };

    class EmptyImpl : public Impl {
        public:
            Value GetValue() const override;
            std::string GetText() const override;
    };

    class TextImpl : public Impl {
        public:
            explicit TextImpl(std::string text);
            Value GetValue() const override;
            std::string GetText() const override;

        private:
            std::string text_;
    };

    class FormulaImpl : public Impl {
        public:
        FormulaImpl(std::string text, SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool HasCache() override;
        void InvalidateCache() override;

        private:
            std::unique_ptr<FormulaInterface> cell_formula_;
            mutable std::optional<FormulaInterface::Value> cache_;
            SheetInterface& sheet_;
    };
};