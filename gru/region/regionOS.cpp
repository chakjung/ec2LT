#include "region.h"

#include "../errorCode.h" // QUERYOSIDERRNUM

// Get OSId via DescribeImagesRequest
void Region::QueryOSId(const Aws::EC2::Model::DescribeImagesRequest &desReq) {
  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  // Get OS Descriptions
  const Aws::EC2::Model::DescribeImagesOutcome &desOutcome =
      client.DescribeImages(desReq);

  if (!desOutcome.IsSuccess()) {
    std::cout << "Failed to get OSId\n"
              << desOutcome.GetError().GetMessage() << std::endl;
    exit(QUERYOSIDERRNUM);
  }

  const Aws::Vector<Aws::EC2::Model::Image> &images =
      desOutcome.GetResult().GetImages();

  if (images.size() == 0) {
    std::cout << "Got 0 OSId\n"
              << desOutcome.GetError().GetMessage() << std::endl;
    exit(QUERYOSIDERRNUM);
  }

  // Break tie by picking the newest image
  Aws::String emptyString{""};
  Aws::String const *newestImageDate = &emptyString;
  Aws::String const *newestImageID;

  for (const Aws::EC2::Model::Image &image : images) {
    if (image.GetCreationDate() > *newestImageDate) {
      newestImageDate = &image.GetCreationDate();
      newestImageID = &image.GetImageId();
    }
  }

  OSId = *newestImageID;

  std::cout << "OSId: " << OSId << std::endl;
}
