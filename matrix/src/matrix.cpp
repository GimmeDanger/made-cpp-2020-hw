#include <algorithm>
#include <cmath>

#include "matrix.h"

using namespace task;

/////////////////////////// Row implementation
Row::Row(size_t size)
  : m_size(size)
  , m_row(m_size > 0 ? new double[m_size] : nullptr)
{
}

Row::Row(const Row &rhs)
  : m_size(rhs.m_size)
  , m_row(m_size > 0 ? new double[m_size] : nullptr)
{
  if (m_size > 0) {
    std::copy(rhs.m_row, rhs.m_row + rhs.m_size, m_row);
  }
}

Row &Row::operator=(const Row &rhs) {
  if (this != &rhs) {
    clear();
    m_size = rhs.m_size;
    if (m_size > 0) {
      m_row = new double[m_size];
      std::copy(rhs.m_row, rhs.m_row + rhs.m_size, m_row);
    }
  }
  return *this;
}

Row::~Row() {
  clear();
}

void Row::clear() {
  if (m_size > 0) {
    delete[] m_row;
    m_row = nullptr;
  }
  m_size = 0;
}

double &Row::operator[](size_t col) {
  if (col >= m_size)
    throw OutOfBoundsException{};
  return m_row[col];
}

const double &Row::operator[](size_t col) const {
  return const_cast<Row *>(this)->operator[](col);
}

/////////////////////////// Row view implementation

RowView::RowView(Row &row)
  : m_row(&row)
{
}

double &RowView::operator[](size_t col) {
  return m_row->operator[](col);
}

const double &RowView::operator[](size_t col) const {
  return m_row->operator[](col);
}
/////////////////////////// Matrix implementation

Matrix::Matrix()
  : m_rows(default_size)
  , m_cols(default_size)
  , m_data(m_rows > 0 ? new Row[m_rows] : nullptr)
{
  for (size_t r = 0; r < m_rows; r++) {
    m_data[r] = Row(m_cols);
  }
  initialize(diag_default, off_diag_default);
}

Matrix::Matrix(size_t rows, size_t cols, double diag_value, double off_diag_value)
    : m_rows(rows)
    , m_cols(cols)
    , m_data(m_rows > 0 ? new Row[m_rows] : nullptr)
{
  for (size_t r = 0; r < m_rows; r++) {
    m_data[r] = Row(m_cols);
  }
  initialize(diag_value, off_diag_value);
}

Matrix::Matrix(const Matrix &rhs)
  : m_rows(rhs.m_rows)
  , m_cols(rhs.m_cols)
  , m_data(m_rows > 0 ? new Row[m_rows] : nullptr)
{
  for (size_t r = 0; r < m_rows; r++) {
    if (m_cols > 0) {
      m_data[r] = rhs.m_data[r];
    }
  }
}

Matrix::Matrix(Matrix &&rhs) noexcept
  : m_rows(rhs.m_rows)
  , m_cols(rhs.m_cols)
  , m_data(rhs.m_data)
{
  rhs.m_rows = 0;
  rhs.m_cols = 0;
  rhs.m_data = nullptr;
}

Matrix &Matrix::operator=(const Matrix &rhs) {
  if (this != &rhs) {
    clear();
    m_rows = rhs.m_rows;
    m_cols = rhs.m_cols;
    if (m_rows > 0) {
      m_data = new Row[m_rows];
      for (size_t r = 0; r < m_rows; r++) {
        if (m_cols > 0) {
          m_data[r] = rhs.m_data[r];
        }
      }
    }
  }
  return *this;
}

Matrix &Matrix::operator=(Matrix &&rhs) noexcept {
  if (this != &rhs) {
    clear();
    std::swap(m_rows, rhs.m_rows);
    std::swap(m_cols, rhs.m_cols);
    std::swap(m_data, rhs.m_data);
  }
  return *this;
}

Matrix::~Matrix() {
  clear();
}

