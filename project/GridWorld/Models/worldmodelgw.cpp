#include "worldmodelgw.h"
DEFINE_int32(sc1,16,"'State Conv 1': The number of feature maps in the first layer of the Unet encoder. Layer 2 will have twice as many feature maps and so on.");
DEFINE_int32(afc1,32,"'Action Fully Connected 1': The number of hidden units in the first layer of the MLP that maps action vectors to action vector embeddings");
DEFINE_int32(afc2,64,"'Action Fully Connected 2': The number of hidden units in the second layer of the MLP that maps action vectors to action vector embeddings");
DEFINE_int32(rc1,16,"'Reward Conv 1': the number of feature maps in the first layer of the ConvNet that maps predicted states to rewards. The next layer will have twice as many feature maps and so on. THe number of layers depend on the size of the map.");
DEFINE_int32(rfc,64,"'Reward Fully Connected': the number of hidden units in the fully connected layer of the ConvNet that maps predicted states to predicted rewards.");

WorldModelGWImpl::WorldModelGWImpl():
  usedDevice(torch::Device(torch::kCPU))
{
  if (torch::cuda::is_available())
    {
      std::cout << "CUDA detected for WorldModelGW: training and inference will be done using CUDA." << std::endl;
      usedDevice = torch::Device(torch::kCUDA);
    }
  else
    {
      std::cout <<"Training will be done using CPU"<<std::endl;
    }
  this->to(usedDevice);
}

