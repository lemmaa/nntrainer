// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2020 Jijoong Moon <jijoong.moon@samsung.com>
 */
/**
 * @file   tensor_dim.cpp
 * @date   22 May 2020
 * @brief  This is Tensor Dimension Class
 * @see    https://github.com/nnstreamer/nntrainer
 * @author Jijoong Moon <jijoong.moon@samsung.com>
 * @bug    No known bugs except for NYI items
 *
 */

#include <cstring>
#include <regex>
#include <sstream>
#include <stdio.h>

#include <nntrainer_error.h>
#include <nntrainer_log.h>
#include <tensor_dim.h>
#include <util_func.h>

namespace ml {
namespace train {

TensorDim::TensorDim(const std::bitset<MAXDIM> &eff_dim_flag_,
                     const std::bitset<MAXDIM> &dyn_dim_flag_) :
  eff_dim_flag(eff_dim_flag_),
  dyn_dim_flag(dyn_dim_flag_) {
  for (size_t i = 0; i < MAXDIM; ++i) {
    dim[i] = 0;
  }
  len = 0;
  feature_len = 0;
}

TensorDim::TensorDim(std::initializer_list<unsigned int> dims) : TensorDim() {
  int shift_size = MAXDIM - dims.size();

  if (shift_size < 0) {
    throw std::invalid_argument("[TensorDim] max dimension is 4");
  }

  unsigned int cnt = 0;

  for (auto &i : dims) {
    setTensorDim(shift_size + cnt, i);
    cnt += 1;
  }
}

TensorDim::TensorDim(const std::array<unsigned int, 3> &shapes) :
  TensorDim({shapes[0], shapes[1], shapes[2]}) {}

TensorDim::TensorDim(unsigned int b, unsigned int c, unsigned int h,
                     unsigned int w, const std::bitset<MAXDIM> &eff_dim_flag_,
                     const std::bitset<MAXDIM> &dyn_dim_flag_) :
  TensorDim(eff_dim_flag_, dyn_dim_flag_) {
  setTensorDim(0, b);
  setTensorDim(1, c);
  setTensorDim(2, h);
  setTensorDim(3, w);
  feature_len = c * h * w;
  len = b * feature_len;
}

TensorDim::TensorDim(const std::string &shape) : TensorDim() {
  if (setTensorDim(shape) != ML_ERROR_NONE) {
    throw std::invalid_argument("[TensorDim] Setting TensorDim failed");
  }
}

TensorDim &TensorDim::operator=(const TensorDim &rhs) {
  using std::swap;

  TensorDim tmp(rhs);
  swap(*this, tmp);
  return *this;
}

TensorDim &TensorDim::operator=(TensorDim &&rhs) noexcept {
  using std::swap;

  swap(*this, rhs);
  return *this;
}

void TensorDim::resetLen() {
  feature_len = dim[1] * dim[2] * dim[3];
  len = dim[0] * feature_len;
}

const unsigned int TensorDim::getTensorDim(unsigned int idx) const {
  if (idx >= MAXDIM)
    throw std::invalid_argument(
      "[TensorDim] Tensor Dimension index should be between 0 and 4");

  return dim[idx];
}

void TensorDim::setTensorDim(unsigned int idx, unsigned int value) {
  if (idx >= MAXDIM)
    throw std::out_of_range(
      "[TensorDim] Tensor Dimension index should be between 0 and 4");

  if (value <= 0)
    throw std::invalid_argument(
      "[TensorDim] Trying to assign value <=0 to tensor dim");

  if (len == 0) {
    for (size_t i = 0; i < MAXDIM; ++i) {
      dim[i] = 1;
    }
  }

  dim[idx] = value;
  resetLen();
}

int TensorDim::setTensorDim(const std::string &input_shape) {
  int status = ML_ERROR_NONE;
  static const std::regex words_regex("[^\\s.,:;!?]+");
  auto words_begin =
    std::sregex_iterator(input_shape.begin(), input_shape.end(), words_regex);
  auto words_end = std::sregex_iterator();
  int cur_dim = std::distance(words_begin, words_end);
  if (cur_dim <= 0 || (size_t)cur_dim > MAXDIM) {
    ml_loge("Tensor Dimension should be between 1 and 4");
    return ML_ERROR_INVALID_PARAMETER;
  }
  int cn = 0;
  for (std::sregex_iterator i = words_begin; i != words_end; ++i, ++cn) {
    setTensorDim(MAXDIM - cur_dim + cn, std::stoul((*i).str()));
  }

  return status;
}

void TensorDim::setEffDimFlag(const std::bitset<MAXDIM> &dim_flag_) {
  eff_dim_flag = dim_flag_;
}

void TensorDim::setDynDimFlag(const std::bitset<MAXDIM> &dim_flag_) {
  dyn_dim_flag = dim_flag_;
}

const std::bitset<TensorDim::MAXDIM> &TensorDim::getEffDimFlag() const {
  return eff_dim_flag;
}

const std::bitset<TensorDim::MAXDIM> &TensorDim::getDynDimFlag() const {
  return dyn_dim_flag;
}

void swap(TensorDim &lhs, TensorDim &rhs) noexcept {
  std::swap_ranges(std::begin(lhs.dim), std::begin(lhs.dim) + TensorDim::MAXDIM,
                   std::begin(rhs.dim));
  std::swap(lhs.len, rhs.len);
  std::swap(lhs.feature_len, rhs.feature_len);
  std::swap(lhs.eff_dim_flag, rhs.eff_dim_flag);
  std::swap(lhs.dyn_dim_flag, rhs.dyn_dim_flag);
}

unsigned int TensorDim::batch() const { return dim[0]; };

unsigned int TensorDim::channel() const { return dim[1]; };

unsigned int TensorDim::height() const { return dim[2]; };

unsigned int TensorDim::width() const { return dim[3]; };

unsigned int TensorDim::getDataLen() const { return len; };

unsigned int TensorDim::getFeatureLen() const { return feature_len; };

void TensorDim::batch(unsigned int b) { setTensorDim(0, b); }

void TensorDim::channel(unsigned int c) { setTensorDim(1, c); }

void TensorDim::height(unsigned int h) { setTensorDim(2, h); }

void TensorDim::width(unsigned int w) { setTensorDim(3, w); }

const unsigned int *TensorDim::getDim() const { return dim; }

unsigned int TensorDim::getNumDim() { return MAXDIM; }

TensorDim TensorDim::transpose(const std::string &direction) const {
  int dirs[MAXDIM - 1];

  int status = nntrainer::getValues(3, direction, dirs);
  NNTR_THROW_IF(status != ML_ERROR_NONE, std::invalid_argument)
    << "parsing direction failed";

  const std::array<unsigned int, MAXDIM> axes{{0, (unsigned int)dirs[0] + 1,
                                               (unsigned int)dirs[1] + 1,
                                               (unsigned int)dirs[2] + 1}};

  return transpose(axes);
}

TensorDim
TensorDim::transpose(const std::array<unsigned int, MAXDIM> &axes) const {
  TensorDim tmp(*this);

  for (unsigned int i = 0; i < MAXDIM; ++i) {
    tmp.setTensorDim(i, getTensorDim(axes[i]));
  }

  return tmp;
}

bool TensorDim::operator==(const TensorDim &rhs) const {
  for (size_t i = 0; i < MAXDIM; ++i) {
    if (this->dim[i] != rhs.dim[i]) {
      return false;
    }
  }

  return true;
}

bool TensorDim::operator!=(const TensorDim &rhs) const {
  return !(*this == rhs);
}

bool TensorDim::isEmpty() const { return len == 0; }

unsigned int TensorDim::rank() const {
  unsigned int rank = 0;
  for (unsigned int i = 0; i < MAXDIM; i++) {
    if (dim[i] > 1)
      rank += 1;
  }
  return rank;
}

unsigned int &TensorDim::operator[](const unsigned int index) {
  if (index >= MAXDIM)
    throw std::out_of_range(
      "[TensorDim] Tensor Dimension index should be between 0 and 4");
  return dim[index];
}

const unsigned int &TensorDim::operator[](const unsigned int index) const {
  if (index >= MAXDIM)
    throw std::out_of_range(
      "[TensorDim] Tensor Dimension index should be between 0 and 4");
  return dim[index];
}

std::array<unsigned int, TensorDim::MAXDIM> TensorDim::computeStrides() const {
  return {dim[1] * dim[2] * dim[3], dim[2] * dim[3], dim[3], 1};
}

void TensorDim::reverse() { std::reverse(dim, dim + MAXDIM); }

std::vector<int> TensorDim::getEffectiveDimension(bool dynamic) const {
  std::vector<int> eff_dim;
  eff_dim.reserve(eff_dim_flag.count());

  auto get_axis = [dynamic, this](unsigned int axis) -> int {
    if (dynamic && dyn_dim_flag[MAXDIM - axis - 1]) {
      return -1;
    }

    return dim[axis];
  };

  for (unsigned int i = 0; i < MAXDIM; ++i) {
    /// flip dim_flag to effectively match with our cognition
    /// ex) 3:5:1:1 -> 3:5, we are setting eff_dim_flag to 0b1100
    if (eff_dim_flag[MAXDIM - i - 1]) {
      eff_dim.push_back(get_axis(i));
    }
  }

  return eff_dim;
}

bool TensorDim::is_dynamic() const { return dyn_dim_flag.any(); }

std::ostream &operator<<(std::ostream &out, TensorDim const &d) {
  out << "Shape: " << d.batch() << ":" << d.channel() << ":" << d.height()
      << ":" << d.width() << std::endl;
  return out;
}

} /* namespace train */
} /* namespace ml */