void Matrix::clear() {
  if (m_data) {
    for (size_t r = 0; r < m_rows; r++) {
      m_data[r].~Row();
    }
    delete[] m_data;
    m_data = nullptr;
  }
  m_rows = m_cols = 0;
}

void Matrix::initialize(double diag_value, double off_diag_value) {
  for (size_t r = 0; r < m_rows; r++) {
    for (size_t c = 0; c < m_cols; c++) {
      m_data[r][c] = (r == c) ? diag_value : off_diag_value;
    }
  }
}

double &Matrix::get(size_t row, size_t col) {
  return (*this)[row][col];
}

const double &Matrix::get(size_t row, size_t col) const {
  return (*this)[row][col];
}

void Matrix::set(size_t row, size_t col, const double& value) {
  (*this)[row][col] = value;
}

RowView Matrix::operator[](size_t row) {
  if (row >= m_rows)
    throw OutOfBoundsException{};
  return RowView(m_data[row]);
}

const RowView Matrix::operator[](size_t row) const {
  return const_cast<Matrix *>(this)->operator[](row);
}

void Matrix::resize(size_t new_rows, size_t new_cols) {
  Matrix that = Matrix(new_rows, new_cols, off_diag_default, off_diag_default);
  for (size_t r = 0; r < std::min(m_rows, that.m_rows); r++) {
    for (size_t c = 0; c < std::min(m_cols, that.m_cols); c++) {
      that[r][c] = get(r, c);
    }
  }
  *this = std::move(that);
}

bool Matrix::operator==(const Matrix &rhs) const {
  if (m_rows != rhs.m_rows || m_cols != rhs.m_cols)
    throw SizeMismatchException{};
  for (size_t r = 0; r < m_rows; r++) {
    for (size_t c = 0; c < m_cols; c++) {
      if (fabs ((*this)[r][c] - rhs[r][c]) > EPS)
        return false;
    }
  }
  return true;
}

bool Matrix::operator!=(const Matrix &rhs) const {
  return !(*this == rhs);
}

std::vector<double> Matrix::getRow(size_t row) const {
  std::vector<double> res(m_cols);
  for (size_t col = 0; col < m_cols; col++) {
    res[col] = (*this)[row][col];
  }
  return res;
}

std::vector<double> Matrix::getColumn(size_t col) const {
  std::vector<double> res(m_rows);
  for (size_t row = 0; row < m_rows; row++) {
    res[row] = (*this)[row][col];
  }
  return res;
}

Matrix &Matrix::operator+=(const Matrix &rhs) {
  if (m_rows != rhs.m_rows || m_cols != rhs.m_cols)
    throw SizeMismatchException{};
  transform([&rhs](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] += rhs[row][col];
  });
  return *this;
}

Matrix &Matrix::operator-=(const Matrix &rhs) {
  if (m_rows != rhs.m_rows || m_cols != rhs.m_cols)
    throw SizeMismatchException{};
  transform([&rhs](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] -= rhs[row][col];
  });
  return *this;
}
Matrix &Matrix::operator*=(const Matrix &rhs) {
  *this = this->operator*(rhs);
  return *this;
}

Matrix &Matrix::operator*=(const double &number) {
  transform([number](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] *= number;
  });
  return *this;
}

Matrix Matrix::operator+(const Matrix &rhs) const {
  if (m_rows != rhs.m_rows || m_cols != rhs.m_cols)
    throw SizeMismatchException{};
  Matrix res = *this;
  res.transform([&rhs](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] += rhs[row][col];
  });
  return res;
}

Matrix Matrix::operator-(const Matrix &rhs) const {
  if (m_rows != rhs.m_rows || m_cols != rhs.m_cols)
    throw SizeMismatchException{};
  Matrix res = *this;
  res.transform([&rhs](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] -= rhs[row][col];
  });
  return res;
}

