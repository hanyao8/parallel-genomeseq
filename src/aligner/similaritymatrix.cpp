#include "similaritymatrix.h"
#include <iostream>
#include <algorithm>
#include <Eigen/Dense>

Similarity_Matrix::Similarity_Matrix(std::string_view sequence_x, std::string_view sequence_y) :
    similarity_matrix(sequence_x.size() + 1, sequence_y.size() + 1), sequence_x(sequence_x), sequence_y(sequence_y) {
  similarity_matrix.setZero();
}

std::tuple<Eigen::Index, Eigen::Index, double> Similarity_Matrix::find_index_of_maximum() const {
  Eigen::Index x = 0, y = 0;
  auto max = similarity_matrix.maxCoeff(&x, &y);
#ifdef VERBOSE
  std::cout << "Maximum is " << similarity_matrix(x, y) << " @ (" << x << ", " << y << ")" << std::endl;
#endif
  return {x, y, max};
}

void Similarity_Matrix::print_matrix() const {
  std::cout << similarity_matrix << std::endl;
}

double dp_func(double north, double west, double north_west, double score, double gap_penalty) {
  Eigen::Vector4d v{
    north_west + score,
    west - gap_penalty,
    north - gap_penalty,
    0
  };
  return v.maxCoeff();
}

void Similarity_Matrix::iterate_anti_diagonal(const std::function<double(const char &, const char &)> &scoring_function,
                                              double gap_penalty) {
  const unsigned int dim_x = similarity_matrix.rows();
  const unsigned int dim_y = similarity_matrix.cols();

  for (Eigen::Index i = 1; i < dim_x + dim_y - 2; ++i) {
    Eigen::Index local_i = i;
    Eigen::Index starting_k = 1;
    Eigen::Index ending_k = i;
    if (local_i > dim_x - 1) {
      local_i = dim_x - 1;
      starting_k = i - local_i + 1;
      ending_k = starting_k + dim_x - 2;
    }
    if (ending_k > dim_y - 1) {
      ending_k = dim_y - 1;
    }
    for (Eigen::Index k = starting_k; k <= ending_k; ++k) {
      index_tuple idx(local_i, k);
      auto west = similarity_matrix(idx.first, idx.second - 1);
      auto north = similarity_matrix(idx.first - 1, idx.second);
      auto north_west = similarity_matrix(idx.first - 1, idx.second - 1);
      auto a = sequence_x[idx.first - 1];
      auto b = sequence_y[idx.second - 1];
      similarity_matrix(local_i, k) = dp_func(north, west, north_west, scoring_function(a, b), gap_penalty);
      --local_i;
    }
  }
}

const Eigen::MatrixXd &Similarity_Matrix::get_matrix() const {
  return similarity_matrix;
}

Similarity_Matrix_Skewed::Similarity_Matrix_Skewed(std::string_view sequence_x, std::string_view sequence_y)
    : sequence_x(sequence_x), sequence_y(sequence_y) {
  len_x = sequence_x.size() + 1;
  len_y = sequence_y.size() + 1;
  raw_matrix = Eigen::MatrixXd(std::min(len_x,len_y), std::max(len_x,len_y)); 
  raw_matrix.setZero();
}

std::tuple<Eigen::Index, Eigen::Index, double> Similarity_Matrix_Skewed::find_index_of_maximum() const {
  index_tuple maxidx(0,0);
  auto max = raw_matrix.maxCoeff(&maxidx.first, &maxidx.second);
  auto [x, y] = rawindex2trueindex(maxidx);
#ifdef VERBOSE
  std::cout << "Maximum is " << max << " @( " << maxidx.first << "," << maxidx.second << ")" << std::endl;
#endif
  return {x, y, max};
}

void Similarity_Matrix_Skewed::print_matrix() const {
  Eigen::MatrixXd similarity_matrix(len_x,len_y);
  for (int j = 0; j < len_y; j++) {
    for (int i = 0; i < len_x; i++) {
      index_tuple trueidx(i,j);
      auto [ri, rj] = trueindex2rawindex(trueidx);
      similarity_matrix(i,j) = raw_matrix(ri,rj);
    }
  }
  std::cout << similarity_matrix << std::endl;
}

index_tuple _rawindex2trueindex(size_t ri, size_t rj, size_t nrows, size_t ncols, size_t len_x, size_t len_y) {
 if (rj < nrows - 1) {
    if (ri <= rj) {//upper triangular part
      return index_tuple(ri , rj - ri);
    } else {//lower triangular part
      return index_tuple(len_x - nrows + ri, len_y - ri + rj);//index_tuple(len_x - (nrows - 1 - rj) + (ri - rj - 1), len_y - 1 - (ri - rj - 1));
    }
  } else {//equal-length diagonal part
    if (len_x <= len_y) {//diagonal propagates horizontally (+y)
      return index_tuple(ri, rj - ri);
    } else {//diagonal propagates vertically (+x)
      return index_tuple(rj - (nrows - 1) + ri, nrows - 1 - ri);
    }
  }
}

