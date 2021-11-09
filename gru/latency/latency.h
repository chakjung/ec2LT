#pragma once

#include <aws/dynamodb/DynamoDBClient.h> // DynamoDBClient

#include <aws/ec2/model/Instance.h> // Instance

// Test latency between all instances
void testLatency(
    Aws::DynamoDB::DynamoDBClient &, const std::string &,
    std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> *> &,
    const int &, const int &, const int &, const int &);
