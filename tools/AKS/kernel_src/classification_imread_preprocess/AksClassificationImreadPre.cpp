/*
 * Copyright 2019 Xilinx Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <aks/AksKernelBase.h>
#include <aks/AksDataDescriptor.h>
#include <aks/AksNodeParams.h>

class ClassificationImreadPreKernel : public AKS::KernelBase
{
  public:
    //int getNumCUs (void) { return 1; }
    int exec_async (
           std::vector<AKS::DataDescriptor*> &in, 
           std::vector<AKS::DataDescriptor*> &out, 
           AKS::NodeParams* nodeParams,
           AKS::DynamicParamValues* dynParams);
};

extern "C" { /// Add this to make this available for python bindings and 

AKS::KernelBase* getKernel (AKS::NodeParams *params)
{
  return new ClassificationImreadPreKernel();
}

int ClassificationImreadPreKernel::exec_async (
           std::vector<AKS::DataDescriptor*> &in, 
           std::vector<AKS::DataDescriptor*> &out, 
           AKS::NodeParams* nodeParams,
           AKS::DynamicParamValues* dynParams)
{
  //std::cout << "[DBG] ClassificationImreadPreKernel: running now ... " << std::endl ;
  int outHeight = nodeParams->_intParams["net_h"];
  int outWidth  = nodeParams->_intParams["net_w"];
  int batchSize = dynParams->imagePaths.size();

  /// Get mean values
  auto meanIter = nodeParams->_floatVectorParams.find("mean");
  float mean [3];
	mean[0] = meanIter->second[0]; //104.007f;
	mean[1] = meanIter->second[1]; //116.669f;
	mean[2] = meanIter->second[2]; //122.679f;


  /// Create output data buffer for a batch data
  std::vector<int> shape = { batchSize, 3, outHeight, outWidth };
  AKS::DataDescriptor * outDD = new AKS::DataDescriptor(shape, AKS::DataType::FLOAT32);
  float * outData = (float*) outDD->data();
  const uint32_t nelemsPerImg = 3 * outHeight * outWidth;

  /// Load images and pre-process it.
  for(int i=0; i < batchSize; ++i) {
    cv::Mat inImage = cv::imread (dynParams->imagePaths[i].c_str());
    if (!inImage.data) {
      std::cerr << "[ERR] Unable to read image: " << dynParams->imagePaths[i] << std::endl;
      return -2;
    }

    /// Resize the image to Network Shape
    cv::Mat resizedImage = cv::Mat(outHeight, outWidth, CV_8SC3);
    cv::resize(inImage, resizedImage, cv::Size(outHeight, outWidth));

    //FILE * fp = fopen ("input.txt", "w");
    /// Pre-Processing loop
    float* batchData = outData + i * nelemsPerImg;
    if (1 /* TODO: Insert correct condition */ ) {
      for (int c = 0; c < 3; c++)
        for (int h = 0; h < outHeight; h++)
          for (int w = 0; w < outWidth; w++) {
            batchData[(c*outHeight*outWidth)
              + (h*outWidth) + w]
              = resizedImage.at<cv::Vec3b>(h,w)[c]-mean[c];
            //fprintf(fp, "%f\n", outData[ (c*outHeight*outWidth) + (h*outWidth) + w] );
          }
    } else {
      for (int h = 0; h < outHeight; h++)
        for (int w = 0; w < outWidth; w++)
          for (int c = 0; c < 3; c++)
            batchData[h*outWidth*3 + w*3 + c]
              = resizedImage.at<cv::Vec3b>(h,w)[c]-mean[c];
    }
    //fclose(fp);
  }

  //std::cout << "[DBG] ClassificationImreadPreKernel: in_height  x in_width  : " << inHeight << " x " << inWidth << std::endl;
  //std::cout << "[DBG] ClassificationImreadPreKernel: out_height x out_width : " << outHeight << " x " << outWidth << std::endl;

  /// Create a cv::Mat with input data
  /*
  FILE * fp1 = fopen ("preproc-input.txt", "w");
  for (int h = 0; h < inImage.rows; h++)
    for (int w = 0; w < inImage.cols; w++)
      for (int c = 0; c < inImage.channels(); c++) {
       fprintf (fp1, "%u\n", inImage.at<cv::Vec3b>(h, w)[c]);
    }
  fclose(fp1);
  */

  /// Push back output
  out.push_back(outDD);
  //std::cout << "[DBG] ClassificationImreadPreKernel: Done!" << std::endl << std::endl;
  return -1; /// No wait
}

}//extern "C"
