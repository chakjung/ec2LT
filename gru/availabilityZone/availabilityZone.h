#pragma once

#include <aws/ec2/model/Instance.h> // Instance

class AZ {
public:
  Aws::String AZId;
  Aws::EC2::Model::Instance Instance;
}
