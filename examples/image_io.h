// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <cstdio>
#include <algorithm>
#include <map>
#include "common/tensor.h"

namespace oidn {

  // Loads the contents of a file into a buffer
  static shared_vector<char> loadFile(const std::string& filename)
  {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file)
      throw std::runtime_error("cannot open file '" + filename + "'");

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    shared_vector<char> buffer = std::make_shared<std::vector<char>>(size);
    if (fread(buffer->data(), 1, size, file) != size)
      throw std::runtime_error("read error");

    fclose(file);
    return buffer;
  }

  // Loads images stored in a tensor archive
  static Tensor loadImageTZA(const std::string& filename)
  {
    shared_vector<char> buffer = loadFile(filename);
    std::map<std::string, Tensor> tensors = parseTensors(buffer->data());
    auto image = tensors.find("image");
    if (image == tensors.end())
      throw std::runtime_error("invalid tensor image '" + filename + "'");

    // Add ref to the buffer
    image->second.buffer = buffer;

    return image->second;
  }

  // Saves a 3-channel image in HWC format to a PPM file
  static void saveImagePPM(const Tensor& image, const std::string& filename)
  {
    if (image.ndims() != 3 || image.dims[2] < 3 || image.format != "hwc")
      throw std::invalid_argument("image must have 3 channels");

    FILE* file = fopen(filename.c_str(), "wb");
    if (!file)
      throw std::runtime_error("cannot save image: '" + filename + "'");

    fprintf(file, "P6\n%d %d\n255\n", image.dims[1], image.dims[0]);

    for (int i = 0; i < image.dims[0]*image.dims[1]; ++i)
    {
      for (int k = 0; k < 3; ++k)
      {
        int c = std::min(std::max(int(image[i*image.dims[2]+k] * 255.f), 0), 255);
        fputc(c, file);
      }
    }

    fclose(file);
  }

} // namespace oidn