// Returns A[n x m] * B[m * k] = C[n x k],
// where C[i][j] = sum_{s} (A[i][s] x B[s][j])
Matrix Matrix::operator*(const Matrix &rhs) const {
  if (m_cols != rhs.m_rows)
    throw SizeMismatchException{};
  Matrix res = Matrix(m_rows, rhs.m_cols);
  const size_t common_dimension = m_cols;
  for (size_t i = 0; i < res.getRows(); i++) {
    for (size_t j = 0; j < res.getCols(); j++) {
      double value = 0;
      for (size_t s = 0; s < common_dimension; s++) {
        value += (*this)[i][s] * rhs[s][j];
      }
      res[i][j] = value;
    }
  }
  return res;
}
Matrix Matrix::operator*(const double &number) const {
  Matrix res = *this;
  res.transform([number](Matrix &lhs, size_t row, size_t col) {
    lhs[row][col] *= number;
  });
  return res;
}

Matrix Matrix::operator-() const {
  return this->operator*(-1.0);
}

Matrix Matrix::operator+() const {
  return *this;
}

size_t Matrix::getRows() const {
  return m_rows;
}

size_t Matrix::getCols() const {
  return m_cols;
}

double Matrix::trace() const {
  if (m_rows != m_cols)
    throw SizeMismatchException{};
  double sum = 0;
  for (size_t row = 0; row < m_rows; row++) {
    sum += (*this)[row][row];
  }
  return sum;
}

Matrix Matrix::transposed() const {
  Matrix res(m_cols, m_rows);
  for (size_t row = 0; row < m_rows; row++) {
    for (size_t col = 0; col < m_cols; col++) {
      res[col][row] = (*this)[row][col];
    }
  }
  return res;
}

void Matrix::transpose() {
  *this = transposed();
}

Matrix Matrix::upperTriangularForm() const {
  constexpr double min_on_diagonal = 1.e-12;
  constexpr double min_for_division = 1.e-64;
  Matrix matrix_a = (*this);

  if (matrix_a.m_rows <= 1)
    return matrix_a;

  // direct sweep
  const size_t dim = matrix_a.getRows();
  for (size_t k = 0; k < dim - 1; k++) {

    double pivot = matrix_a[k][k];
    // zero column case
    if (fabs (pivot) < min_on_diagonal) {
      continue;
    }

    // subtract k-th row from lower rows
    // a_{ij} = a_{ij} - a_{ik} * a_{kj} / a_{kk}
    auto rv_k = matrix_a[k];
    for (size_t i = k + 1; i < dim; i++) {
      auto rv_i = matrix_a[i];
      double t = rv_i[k];
      rv_i[k] = 0.;
      for (size_t j = k + 1; j < dim; j++) {
        if (fabs(rv_k[j]) > min_for_division)
          rv_i[j] -= t * rv_k[j] / pivot;
      }
    }
  }
  return matrix_a;
}

double Matrix::det() const {
  if (m_rows != m_cols)
    throw SizeMismatchException{};
  Matrix mat = upperTriangularForm();
  double det = 1;
  for (size_t row = 0; row < m_rows; row++)
    det *= mat[row][row];
  return det;
}

Matrix task::operator*(const double &a, const Matrix &b) {
  return b * a;
}

std::ostream &task::operator<<(std::ostream &output, const Matrix &matrix) {
  size_t rows = matrix.getRows(), cols = matrix.getCols();
  for (size_t row = 0; row < rows; row++) {
    for (size_t col = 0; col < cols; col++) {
      output << matrix[row][col];
      if (col != cols - 1)
        output << " ";
      else
        output << "\n";
    }
  }
  return output;
}

std::istream &task::operator>>(std::istream &input, Matrix &matrix) {
  size_t rows, cols;
  input >> rows >> cols;
  matrix.resize(rows, cols);
  for (size_t row = 0; row < rows; row++) {
    for (size_t col = 0; col < cols; col++) {
      input >> matrix[row][col];
    }
  }
  return input;
}