WorldModelGWImpl::WorldModelGWImpl(int size,int nStateConv1, int nActionfc1, int nActionfc2, int nRewardConv1, int nRewardfc):
  usedDevice(torch::Device(torch::kCPU)), size(size), nStateConv1(nStateConv1), nActionfc1(nActionfc1), nActionfc2(nActionfc2), nRewardfc(nRewardfc), nRewardConv1(nRewardConv1) 
{

  //Switching to CUDA if available
  
  if (torch::cuda::is_available())
    {
      std::cout << "CUDA detected for WorldModelGW: training and inference will be done using CUDA." << std::endl;
      usedDevice = torch::Device(torch::kCUDA);
    }
  else
    {
      std::cout <<"Training will be done using CPU"<<std::endl;
    }
  this->to(usedDevice);
  nUnetLayers = -1+log(size)/log(2); 

  //Adding the convolutionnal layers of the encoder 

  stateConvLayers1.push_back(register_module("Encoder Conv1_1",torch::nn::Conv2d(torch::nn::Conv2dOptions(3,nStateConv1,3).stride(1).padding(1))));
  stateConvLayers1.push_back(register_module("Encoder Conv1_2",torch::nn::Conv2d(torch::nn::Conv2dOptions(nStateConv1,nStateConv1,3).stride(1).padding(1))));

  for (int i=1;i<nUnetLayers;i++)
    {
      stateConvLayers1.push_back(register_module("Encoder Conv"+std::to_string(i+1)+"_1",torch::nn::Conv2d(torch::nn::Conv2dOptions(nStateConv1*pow(2,i-1),nStateConv1*pow(2,i),3).stride(1).padding(1))));
      stateConvLayers1.push_back(register_module("Encoder Conv"+std::to_string(i+1)+"_2",torch::nn::Conv2d(torch::nn::Conv2dOptions(nStateConv1*pow(2,i),nStateConv1*pow(2,i),3).stride(1).padding(1))));
    }

  //Adding the fully connected layers of the action MLP

  int fcOutputSize = FLAGS_sc1*pow(2,nUnetLayers+2);
  actionfc1 = register_module("actionfc1",torch::nn::Linear(4,nActionfc1));
  actionfc2 = register_module("actionfc2",torch::nn::Linear(nActionfc1,nActionfc2));
  actionfc3 = register_module("actionfc3",torch::nn::Linear(nActionfc2,fcOutputSize));

  //Adding the transposed convolutionnal layers of the decoder
  
  nc_actEmb = fcOutputSize/4; 
  nc_encoderOut = FLAGS_sc1*pow(2,nUnetLayers-1);
  nc_decoderIn = nc_actEmb + nc_encoderOut;
  nc_decoderConv1In = (nc_encoderOut + nc_decoderIn)/2;
  
  int chanCount = nc_decoderIn;
  stateDeconvLayers.push_back(register_module("Decoder Deconv1",torch::nn::Conv2d(torch::nn::Conv2dOptions(chanCount,chanCount/2.,3).stride(2).dilation(1).padding(1).output_padding(1).transposed(true))));
  chanCount = nc_encoderOut;
  for (int i=1;i<nUnetLayers;i++)
    {
      chanCount/=2.;
      stateDeconvLayers.push_back(register_module("Decoder Deconv" + std::to_string(i+1),torch::nn::Conv2d(torch::nn::Conv2dOptions(chanCount,chanCount/2,3).stride(2).dilation(1).padding(1).output_padding(1).transposed(true))));
    }
  
  //Adding the convolutionnal layers of the decoder
  
  chanCount = nc_decoderConv1In;
  stateConvLayers2.push_back(register_module("Decoder Conv1_1",torch::nn::Conv2d(torch::nn::Conv2dOptions(nc_decoderConv1In,nc_encoderOut/2,3).stride(1).padding(1))));
  stateConvLayers2.push_back(register_module("Decoder Conv1_2",torch::nn::Conv2d(torch::nn::Conv2dOptions(nc_encoderOut/2,nc_encoderOut/2,3).stride(1).padding(1))));
  chanCount = nc_encoderOut/2;
  for (int i=1;i<nUnetLayers-1;i++)
    {
      stateConvLayers2.push_back(register_module("Decoder Conv"+std::to_string(i+1)+"_1",torch::nn::Conv2d(torch::nn::Conv2dOptions(chanCount,chanCount/2,3).stride(1).padding(1))));
      chanCount/=2.;
      stateConvLayers2.push_back(register_module("Decoder Conv"+std::to_string(i+1)+"_2",torch::nn::Conv2d(torch::nn::Conv2dOptions(chanCount,chanCount,3).stride(1).padding(1))));
    }

  stateConvLayers2.push_back(register_module("Decoder Conv Final",torch::nn::Conv2d(torch::nn::Conv2dOptions(chanCount/2,1,3).stride(1).padding(1))));

  //Building the CNN of the reward function

  rewardfc1 = register_module("Reward FC 1",torch::nn::Linear(nc_decoderIn*4,nc_decoderIn*2));
  rewardfc2 = register_module("Reward FC 2", torch::nn::Linear(nc_decoderIn*2,nc_decoderIn));
  rewardOut = register_module("Reward FC Out", torch::nn::Linear(nc_decoderIn,3));
  
  /*
  //Building the CNN of the reward function

  rewardConvLayers.push_back(register_module("Reward CNN Conv"+std::to_string(1),torch::nn::Conv2d(torch::nn::Conv2dOptions(3,nRewardConv1,3).stride(1).padding(1))));
  for (int i=1;i<nUnetLayers;i++)
    {
      rewardConvLayers.push_back(register_module("Reward CNN Conv"+std::to_string(i+1),torch::nn::Conv2d(torch::nn::Conv2dOptions(nRewardConv1*pow(2,i-1),nRewardConv1*pow(2,i),3).stride(1).padding(1))));
    }
  rewardfc = register_module("Reward CNN fc", torch::nn::Linear(4*nRewardConv1*pow(2,nUnetLayers-1),nRewardfc));
  rewardOut = register_module("Reward CNN out", torch::nn::Linear(nRewardfc,3));*/
}

torch::Tensor WorldModelGWImpl::encoderForward(torch::Tensor x)
{
  outputCopies = std::vector<torch::Tensor>();
  for (int i=0;i<nUnetLayers;i++)
    {
      x = stateConvLayers1[2*i]->forward(x);
      x = torch::relu(x);
      x = stateConvLayers1[2*i+1]->forward(x);
      x = torch::relu(x);
      x = torch::max_pool2d(x,2);
      outputCopies.push_back(x); //Making a backup that will be used in the decoder
    }
  return x;
}