index_tuple Similarity_Matrix_Skewed::rawindex2trueindex(index_tuple raw_index) const {
  auto [ri,rj] = raw_index;
  auto nrows = raw_matrix.rows();
  auto ncols = raw_matrix.cols();//Always have nrows <= ncols
  return _rawindex2trueindex(ri, rj, nrows, ncols, len_x, len_y);
}

index_tuple _trueindex2rawindex(size_t ti, size_t tj, size_t nrows, size_t ncols, size_t len_x, size_t len_y) {
  if (ti + tj < nrows - 1) {//upper triangular part
    return index_tuple(ti ,ti + tj);
  } else if (ti + tj > ncols - 1) {//lower triangular part
    return index_tuple(ti - ncols + len_y , ti + tj - (ncols - 1) - 1);//index_tuple(ti + tj - (ncols - 1) + len_y - 1 - tj , ti + tj - (ncols - 1) - 1);
  } else {//equal-length diagonal part
    auto delta_x = len_x <= len_y ? ti : len_y - 1 - tj;
    return index_tuple(delta_x, ti + tj);
  }
}

index_tuple Similarity_Matrix_Skewed::trueindex2rawindex(index_tuple true_index) const {
  auto [ti,tj] = true_index;
  auto nrows = raw_matrix.rows();
  auto ncols = raw_matrix.cols();//Always have nrows <= ncols
  return _trueindex2rawindex(ti, tj, nrows, ncols, len_x, len_y);
}


void Similarity_Matrix_Skewed::iterate_anti_diagonal(const std::function<double(const char &, const char &)> &scoring_function,
                                                     double gap_penalty) {
  auto nrows = raw_matrix.rows();
  auto ncols = raw_matrix.cols();//Always have nrows <= ncols
  auto flag = len_x < len_y;
  //Phase 1: Upper triangular part
  for (int j = 2; j < nrows; j++) {
    for (int i = 1; i < j; i++) {
      auto [ti, tj] = _rawindex2trueindex(i, j, nrows, ncols, len_x, len_y);
      auto a = sequence_x[ti - 1];
      auto b = sequence_y[tj - 1];
      raw_matrix(i, j) = dp_func(raw_matrix(i - 1, j - 1), raw_matrix(i, j - 1), raw_matrix(i - 1, j - 2), scoring_function(a, b), gap_penalty); 
    }
  }
  //Phase 2: Equal-length diagonal part
  for (int j = nrows; j < ncols; j++) {
    if (flag) {//Condition 1: diagonal propagate horizontaly (+y)
      for (int i = 1; i < nrows; i++) {
        auto [ti, tj] = _rawindex2trueindex(i, j, nrows, ncols, len_x, len_y);
        auto a = sequence_x[ti - 1];
        auto b = sequence_y[tj - 1];
        raw_matrix(i, j) = dp_func(raw_matrix(i - 1, j - 1), raw_matrix(i, j - 1), raw_matrix(i - 1, j - 2), scoring_function(a, b), gap_penalty); 
      }
    } else {//Condition 2: diagonal propagate vertically (+x)
      auto di_nw = j == nrows ? 0 : 1;
      for (int i = 0; i < nrows - 1; i++) {
        auto [ti, tj] = _rawindex2trueindex(i, j, nrows, ncols, len_x, len_y);
        auto a = sequence_x[ti - 1];
        auto b = sequence_y[tj - 1];
        raw_matrix(i, j) = dp_func(raw_matrix(i , j - 1), raw_matrix(i + 1, j - 1), raw_matrix(i + di_nw , j - 2), scoring_function(a, b), gap_penalty); 
      }
    }
  }
  //Phase 3: Lower triangular part
  for (int j = 0; j < nrows - 1; j++) {
    auto j_prev = j == 0 ? ncols - 1 : j - 1;
    auto j_prev2 = j <= 1 ? ncols - 2 + j : j - 2;
    auto di_nw = (j == 0) && (!flag) ? 1 : 0;
    for (int i = j + 1; i < nrows; i++) {
      auto [ti, tj] = _rawindex2trueindex(i, j, nrows, ncols, len_x, len_y);
      auto a = sequence_x[ti - 1];
      auto b = sequence_y[tj - 1];
      raw_matrix(i, j) = dp_func(raw_matrix(i - 1, j_prev), raw_matrix(i, j_prev), raw_matrix(i - 1 + di_nw, j_prev2), scoring_function(a, b), gap_penalty);
    }
  }
}
