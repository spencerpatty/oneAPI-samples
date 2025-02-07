/*
 * Copyright 2020 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

#include <cstdio>
#include <cstdlib>
#include <dpct/dpct.hpp>
#include <dpct/blas_utils.hpp>
#include <oneapi/mkl.hpp>
#include <sycl/sycl.hpp>
#include <vector>

#include "cublas_utils.h"

using data_type = double;

int main(int argc, char *argv[]) try {
  // dpct::device_ext& dev_ct1 = dpct::get_current_device();
  dpct::device_ext dev_ct1 = sycl::device{aspect_selector(sycl::aspect::cpu)};
  sycl::queue &q_ct1 = dev_ct1.default_queue();
  sycl::queue *cublasH = NULL;
  dpct::queue_ptr stream = &q_ct1;

  /*
   *   A = 1.0
   *   B = 5.0
   *   X = 2.1
   *   Y = 1.2
   */

  data_type A = 1.0;
  data_type B = 5.0;
  data_type X = 2.1;
  data_type Y = 1.2;
  std::vector<data_type> param = {1.0, 5.0, 6.0, 7.0, 8.0};  // flag = param[0]

  data_type *d_A = nullptr;
  data_type *d_B = nullptr;

  printf("A\n");
  std::printf("%0.2f\n", A);
  printf("=====\n");

  printf("B\n");
  std::printf("%0.2f\n", B);
  printf("=====\n");

  printf("X\n");
  std::printf("%0.2f\n", X);
  printf("=====\n");

  printf("Y\n");
  std::printf("%0.2f\n", Y);
  printf("=====\n");

  printf("param\n");
  print_vector(param.size(), param.data());
  printf("=====\n");

  /* step 1: create cublas handle, bind a stream */
  cublasH = &q_ct1;

  stream = dev_ct1.create_queue();
  cublasH = stream;

  /* step 3: compute */
  [&]() {
    double *d1_ct13 = &A;
    double *d2_ct14 = &B;
    double *x1_ct15 = &X;
    double *param_ct16 = param.data();
    if (sycl::get_pointer_type(&A, cublasH->get_context()) !=
            sycl::usm::alloc::device &&
        sycl::get_pointer_type(&A, cublasH->get_context()) !=
            sycl::usm::alloc::shared) {
      d1_ct13 = sycl::malloc_shared<double>(8, dpct::get_default_queue());
      d2_ct14 = d1_ct13 + 1;
      x1_ct15 = d1_ct13 + 2;
      param_ct16 = d1_ct13 + 3;
      *d1_ct13 = A;
      *d2_ct14 = B;
      *x1_ct15 = X;
    }
    oneapi::mkl::blas::column_major::rotmg(*cublasH, d1_ct13, d2_ct14, x1_ct15,
                                           Y, param_ct16);
    if (sycl::get_pointer_type(&A, cublasH->get_context()) !=
            sycl::usm::alloc::device &&
        sycl::get_pointer_type(&A, cublasH->get_context()) !=
            sycl::usm::alloc::shared) {
      cublasH->wait();
      A = *d1_ct13;
      B = *d2_ct14;
      X = *x1_ct15;
      dpct::get_default_queue()
          .memcpy(param.data(), param_ct16, sizeof(double) * 5)
          .wait();
      sycl::free(d1_ct13, dpct::get_default_queue());
    }
    return 0;
  }();

  stream->wait();

  /*
   *   A = 3.10
   *   B = 0.62
   *   X = 1.94
   */

  printf("A\n");
  std::printf("%0.2f\n", A);
  printf("=====\n");

  printf("B\n");
  std::printf("%0.2f\n", B);
  printf("=====\n");

  printf("X\n");
  std::printf("%0.2f\n", X);
  printf("=====\n");

  /* free resources */
  sycl::free(d_A, q_ct1);
  sycl::free(d_B, q_ct1);

  cublasH = nullptr;

  dev_ct1.destroy_queue(stream);

  dev_ct1.reset();

  return EXIT_SUCCESS;
} catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}