torch::Tensor WorldModelGWImpl::actionForward(torch::Tensor x)
{
  //One-Hot encoding the batch of actions

  torch::Tensor y = torch::zeros({x.size(0),4}).to(usedDevice);
  x = x.to(torch::kInt32);
  for (int i=0;i<x.size(0);i++)
    {
      y[i][*x[i].to(torch::Device(torch::kCPU)).data<int>()] = 1;
    }

  //Going through the MLP

  y = torch::relu(actionfc1->forward(y));  
  y = torch::relu(actionfc2->forward(y));
  y = torch::relu(actionfc3->forward(y));
  return y;
}

torch::Tensor WorldModelGWImpl::decoderForward(torch::Tensor x)
{
  for (int i=0;i<nUnetLayers-1;i++)
    {
      x = stateDeconvLayers[i]->forward(x);
      x = torch::cat({x,outputCopies[nUnetLayers-i-2]},1);
      x = stateConvLayers2[2*i]->forward(x);
      x = torch::relu(x);
      x = stateConvLayers2[2*i+1]->forward(x);
      x = torch::relu(x);
    }
  x = stateDeconvLayers[nUnetLayers-1]->forward(x);
  x = stateConvLayers2[2*(nUnetLayers-1)]->forward(x);
  int imSize = x.size(2);
  x = x.reshape({x.size(0),imSize*imSize});
  x = torch::softmax(x,1);
  x = x.reshape({x.size(0),imSize,imSize});
  return x;
} 

torch::Tensor WorldModelGWImpl::rewardForward(torch::Tensor x)
{
  /*
  for (int i=0;i<nUnetLayers;i++)
    {
      x = rewardConvLayers[i]->forward(x);
      x = torch::relu(x);
      x = torch::max_pool2d(x,2);
    }
  x = x.view({-1,x.size(1)*x.size(2)*x.size(3)});
  x = rewardfc->forward(x);
  x = torch::relu(x);
  x = rewardOut->forward(x);
  x = torch::log_softmax(x,1);
  return x;
  */
  x = x.view({-1,nc_decoderIn*4});
  x = rewardfc1->forward(x);
  x = torch::relu(x);
  x = rewardfc2->forward(x);
  x = torch::relu(x);
  x = rewardOut->forward(x);
  x = torch::log_softmax(x,1);
  return x;
}

torch::Tensor WorldModelGWImpl::predictState(torch::Tensor stateBatch, torch::Tensor actionBatch)
{
  torch::Tensor encoderOut = this->encoderForward(stateBatch);
  torch::Tensor actionEmbedding = this->actionForward(actionBatch);
  actionEmbedding = actionEmbedding.reshape({actionEmbedding.size(0),nc_actEmb,2,2});
  decoderIn = torch::cat({encoderOut,actionEmbedding},1);
  decoderOut = decoderForward(decoderIn);
  return decoderOut;
}

torch::Tensor WorldModelGWImpl::predictReward(torch::Tensor stateBatch, torch::Tensor actionBatch, bool inputAvailable)
{
  if (!inputAvailable)
    {
      torch::Tensor encoderOut = this->encoderForward(stateBatch);
      torch::Tensor actionEmbedding = this->actionForward(actionBatch);
      actionEmbedding = actionEmbedding.reshape({actionEmbedding.size(0),nc_actEmb,2,2});
      decoderIn = torch::cat({encoderOut,actionEmbedding},1);
    }
  
  return rewardForward(decoderIn); 
  /*

  if(!inputAvailable)
    {
      decoderOut = predictState(stateBatch,actionBatch);
    }
  return rewardForward(decoderOut);
  */
}

torch::Device WorldModelGWImpl::getUsedDevice()
{
  return usedDevice;
}

torch::Tensor WorldModelGWImpl::getDecoderIn()
{
  return decoderIn;
}

torch::Tensor WorldModelGWImpl::getDecoderOut()
{
  return decoderOut;
}


