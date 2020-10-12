#pragma once

#include <vector>
#include <iostream>


namespace task {

static constexpr double EPS = 1e-6;

class OutOfBoundsException : public std::exception {};
class SizeMismatchException : public std::exception {};

// Row declaration
class Row {
  size_t m_size = 0;
  double *m_row = nullptr;

private:
  // Free all data
  void clear();

public:
  // constructors
  explicit Row(size_t size = 0);
  Row(const Row &rhs);
  Row &operator=(const Row &rhs);
  ~Row();

  // getters
  double &operator[](size_t col);
  const double &operator[](size_t col) const;
};

class RowView {
  Row *m_row = nullptr;
public:
  explicit RowView(Row &row);
  double &operator[](size_t col);
  const double &operator[](size_t col) const;
};

// Matrix declaration
class Matrix {
private:
  // Matrix == array of rows
  size_t m_rows = 0;
  size_t m_cols = 0;
  Row *m_data = nullptr;

  // Defaults
  static constexpr size_t default_size = 1;
  static constexpr double diag_default = 1;
  static constexpr double off_diag_default = 0;

private:
  // Init with ones on the main diagonal, zeros otherwise
  void initialize(double diag_value, double off_diag_value);

  // Free all data
  void clear();

  // Get upper triangular form of original matrix by gauss elimination
  Matrix upperTriangularForm() const;

  // Update matrix by Func(Matrix&, size_t, size_t)
  template<typename Func>
  void transform(Func f) {
    for (size_t row = 0; row < m_rows; row++) {
      for (size_t col = 0; col < m_cols; col++) {
        f(*this, row, col);
      }
    }
  }

public:
    // constructors
  Matrix();
  Matrix(size_t rows, size_t cols, double diag_value = diag_default,
         double off_diag_value = off_diag_default);
  Matrix(const Matrix &rhs);
  Matrix(Matrix &&rhs) noexcept;
  Matrix &operator=(const Matrix &rhs);
  Matrix &operator=(Matrix &&rhs) noexcept;
  ~Matrix();

  double &get(size_t row, size_t col);
  const double &get(size_t row, size_t col) const;
  void set(size_t row, size_t col, const double &value);
  void resize(size_t new_rows, size_t new_cols);

  RowView operator[](size_t row);
  const RowView operator[](size_t row) const;

  Matrix &operator+=(const Matrix &rhs);
  Matrix &operator-=(const Matrix &rhs);
  Matrix &operator*=(const Matrix &rhs);
  Matrix &operator*=(const double &number);

  Matrix operator+(const Matrix &rhs) const;
  Matrix operator-(const Matrix &rhs) const;
  Matrix operator*(const Matrix &rhs) const;
  Matrix operator*(const double &number) const;

  Matrix operator-() const;
  Matrix operator+() const;

  void transpose();
  Matrix transposed() const;
  double trace() const;
  double det() const;

  size_t getRows() const;
  size_t getCols() const;

  std::vector<double> getRow(size_t row) const;
  std::vector<double> getColumn(size_t column) const;

  bool operator==(const Matrix &a) const;
  bool operator!=(const Matrix &a) const;
};

Matrix operator*(const double &a, const Matrix &b);
std::ostream &operator<<(std::ostream &output, const Matrix &matrix);
std::istream &operator>>(std::istream &input, Matrix &matrix);

}  // namespace task
