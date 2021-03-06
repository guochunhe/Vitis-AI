### Contents
1. [Installation](#installation)
2. [Preparation](#preparation)
3. [Train/Eval](#traineval)
4. [Performance](#Performance)
5. [Model info](#Model info)

### Installation
1. Get the code. We will call the directory that you cloned Caffe into `$CAFFE_ROOT`
  **Note:** To download Caffe_Xlinx
  
  ```shell
  unzip caffe-xilinx.zip
  cd caffe-xilinx
  ```

2. Build the code. Please follow [Caffe instruction](http://caffe.berkeleyvision.org/installation.html) to install all necessary packages and build it.
  ```shell
  # Modify Makefile.config according to your Caffe installation.
  cp Makefile.config.example Makefile.config
  make -j8
  # Make sure to include $CAFFE_ROOT/python to your PYTHONPATH.
  # python version(python2)
  make py
  ```
Note: If you are in the released Docker env, there is no need to build Caffe.

### Preparation
1. Dataset Diretory Structure like:
   ```shell
   + train 
     + images 
       + images_id1.jpg
       + images_id2.jpg
   label.txt: please refer to code/train/train_clatech.txt
    # the format is as follows: 
       < image name i >  < number of lane anchor in this image >  < lane id1 > < xmin > < ymin > < xmax > <ymax> < lane classes> \
       <lane id2> < xmin > ... 

   ```

2. Create the LMDB file.
  ```shell
  Because the data is private so you should copy your data to 'code/train/images/' and modify the 'code/train/images/train_caltech.txt' information. You can use the 'create_data.sh.' convert the data to LMDB.
  #modify the "caffe_xilinx_dir" in "create_data.sh"
  sh ./code/train/create_data.sh
  ```

### Train/Eval
1. Train your model and evaluate the model on the fly.
  ```shell
  #modify the "caffe_xilinx_dir" in "trainval.sh"
  sh ./code/train/trainval.sh
  ```

2. Evaluate the most recent snapshot.
  ```shell
  # Evaluate F1-score
  FLoat model: python ./code/test/demo_evaluate.py
  Quantized model: python ./code/test/demo_quantized_evaluate.py
  ```
### Performance
  ```shell
  images: Caltech
  model: vpg
  Float F1-score:88.639%
  Quantized F1-score:88.82%
  ```
### Model info


1. data preprocess
```
1. data channel order: BGR(0~255)                  
2. resize: 480 * 640(H * W) 
3. mean_value: 104, 117, 123
4. scale: 1
```
2.For quantization with calibration mode:
  ```
  Modify datalayer of test.prototxt for model quantization:
  a. Replace the "Input" data layer of test.prototxt with the "ImageData" data layer.
  b. Modify the "ImageData" layer parameters according to the data preprocess information.
  c. Provide a "quant.txt" file, including image path and label information with fake value(like 1).
  d. Give examples of data layer and "quant.txt":
  # data layer example
    layer {
    name: "data"
    type: "ImageData"
    top: "data"
    top: "label"
    include {
      phase: TRAIN
    }
    transform_param {
      mirror: false
      mean_value: 104
      mean_value: 117
      mean_value: 123
     }

    image_data_param {
      source: "quant.txt"
      new_width: 640  
      new_height: 480
      batch_size: 16
    }
  }
  # quant.txt: image path label
    images/000001.jpg 1
    images/000002.jpg 1
    images/000003.jpg 1

  ```
3.For quantization with finetuning mode: 
  ```
  use trainval.prototxt for model quantization.
  ```
4.For deployment, modify "deploy.prototxt" generated by quantizer
  ```
  remove layer "pixel-tile" and "bb-tile"

  ```
